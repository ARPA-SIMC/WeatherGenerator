#include "wgProject.h"
#include "commonConstants.h"
#include "fileUtility.h"
#include "timeUtility.h"
#include "wgClimate.h"
#include "weatherGenerator.h"
#include "importData.h"
#include "waterTable.h"
#include "gis.h"

#include <time.h>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QSettings>


WGSettings::WGSettings()
{
    this->observedPath = "";
    this->climatePath = "";
    this->seasonalForecastPath = "";
    this->scenarioPath = "";
    this->outputPath = "";
    this->waterTablePath = "";

    this->isSeasonalForecast = false;
    this->isScenario = false;
    this->isWaterTable = false;

    this->valuesSeparator = ',';

    this->minDataPercentage = 0.8f;
    this->rainfallThreshold = 0.2f;
    this->waterTableMaximumDepth = NODATA;
    this->lat = NODATA;
    this->lon = NODATA;

    this->firstYear = 2001;
    this->nrYears = 1;
}

bool readWGSettings(const QString &settingsFileName, WGSettings &wgSettings)
{
    QFile myFile(settingsFileName);
    if (! myFile.exists())
    {
        qDebug() << "Error!" << "\nMissing settings file:" << settingsFileName;
        return false;
    }

    QFileInfo fileInfo(settingsFileName);
    QString pathSettingsFile = fileInfo.path() + "/";


    QSettings* mySettings = new QSettings(settingsFileName, QSettings::IniFormat);
    mySettings->beginGroup("settings");

    wgSettings.climatePath = mySettings->value("climate").toString();
    if (wgSettings.climatePath.left(1) == ".")
    {
        wgSettings.climatePath = pathSettingsFile + wgSettings.climatePath;
    }

    if (wgSettings.climatePath == "")
    {
        qDebug() << "Missing climate path in file .ini";
        return false;
    }

    wgSettings.observedPath = mySettings->value("observed").toString();
    if (wgSettings.observedPath.left(1) == ".")
    {
        wgSettings.observedPath = pathSettingsFile + wgSettings.observedPath;
    }

    // seasonal forecast
    wgSettings.isSeasonalForecast = false;
    QVariant myValue = mySettings->value("seasonalForecast");
    if (! myValue.isValid())
    {
        myValue = mySettings->value("seasonalPredictions");
    }
    if (myValue.isValid())
    {
        wgSettings.isSeasonalForecast = true;
        wgSettings.seasonalForecastPath = myValue.toString();
        if (wgSettings.seasonalForecastPath.left(1) == ".")
        {
            wgSettings.seasonalForecastPath = pathSettingsFile + wgSettings.seasonalForecastPath;
        }
    }

    // scenarios
    wgSettings.isScenario = false;
    myValue = mySettings->value("scenario");
    if (myValue.isValid())
    {
        wgSettings.isScenario = true;
        wgSettings.scenarioPath = myValue.toString();
        if (wgSettings.scenarioPath.left(1) == ".")
        {
            wgSettings.scenarioPath = pathSettingsFile + wgSettings.scenarioPath;
        }
    }

    // waterTable
    wgSettings.isWaterTable = false;
    myValue = mySettings->value("waterTablePath");
    if (myValue.isValid())
    {
        wgSettings.isWaterTable = true;
        wgSettings.waterTablePath = myValue.toString();
        if (wgSettings.waterTablePath.left(1) == ".")
        {
            wgSettings.waterTablePath = pathSettingsFile + wgSettings.waterTablePath;
        }
    }

    wgSettings.outputPath = mySettings->value("output").toString();
    if (wgSettings.outputPath.left(1) == ".")
    {
        wgSettings.outputPath = QDir::cleanPath(pathSettingsFile + wgSettings.outputPath);
    }

    if (wgSettings.outputPath != "")
    {
        if (! QDir(wgSettings.outputPath).exists())
        {
            QDir().mkdir(wgSettings.outputPath);
        }
    }

    //check output directory
    QDir outputDirectory(wgSettings.outputPath);
    if (! outputDirectory.exists())
        if (! outputDirectory.mkdir(wgSettings.outputPath))
        {
            qDebug() << "Error: missing output directory" << wgSettings.outputPath;
            return false;
        }

    QString mySeparator = mySettings->value("separator").toString();
    wgSettings.valuesSeparator = mySeparator.toStdString().c_str()[0];

    wgSettings.minDataPercentage = mySettings->value("minDataPercentage").toFloat();
    wgSettings.rainfallThreshold = mySettings->value("rainfallThreshold").toFloat();
    wgSettings.waterTableMaximumDepth = mySettings->value("waterTableMaximumDepth").toInt();

    wgSettings.firstYear = mySettings->value("firstYear").toInt();
    wgSettings.nrYears = mySettings->value("nrYears").toInt();

    bool ok;
    wgSettings.lat = mySettings->value("latitude_default").toFloat(&ok);
    if (ok == false)
    {
        wgSettings.lat = NODATA;
    }
    wgSettings.lon = mySettings->value("longitude_default").toFloat(&ok);
    if (ok == false)
    {
        wgSettings.lon = NODATA;
    }

    return true;
}


bool WG_SeasonalForecast(const WGSettings &wgSettings)
{
    XMLSeasonalAnomaly XMLAnomaly;
    TinputObsData climateDailyObsData;
    TinputObsData dailyObsData;
    TweatherGenClimate wGenClimate;

    QString season;
    int wgDoy1 = NODATA;
    int wgDoy2 = NODATA;
    Crit3DDate climateDateIni, climateDateFin;

    // iterate input files on climate (climateName.csv = observedName.csv = forecastName.xml)
    QString fileName, climateFileName, observedFileName, xmlFileName, outputFileName;
    QDir climateDirectory(wgSettings.climatePath);
    QStringList filters ("*.csv");
    QFileInfoList fileList = climateDirectory.entryInfoList (filters);

    // check
    if (fileList.size() == 0)
    {
        qDebug() << "Missing climate files in path: " + wgSettings.climatePath;
        return false;
    }

    bool isOk;
    QFile *testFile;
    for (int i = 0; i < fileList.size(); ++i)
    {
        fileName = fileList.at(i).fileName();
        climateFileName = wgSettings.climatePath + "/" + fileName;
        observedFileName = wgSettings.observedPath + "/" + fileName;
        xmlFileName = wgSettings.seasonalForecastPath + "/" + fileName.left(fileName.length()-4) + ".xml";
        outputFileName = wgSettings.outputPath + "/" + fileName;

        //check observed data
        isOk = true;
        testFile = new QFile(observedFileName);
        if (! testFile->exists())
        {
            qDebug() << "ERROR: missing observed data:" << fileName;
            isOk = false;
        }

        // check xml file
        testFile = new QFile(xmlFileName);
        if (! testFile->exists())
        {
            xmlFileName = wgSettings.seasonalForecastPath + "/" + "GRD_" + fileName.left(fileName.length()-4) + ".xml";
            testFile = new QFile(xmlFileName);
            if (! testFile->exists())
            {
                qDebug() << "ERROR: missing seasonal forecast:" << xmlFileName;
                isOk = false;
            }
        }

        if (isOk)
        {
            qDebug() << "\nCompute seasonal:" << fileName;

            // read SEASONAL PREDICTIONS
            if (! parseXMLSeasonal(xmlFileName, XMLAnomaly))
                return false;

            // compute first and last day of the year of the season period
            season = XMLAnomaly.anomalySeason.toUpper();
            getDoyFromSeason(season, XMLAnomaly.anomalyYear, wgDoy1, wgDoy2);

            // set climate dates
            climateDateIni = Crit3DDate(1,1,XMLAnomaly.climatePeriod.yearFrom);
            climateDateFin = Crit3DDate(31, 12, XMLAnomaly.climatePeriod.yearTo);

            XMLAnomaly.printInfo();

            // read CLIMATE data
            if (! readMeteoDataCsv(climateFileName, wgSettings.valuesSeparator, NODATA, climateDailyObsData) )
                return false;

            // read OBSERVED data (at least last 9 months)
            if (! readMeteoDataCsv(observedFileName, wgSettings.valuesSeparator, NODATA, dailyObsData) )
                return false;

            //check climate dates
            Crit3DDate climateObsFirstDate = climateDailyObsData.inputFirstDate;
            climateObsFirstDate = std::max(climateDateIni, climateObsFirstDate);

            Crit3DDate climateObsLastDate = climateDailyObsData.inputFirstDate.addDays(climateDailyObsData.dataLength-1);
            climateObsLastDate = std::min(climateDateFin, climateObsLastDate);

            int requestedClimateDays = climateDateIni.daysTo(climateDateFin);
            int inputClimateNrDays = climateObsFirstDate.daysTo(climateObsLastDate);

            if ((float(inputClimateNrDays) / float(requestedClimateDays)) < wgSettings.minDataPercentage)
            {
                qDebug() << "\nERROR:" << "\nRequested climate period is:" << XMLAnomaly.climatePeriod.yearFrom << "-" << XMLAnomaly.climatePeriod.yearTo;
                qDebug() << "Percentage of climate data are less than requested (" << (wgSettings.minDataPercentage*100) << "%)";
                qDebug() << "\n***** ERROR! *****" << fileName << "Computation FAILED\n";
            }
            else
            {
                if (XMLAnomaly.point.latitude == NODATA && wgSettings.isWaterTable)
                {
                    XMLAnomaly.point.latitude = wgSettings.lat;
                    qDebug() << "\n***** WARNING! *****" << fileName << " : missing latitude inside xml, using latitude_default \n";
                }
                if (XMLAnomaly.point.longitude == NODATA && wgSettings.isWaterTable)
                {
                    XMLAnomaly.point.longitude = wgSettings.lon;
                    qDebug() << "\n***** WARNING! *****" << fileName << " : missing longitude inside xml, using longitude_default \n";
                }
                // weather generator - computes climate without anomaly
                if (! climateGenerator(climateDailyObsData.dataLength, climateDailyObsData, climateObsFirstDate, climateObsLastDate, wgSettings.rainfallThreshold, wgSettings.minDataPercentage, &wGenClimate))
                {
                    qDebug() << "Error in climateGenerator";
                    qDebug() << "\n***** ERROR! *****" << fileName << "Computation FAILED\n";
                }
                else
                {
                    qDebug() << "Climate OK";

                    /* initialize random seed: */
                    srand (time(nullptr));

                    // SEASONAL FORECAST
                    if (! makeSeasonalForecast(outputFileName, wgSettings.valuesSeparator, &XMLAnomaly,
                                wGenClimate, &dailyObsData, XMLAnomaly.repetitions,
                                XMLAnomaly.anomalyYear, wgDoy1, wgDoy2, wgSettings.rainfallThreshold))
                    {
                        qDebug() << "\n***** ERROR! *****" << fileName << "Computation FAILED\n";
                    }
                }
            }

            clearInputData(climateDailyObsData);
            clearInputData(dailyObsData);
        }
    }
    return true;
}


bool WG_Scenario(const WGSettings &wgSettings)
{
    XMLScenarioAnomaly XMLAnomaly;
    TinputObsData climateDailyObsData;
    TweatherGenClimate wGenClimate,wGen;
    std::vector<ToutputDailyMeteo> outputDailyData;
    // iterate input files on climate
    QString fileName, climateFileName, outputFileName;
    QDir climateDirectory(wgSettings.climatePath);
    QStringList filters ("*.csv");
    QFileInfoList fileList = climateDirectory.entryInfoList (filters);

    QString season[4]={"DJF", "MAM", "JJA", "SON"};
    int wgDoy1[4] = {NODATA, NODATA, NODATA, NODATA};
    int wgDoy2[4] = {NODATA, NODATA, NODATA, NODATA};

    // check file presence
    if (fileList.size() == 0)
    {
        qDebug() << "Missing climate files in path: " + wgSettings.climatePath;
        return false;
    }

    for (int i = 0; i < fileList.size(); ++i)
    {
        QString fileName = fileList.at(i).fileName();
        QString climateFileName = wgSettings.climatePath + "/" + fileName;
        QString xmlFileName = wgSettings.scenarioPath + "/" + fileName.left(fileName.length()-4) + ".xml";

        QFile *testFile = new QFile(xmlFileName);
        if (! testFile->exists())
        {
            qDebug() << "ERROR:\nMissing scenario:" << xmlFileName;
            continue;
        }

        qDebug() << "\n*** Compute scenario:" << fileName;

        // read SCENARIO
        if (! parseXMLScenario(xmlFileName, XMLAnomaly))
        {
            qDebug() << "ERROR:\nWrong scenario:" << xmlFileName;
            continue;
        }
        else
        {
            qDebug() << "Scenario XML OK";
        }
        XMLAnomaly.anomalyYear = XMLAnomaly.scenario.yearFrom;
        // compute first and last day of the year of the season period
        for (int iSeason = 0; iSeason < 4; iSeason++)
        {
            getDoyFromSeason(season[iSeason], XMLAnomaly.anomalyYear, wgDoy1[iSeason], wgDoy2[iSeason]);
        }
        // set climate dates
        Crit3DDate climateDateIni = Crit3DDate(1, 1, XMLAnomaly.climatePeriod.yearFrom);
        Crit3DDate climateDateFin = Crit3DDate(31, 12, XMLAnomaly.climatePeriod.yearTo);

        XMLAnomaly.printInfo();

        // read CLIMATE data
        if (! readMeteoDataCsv(climateFileName, wgSettings.valuesSeparator, NODATA, climateDailyObsData) )
            return false;

        // check climate dates
        Crit3DDate climateObsFirstDate = climateDailyObsData.inputFirstDate;
        climateObsFirstDate = std::max(climateDateIni, climateObsFirstDate);

        Crit3DDate climateObsLastDate = climateDailyObsData.inputFirstDate.addDays(climateDailyObsData.dataLength-1);
        climateObsLastDate = std::min(climateDateFin, climateObsLastDate);

        int requestedClimateDays = climateDateIni.daysTo(climateDateFin);
        int obsClimateDays = climateObsFirstDate.daysTo(climateObsLastDate);
        float ratioData = float(obsClimateDays) / float(requestedClimateDays);

        if (ratioData < wgSettings.minDataPercentage)
        {
            qDebug() << "\nERROR:" << "\nRequested climate period is:" << XMLAnomaly.climatePeriod.yearFrom << "-" << XMLAnomaly.climatePeriod.yearTo;
            qDebug() << "Percentage of climate data are less than requested (" << (wgSettings.minDataPercentage*100) << "%)";
            qDebug() << "\n***** ERROR! *****" << fileName << "Computation FAILED\n";
        }
        else
        {
            // weather generator - computes climate without anomaly
            initializeWeather(wGenClimate);
            initializeWeather(wGen);
            if (! climateGenerator(climateDailyObsData.dataLength, climateDailyObsData, climateObsFirstDate, climateObsLastDate, wgSettings.rainfallThreshold, wgSettings.minDataPercentage, &wGenClimate))
            {
                qDebug() << "Error in climateGenerator";
                qDebug() << "\n***** ERROR! *****" << fileName << "Computation FAILED\n";
            }
            else
            {
                qDebug() << "Climate OK";

                // initialize random seed
                srand (time(nullptr));

                // SCENARIO
                // output size
                int outputSize = 0;
                for (int iYear=0; iYear<wgSettings.nrYears; iYear++)
                {
                    outputSize += 365 + isLeapYear(wgSettings.firstYear+iYear);
                }
                outputDailyData.resize(outputSize);

                outputFileName = wgSettings.outputPath + "/" + fileName;
                QString startingSeason = XMLAnomaly.period[0].type;
                int anomalyMonth1, anomalyMonth2;

                if(startingSeason == "DJF")
                {
                    anomalyMonth1 = 12; anomalyMonth2 = 2;
                }
                else if (startingSeason == "MAM")
                {
                    anomalyMonth1 = 3; anomalyMonth2 = 5;
                }
                else if (startingSeason == "JJA")
                {
                    anomalyMonth1 = 6; anomalyMonth2 = 8;
                }
                else
                {
                    anomalyMonth1 = 9; anomalyMonth2 = 11;
                }
                assignXMLAnomalyScenario(&XMLAnomaly,0,anomalyMonth1, anomalyMonth2, wGenClimate,wGen);
                int myDoy;
                Crit3DDate firstDate, lastDate,myDate;
                firstDate.day = 1;
                firstDate.month = 1;
                firstDate.year = wgSettings.firstYear;
                lastDate.day = 31;
                lastDate.month = 12;
                lastDate.year = wgSettings.firstYear + wgSettings.nrYears - 1;
                int currentIndex = 0 ;
                for (myDate = firstDate; myDate <= lastDate; ++myDate)
                {
                    myDoy = getDoyFromDate(myDate);
                    initializeDailyDataBasic (&outputDailyData[currentIndex], myDate);
                    outputDailyData[currentIndex].maxTemp = getTMax(myDoy, wgSettings.rainfallThreshold, wGen);
                    outputDailyData[currentIndex].minTemp = getTMin(myDoy, wgSettings.rainfallThreshold, wGen);
                    outputDailyData[currentIndex].prec = getPrecip(myDoy, wgSettings.rainfallThreshold, wGen);
                    currentIndex++;
                }


            }
            qDebug() << "Weather Generator OK";
            qDebug() << "Output:" << outputFileName;
            writeMeteoDataCsv(outputFileName, wgSettings.valuesSeparator, outputDailyData);
        }

        clearInputData(climateDailyObsData);
    }

    return true;
}


bool WG_Climate(WGSettings &wgSettings)
{
    TinputObsData climateDailyObsData;
    TweatherGenClimate wGenClimate;

    // iterate input files on climate
    QString fileName, climateFileName, outputFileName;
    QDir climateDirectory(wgSettings.climatePath);
    QStringList filters ("*.csv");
    QFileInfoList fileList = climateDirectory.entryInfoList (filters);
    std::vector<ToutputDailyMeteo> outputDailyData;

    if (fileList.isEmpty())
    {
        qDebug() << "Missing data in climate directory.";
        return false;
    }

    for (int i = 0; i < fileList.size(); ++i)
    {
        fileName = fileList.at(i).fileName();
        climateFileName = wgSettings.climatePath + "/" + fileName;
        outputFileName = wgSettings.outputPath + "/" + fileName;

        qDebug() << "\n...Compute climate:" << fileName;

        // read CLIMATE data
        if (! readMeteoDataCsv(climateFileName, wgSettings.valuesSeparator, NODATA, climateDailyObsData) )
            return false;

        // check climate dates
        Crit3DDate climateObsFirstDate = climateDailyObsData.inputFirstDate;
        Crit3DDate climateObsLastDate = climateDailyObsData.inputFirstDate.addDays(climateDailyObsData.dataLength-1);

        if (wgSettings.isWaterTable)
        {
            Well myWell;
            myWell.setId(fileName);
            if(wgSettings.lat == NODATA || wgSettings.lon == NODATA)
            {
                qDebug() << "\n***** ERROR! ***** Missing lat-lon coordination\n";
                return false;
            }
            double utmEasting;
            double utmNorthing;
            int zoneNumber;
            gis::latLonToUtm(wgSettings.lat, wgSettings.lon, &utmEasting, &utmNorthing, &zoneNumber);
            myWell.setUtmX(utmEasting);
            myWell.setUtmY(utmNorthing);

            QDate first(climateObsFirstDate.year,climateObsFirstDate.month, climateObsFirstDate.day);
            QDate last(climateObsLastDate.year,climateObsLastDate.month, climateObsLastDate.day);
            QString waterTableFileName = wgSettings.waterTablePath + "/" + fileName;
            QString errorString;
            int wrongLines = 0;
            if (! loadCsvDepthsSingleWell(waterTableFileName, &myWell, wgSettings.waterTableMaximumDepth, first, last, errorString, wrongLines))
            {
                qDebug() << "\n***** ERROR! *****" << errorString << "Import Csv depths FAILED\n";
                continue;
            }

            if (wrongLines>0)
            {
                qDebug() << "\n***** WARNING! *****" << fileName << ": " << QString::number(wrongLines) << " lines of data were not loaded\n";
            }

            int minValuePerMonth = myWell.minValuesPerMonth();
            if (minValuePerMonth < 1)
            {
                qDebug() << "\n***** ERROR! *****" << fileName << "There are less than 1 value per month\n";
                continue;
            }

            int maxNrDays = 730;  // attualmente fisso
            Crit3DMeteoSettings meteoSettings;
            meteoSettings.setMinimumPercentage(wgSettings.minDataPercentage);
            meteoSettings.setRainfallThreshold(wgSettings.rainfallThreshold);
            meteoSettings.setTransSamaniCoefficient(float(SAMANI_COEFF));
            gis::Crit3DGisSettings gisSettings;
            gisSettings.utmZone = zoneNumber;
            gisSettings.startLocation.latitude = wgSettings.lat;
            gisSettings.startLocation.longitude = wgSettings.lon;
            WaterTable waterTable(climateDailyObsData.inputTMin, climateDailyObsData.inputTMax, climateDailyObsData.inputPrecip, first, last, meteoSettings, gisSettings);
            if (!waterTable.computeWaterTableParameters(myWell, maxNrDays))
            {
                qDebug() << "\n***** ERROR! *****" << waterTable.getError() << "computeWaterTable FAILED\n";
                continue;
            }
            qDebug() << "Nr of observed depth: " << waterTable.getNrObsData() << "\n";
            qDebug() << "alpha [-]: " << waterTable.getAlpha() << "\n";
            qDebug() << "H0 [cm]: " << (int)waterTable.getH0() << "\n";
            qDebug() << "Nr days: " << waterTable.getNrDaysPeriod() << "\n";
            qDebug() << "R2 [-]: " << waterTable.getR2() << "\n";
            qDebug() << "RMSE [cm]: " << waterTable.getRMSE() << "\n";
            qDebug() << "Nash-Sutcliffe [-]: " << waterTable.getNASH() << "\n";
            qDebug() << "Efficiency Index [-]: " << waterTable.getEF() << "\n";
        }

        // weather generator - computes climate
        if (! climateGenerator(climateDailyObsData.dataLength, climateDailyObsData, climateObsFirstDate, climateObsLastDate, wgSettings.rainfallThreshold, wgSettings.minDataPercentage, &wGenClimate))
        {
            qDebug() << "Error in climateGenerator";
            qDebug() << "\n***** ERROR! *****" << fileName << "Computation FAILED\n";
        }
        else
        {
            qDebug() << "Climate OK";

            /* initialize random seed: */
            srand(unsigned(time(nullptr)));

            outputDailyData.clear();

            if (! computeClimate(wGenClimate, wgSettings.firstYear, wgSettings.nrYears,
                                wgSettings.rainfallThreshold, outputDailyData))
            {
                qDebug() << "\n***** ERROR! *****" << fileName << "Computation FAILED\n";
            }

            qDebug() << "Weather Generator OK";
            qDebug() << "Output:" << outputFileName;
            writeMeteoDataCsv(outputFileName, wgSettings.valuesSeparator, outputDailyData);
        }

        clearInputData(climateDailyObsData);
    }

    return true;
}

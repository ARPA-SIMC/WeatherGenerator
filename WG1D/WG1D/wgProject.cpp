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

    wgSettings.lat = mySettings->value("latitude_default").toFloat();
    wgSettings.lon = mySettings->value("longitude_default").toFloat();

    return true;
}


bool WG_SeasonalForecast(const WGSettings &wgSettings)
{
    XMLSeasonalAnomaly XMLAnomaly;
    TinputObsData climateDailyObsData;
    TinputObsData lastYearDailyObsData;
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
            if (! readMeteoDataCsv(observedFileName, wgSettings.valuesSeparator, NODATA, lastYearDailyObsData) )
                return false;

            //check climate dates
            Crit3DDate climateObsFirstDate = climateDailyObsData.inputFirstDate;
            climateObsFirstDate = std::max(climateDateIni, climateObsFirstDate);

            Crit3DDate climateObsLastDate = climateDailyObsData.inputFirstDate.addDays(climateDailyObsData.dataLength-1);
            climateObsLastDate = std::min(climateDateFin, climateObsLastDate);

            int requestedClimateDays = climateDateIni.daysTo(climateDateFin);
            int obsClimateDays = climateObsFirstDate.daysTo(climateObsLastDate);

            if ((float(obsClimateDays) / float(requestedClimateDays)) < wgSettings.minDataPercentage)
            {
                qDebug() << "\nERROR:" << "\nRequested climate period is:" << XMLAnomaly.climatePeriod.yearFrom << "-" << XMLAnomaly.climatePeriod.yearTo;
                qDebug() << "Percentage of climate data are less than requested (" << (wgSettings.minDataPercentage*100) << "%)";
                qDebug() << "\n***** ERROR! *****" << fileName << "Computation FAILED\n";
            }
            else
            {
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
                                wGenClimate, &lastYearDailyObsData, XMLAnomaly.repetitions,
                                XMLAnomaly.anomalyYear, wgDoy1, wgDoy2, wgSettings.rainfallThreshold))
                    {
                        qDebug() << "\n***** ERROR! *****" << fileName << "Computation FAILED\n";
                    }
                }
            }

            clearInputData(climateDailyObsData);
            clearInputData(lastYearDailyObsData);
        }
    }
    return true;
}


bool WG_Scenario(const WGSettings &wgSettings)
{
    XMLScenarioAnomaly XMLAnomaly;
    TinputObsData climateDailyObsData;
    TweatherGenClimate wGenClimate;

    QString season[4]={"DJF", "MAM", "JJA", "SON"};
    int wgDoy1[4] = {NODATA, NODATA, NODATA, NODATA};
    int wgDoy2[4] = {NODATA, NODATA, NODATA, NODATA};

    // iterate input files on climate (climateName.csv = xmlFileName.xml)
    QDir climateDirectory(wgSettings.climatePath);
    QStringList filters ("*.csv");
    QFileInfoList fileList = climateDirectory.entryInfoList(filters);

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

                QString outputFileName = wgSettings.outputPath + "/" + fileName;

                if (! makeScenario(outputFileName,wgSettings.valuesSeparator,&XMLAnomaly,wGenClimate,wgSettings.nrYears,2021,wgDoy1,wgDoy2,wgSettings.rainfallThreshold))
                {
                    qDebug() << "\n***** ERROR! *****" << fileName << "Computation FAILED\n";
                }
            }
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

        Well myWell;
        if (wgSettings.isWaterTable)
        {
            myWell.setId(fileName);
            if(wgSettings.lat == NODATA || wgSettings.lon == NODATA)
            {
                qDebug() << "\n***** ERROR! *****" << fileName << ": " << " missing lat-lon coordination\n";
                continue;
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
            // meteoSettings.setTransSamaniCoefficient(); // TO DO
            gis::Crit3DGisSettings gisSettings;
            gisSettings.utmZone = zoneNumber;
            //gisSettings.startLocation = ; // TO DO
            WaterTable waterTable(climateDailyObsData.inputTMin, climateDailyObsData.inputTMax, climateDailyObsData.inputPrecip, first, last, meteoSettings, gisSettings);
            waterTable.computeWaterTable(myWell, maxNrDays);
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

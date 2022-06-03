#include "wgProject.h"
#include "commonConstants.h"
#include "fileUtility.h"
#include "timeUtility.h"
#include "wgClimate.h"

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
    this->outputPath = "";

    this->isSeasonalForecast = false;

    this->valuesSeparator = ',';

    this->minDataPercentage = 0.8f;
    this->rainfallThreshold = 0.2f;

    this->firstYear = 2001;
    this->nrYears = 1;
}


void printSeasonalForecastInfo(TXMLSeasonalAnomaly *XMLAnomaly)
{
    qDebug() << "\npoint.name = " << XMLAnomaly->point.name;
    qDebug() << "point.longitude = " << XMLAnomaly->point.longitude;
    qDebug() << "point.latitude = " << XMLAnomaly->point.latitude;
    qDebug() << "climate first year = " << XMLAnomaly->climatePeriod.yearFrom;
    qDebug() << "climate last year = " << XMLAnomaly->climatePeriod.yearTo;
    qDebug() << "number of models = " << XMLAnomaly->modelNumber;
    qDebug() << "models = " << XMLAnomaly->modelName;
    qDebug() << "number of members = " << XMLAnomaly->modelMember;
    qDebug() << "number of repetitions = " << XMLAnomaly->repetitions;
    qDebug() << "anomaly year = " << XMLAnomaly->anomalyYear;
    qDebug() << "anomaly season = " << XMLAnomaly->anomalySeason;
    qDebug() << "";
}


bool readWGSettings(QString settingsFileName, WGSettings* wgSettings)
{
    QFile myFile(settingsFileName);
    if (!myFile.exists())
    {
        qDebug() << "Error!" << "\nMissing settings file:" << settingsFileName;
        return false;
    }

    QFileInfo fileInfo(settingsFileName);
    QString pathSettingsFile = fileInfo.path() + "/";


    QSettings* mySettings = new QSettings(settingsFileName, QSettings::IniFormat);
    mySettings->beginGroup("settings");

    wgSettings->climatePath = mySettings->value("climate").toString();
    if (wgSettings->climatePath.left(1) == ".")
    {
        wgSettings->climatePath = pathSettingsFile + wgSettings->climatePath;
    }

    wgSettings->observedPath = mySettings->value("observed").toString();
    if (wgSettings->observedPath.left(1) == ".")
    {
        wgSettings->observedPath = pathSettingsFile + wgSettings->observedPath;
    }

    // seasonal forecast
    wgSettings->isSeasonalForecast = false;
    QVariant myValue = mySettings->value("seasonalForecast");
    if (! myValue.isValid())
    {
        myValue = mySettings->value("seasonalPredictions");
    }
    if (myValue.isValid())
    {
        wgSettings->isSeasonalForecast = true;
        wgSettings->seasonalForecastPath = myValue.toString();
        if (wgSettings->seasonalForecastPath.left(1) == ".")
        {
            wgSettings->seasonalForecastPath = pathSettingsFile + wgSettings->seasonalForecastPath;
        }
    }

    wgSettings->outputPath = mySettings->value("output").toString();
    if (wgSettings->outputPath.left(1) == ".")
    {
        wgSettings->outputPath = QDir::cleanPath(pathSettingsFile + wgSettings->outputPath);
    }

    if (wgSettings->outputPath != "")
    {
        if (! QDir(wgSettings->outputPath).exists())
        {
            QDir().mkdir(wgSettings->outputPath);
        }
    }

    //check output directory
    QDir outputDirectory(wgSettings->outputPath);
    if (!outputDirectory.exists())
        if (!outputDirectory.mkdir(wgSettings->outputPath))
        {
            qDebug() << "Error: missing output directory" << wgSettings->outputPath;
            return false;
        }

    QString mySeparator = mySettings->value("separator").toString();
    wgSettings->valuesSeparator = mySeparator.toStdString().c_str()[0];

    wgSettings->minDataPercentage = mySettings->value("minDataPercentage").toFloat();
    wgSettings->rainfallThreshold = mySettings->value("rainfallThreshold").toFloat();

    wgSettings->firstYear = mySettings->value("firstYear").toInt();
    wgSettings->nrYears = mySettings->value("nrYears").toInt();

    return true;
}


bool WG_SeasonalForecast(WGSettings wgSettings)
{
    TXMLSeasonalAnomaly XMLAnomaly;
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
        if (!testFile->exists())
        {
            qDebug() << "ERROR: missing observed data:" << fileName;
            isOk = false;
        }

        // check xml file
        testFile = new QFile(xmlFileName);
        if (!testFile->exists())
        {
            qDebug() << "ERROR: missing seasonal forecast:" << xmlFileName;
            isOk = false;
        }

        if (isOk)
        {
            qDebug() << "\nCompute seasonal:" << fileName;

            // read SEASONAL PREDICTIONS
            if (! parseXMLSeasonal(xmlFileName, &XMLAnomaly))
                return false;

            // compute first and last day of the year of the season period
            season = XMLAnomaly.anomalySeason.toUpper();
            getDoyFromSeason(season, XMLAnomaly.anomalyYear, &wgDoy1, &wgDoy2);

            // set climate dates
            climateDateIni = Crit3DDate(1,1,XMLAnomaly.climatePeriod.yearFrom);
            climateDateFin = Crit3DDate(31, 12, XMLAnomaly.climatePeriod.yearTo);

            printSeasonalForecastInfo(&XMLAnomaly);

            // read CLIMATE data
            if ( !readMeteoDataCsv(climateFileName, wgSettings.valuesSeparator, NODATA, &climateDailyObsData) )
                return false;

            // read OBSERVED data (at least last 9 months)
            if ( !readMeteoDataCsv(observedFileName, wgSettings.valuesSeparator, NODATA, &lastYearDailyObsData) )
                return false;

            //check climate dates
            Crit3DDate climateObsFirstDate = climateDailyObsData.inputFirstDate;
            climateObsFirstDate = std::max(climateDateIni, climateObsFirstDate);

            Crit3DDate climateObsLastDate = climateDailyObsData.inputFirstDate.addDays(climateDailyObsData.dataLenght-1);
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
                if (!climateGenerator(climateDailyObsData.dataLenght, climateDailyObsData, climateObsFirstDate, climateObsLastDate, wgSettings.rainfallThreshold, wgSettings.minDataPercentage, &wGenClimate))
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

            clearInputData(&climateDailyObsData);
            clearInputData(&lastYearDailyObsData);
        }
    }
    return true;
}


bool WG_Climate(WGSettings wgSettings)
{
    TinputObsData climateDailyObsData;
    TweatherGenClimate wGenClimate;

    // iterate input files on climate
    QString fileName, climateFileName, outputFileName;
    QDir climateDirectory(wgSettings.climatePath);
    QStringList filters ("*.csv");
    QFileInfoList fileList = climateDirectory.entryInfoList (filters);
    std::vector<ToutputDailyMeteo> outputDailyData;

    for (int i = 0; i < fileList.size(); ++i)
    {
        fileName = fileList.at(i).fileName();
        climateFileName = wgSettings.climatePath + "/" + fileName;
        outputFileName = wgSettings.outputPath + "/" + fileName;

        qDebug() << "\n...Compute climate:" << fileName;

        // read CLIMATE data
        if ( !readMeteoDataCsv(climateFileName, wgSettings.valuesSeparator, NODATA, &climateDailyObsData) )
            return false;

        // check climate dates
        Crit3DDate climateObsFirstDate = climateDailyObsData.inputFirstDate;
        Crit3DDate climateObsLastDate = climateDailyObsData.inputFirstDate.addDays(climateDailyObsData.dataLenght-1);

        // weather generator - computes climate
        if (!climateGenerator(climateDailyObsData.dataLenght, climateDailyObsData, climateObsFirstDate, climateObsLastDate, wgSettings.rainfallThreshold, wgSettings.minDataPercentage, &wGenClimate))
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

            qDebug() << "\n...write output:" << outputFileName;
            writeMeteoDataCsv(outputFileName, wgSettings.valuesSeparator, outputDailyData);
        }

        clearInputData(&climateDailyObsData);
    }
    return true;
}

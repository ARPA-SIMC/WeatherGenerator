#ifndef WGPROJECT_H
#define WGPROJECT_H

    #ifndef WEATHERGENERATOR_H
        #include "weatherGenerator.h"
    #endif

    class WGSettings
    {
    public:
        QString observedPath;
        QString climatePath;
        QString seasonalForecastPath;
        QString outputPath;

        bool isSeasonalForecast;
        char valuesSeparator;
        float minDataPercentage;
        float rainfallThreshold;

        WGSettings();
    };


    void printSeasonalForecastInfo(TXMLSeasonalAnomaly *XMLAnomaly);
    bool readWGSettings(QString settingsFileName, WGSettings* wgSettings);
    bool WG_SeasonalForecast(WGSettings wgSettings);
    bool WG_Climate(WGSettings wgSettings);


#endif // WGPROJECT_H

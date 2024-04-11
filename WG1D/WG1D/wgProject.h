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

        int firstYear;
        int nrYears;

        WGSettings();
    };

    bool readWGSettings(QString settingsFileName, WGSettings &wgSettings);
    bool WG_SeasonalForecast(const WGSettings &wgSettings);
    bool WG_Climate(const WGSettings &wgSettings);


#endif // WGPROJECT_H

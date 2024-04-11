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
        QString scenarioPath;
        QString outputPath;

        bool isSeasonalForecast;
        bool isScenario;
        char valuesSeparator;
        float minDataPercentage;
        float rainfallThreshold;

        int firstYear;
        int nrYears;

        WGSettings();
    };

    bool readWGSettings(const QString &settingsFileName, WGSettings &wgSettings);
    bool WG_SeasonalForecast(const WGSettings &wgSettings);
    bool WG_Scenario(const WGSettings &wgSettings);
    bool WG_Climate(const WGSettings &wgSettings);


#endif // WGPROJECT_H

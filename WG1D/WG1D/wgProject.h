#ifndef WGPROJECT_H
#define WGPROJECT_H

    #ifndef WEATHERGENERATOR_H
        #include "weatherGenerator.h"
    #endif

    #ifndef WELL_H
        #include "well.h"
    #endif

    class WGSettings
    {
        public:
            QString observedPath;
            QString climatePath;
            QString seasonalForecastPath;
            QString scenarioPath;
            QString outputPath;
            QString waterTablePath;

            bool isSeasonalForecast;
            bool isScenario;
            bool isWaterTable;
            char valuesSeparator;
            float minDataPercentage;
            float rainfallThreshold;
            double lat;
            double lon;

            int firstYear;
            int nrYears;
            int waterTableMaximumDepth;

            std::vector<Well> wellPoints;
            WGSettings();
    };

    bool readWGSettings(const QString &settingsFileName, WGSettings &wgSettings);
    bool WG_SeasonalForecast(const WGSettings &wgSettings);
    bool WG_Scenario(const WGSettings &wgSettings);
    bool WG_Climate(const WGSettings &wgSettings);


#endif // WGPROJECT_H

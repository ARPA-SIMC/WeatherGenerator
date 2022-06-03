/*!
    \name WG1D
    \brief it generates daily time series of Tmin, Tmax, Prec (csv files)
    Modality: CLIMATE or SEASONAL_FORECAST

    CLIMATE test: uncomment #define TEST_WG_CLIMATE
    It generates one year of synthetic data for two points
    -   climate data in data/input/climate
    -   wg settings in data/testWG_climate.ini

    SEASONAL_FORECAST test: uncomment #define TEST_WG_SEASONAL
    It generates seasonal forecast (JJA 2016) for two points
    output is a time series of synthetic data for the JJA season
    and observed data of the previous 9 months
    synthetic data are computed by wg climate + forecast anomalies
    the number of output data years depending on nr. of model members in xml files
    -   climate data in data/input/climate
    -   observed data in data/input/observed
    -   seasonal forecast (xml files) in data/input/seasonalForecast_2016_JJA
    -   wg settings in data/testWG_seasonal.ini
*/

#include "wgProject.h"
#include "utilities.h"
#include <iostream>
#include <QCoreApplication>


// uncomment TEST_WG_CLIMATE or TEST_WG_SEASONAL to execute test
//#define TEST_WG_CLIMATE 1
//#define TEST_WG_SEASONAL 2


int main(int argc, char *argv[])
{
    QCoreApplication wg(argc, argv);

    QString dataPath, settingsFileName;
    if (! searchDataPath(&dataPath)) return -1;

    std::cout << "Weather Generator 1D\n";

    #ifdef TEST_WG_CLIMATE
        settingsFileName = dataPath + "testWG_Climate.ini";
    #else
        #ifdef TEST_WG_SEASONAL
            settingsFileName = dataPath + "testWG_Seasonal.ini";
        #else
            if (argc > 1)
                settingsFileName = argv[1];
            else
            {
                std::cout << "USAGE:" << std::endl;
                std::cout << "WG.exe [settings.ini]" << std::endl;
                std::cout << std::flush;
                return 0;
            }
        #endif
    #endif

    // read settings
    WGSettings wgSettings;
    if (!readWGSettings(settingsFileName, &wgSettings))
        return -1;

    if (wgSettings.isSeasonalForecast)
    {
        if (! WG_SeasonalForecast(wgSettings))
            return -1;
    }
    else
    {
        if (! WG_Climate(wgSettings))
            return -1;
    }

    std::cout << "\nEND\n";
    std::cout << std::flush;
}


/*!
    \name WG1D
    \brief it generates synthetic daily time series of Tmin, Tmax, Prec (csv files)
    Modality: CLIMATE, SEASONAL_FORECAST, SCENARIO

    CLIMATE test: uncomment #define TEST_WG_CLIMATE
    It generates one year of synthetic data for two points
    -   wg settings in DATA\TEST\testWG_climate.ini
    -   climate data in DATA\TEST\input\climate

    SEASONAL_FORECAST test: uncomment #define TEST_WG_SEASONAL
    It generates seasonal forecasts (JJA 2016) for two test points.
    The output is a time series consisting of synthetic data for the requested season
    and observed data in the other months (using data from the previous 9 months).
    The synthetic data are generated from climate (computed by wg) + predicted seasonal anomalies
    The number of years of data output depends on the number of model members in the xml files.
    -   wg settings in DATA\TEST\testWG_seasonal.ini
    -   climate data in DATA\TEST\input\climate
    -   observed data in DATA\TEST\input\observed
    -   predicted seasonal anomalies (xml files) in DATA\TEST\input\seasonalForecast_2016_JJA

    SCENARIO test: uncomment #define TEST_WG_SCENARIO
    It generates years of synthetic data referring to a test scenario for two points.
    ...
*/


#include "wgProject.h"
#include "utilities.h"
#include <iostream>
#include <QCoreApplication>


// uncomment TEST_WG_CLIMATE or TEST_WG_SEASONAL or TEST_WG_SCENARIO to execute test
//#define TEST_WG_CLIMATE 1
//#define TEST_WG_SEASONAL 2
//#define TEST_WG_SCENARIO 3
#define TEST_WG_WATERTABLE 4

void usage()
{
    std::cout << std::endl << "USAGE:" << std::endl;
    std::cout << "WG1D.exe [settingsFile.ini]" << std::endl;
    std::cout << std::flush;
}


int main(int argc, char *argv[])
{
    QCoreApplication wg(argc, argv);

    QString dataPath, settingsFileName;
    if (! searchDataPath(&dataPath)) return -1;

    std::cout << "*** 1D Weather Generator ***\n";

    #ifdef TEST_WG_CLIMATE
        settingsFileName = dataPath + "TEST/testWG_Climate.ini";
    #else
        #ifdef TEST_WG_SEASONAL
            settingsFileName = dataPath + "TEST/testWG_Seasonal.ini";
            //settingsFileName = "//icolt-smr/CRITERIA1D/PROJECTS/icolt2024_MJJ/wg/WG_2024_MJJ_C1.ini";
        #else
            #ifdef TEST_WG_SCENARIO
                settingsFileName = dataPath + "TEST/testWG_Scenario.ini";
            #else
                #ifdef TEST_WG_WATERTABLE
                    settingsFileName = dataPath + "TEST_waterTable/testWG_waterTable.ini";
                #else
                    if (argc > 1)
                        settingsFileName = argv[1];
                    else
                    {
                        usage();
                        return 0;
                    }
                #endif
            #endif
        #endif
    #endif

    // read settings
    WGSettings wgSettings;
    if (! readWGSettings(settingsFileName, wgSettings))
    {
        qDebug() << "*** Error in reading settings file!";
        return -1;
    }

    if (wgSettings.isSeasonalForecast)
    {
        if (! WG_SeasonalForecast(wgSettings))
        {
            qDebug() << "*** Error in Seasonal forecast computation!";
            return -1;
        }
    }
    else if (wgSettings.isScenario)
    {
        if (! WG_Scenario(wgSettings))
        {
            qDebug() << "*** Error in Scenario computation!";
            return -1;
        }
    }
    else
    {
        // CLIMATE
        if (! WG_Climate(wgSettings))
        {
            qDebug() << "*** Error in Climate computation!";
            return -1;
        }
    }

    std::cout << "\nEND\n";
    std::cout << std::flush;
}


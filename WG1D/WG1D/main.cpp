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


// uncomment TEST_WG_CLIMATE or TEST_WG_SEASONAL or TEST_WG_SCENARIO to execute test
//#define TEST_WG_CLIMATE 1
//#define TEST_WG_SEASONAL 2
 #define TEST_WG_SCENARIO 3

void usage()
{
    std::cout << std::endl << "USAGE:" << std::endl;
    std::cout << " WG1D.exe [settingsFile.ini]" << std::endl;
    std::cout << std::flush;
}


int main(int argc, char *argv[])
{
    QCoreApplication wg(argc, argv);

    QString dataPath, settingsFileName;
    if (! searchDataPath(&dataPath)) return -1;

    std::cout << "\n*** 1D Weather Generator ***\n";

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
        else
        {
            qDebug() << "Scenario ok!";
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


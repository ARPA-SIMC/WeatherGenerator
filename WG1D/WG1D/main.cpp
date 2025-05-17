/*!
    \name WG1D
    \brief it generates synthetic daily time series of Tmin, Tmax, Prec (csv files)
    and possibly Water table depth (if observed data are available)
    Mode: CLIMATE, SEASONAL FORECAST, SCENARIO

    CLIMATE test: uncomment #define TEST_WG_CLIMATE
    It generates ten year of synthetic data for two points
    The first year and the number of years generated are governed by the settings
    -   the settings are in the file: DATA\TEST\testWG_climate.ini
    -   climate data are in the directory: DATA\TEST\input\climate

    SEASONAL FORECAST test: uncomment #define TEST_WG_SEASONAL
    It generates seasonal forecasts (JJA 2016) for two test points.
    The output is a time series composed of synthetic data for the requested season
    and observed data in other months (using data from the previous 9 months).
    Synthetic data are generated from climate (calculated from wg) + forecasted seasonal anomalies.
    The number of years of data output depends on the number of model members in the xml files.
    -   the settings are in the file: DATA\TEST\testWG_seasonal.ini
    -   climate data are in the directory: DATA\TEST\input\climate
    -   observed data are in the directory: DATA\TEST\input\observed
    -   predicted seasonal anomalies (xml files): DATA\TEST\input\seasonalForecast_2016_JJA

    SCENARIO test: uncomment #define TEST_WG_SCENARIO
    It generates years of synthetic data referring to a test scenario for two points.
    ...

    WATERTABLE test: uncomment #define TEST_WG_WATERTABLE
    It generates seasonal forecasts (JJA 2024) for two test points, with water table depth data.
    -   the settings are in the file: DATA\TEST_waterTable\testWG_waterTable.ini
    -   climate data are in the directory: DATA\TEST_waterTable\input\climate
    -   observed data are in the directory: DATA\TEST_waterTable\input\observed
    -   observed water table detph are in the directory: DATA\TEST_waterTable\input\waterTable
    -   predicted seasonal anomalies (xml files): DATA\TEST_waterTable\input\seasonalForecast

    \copyright 2024 Fausto Tomei, Antonio Volta, Laura Costantini

    WG1D has been developed under contract issued by ARPAE Emilia-Romagna

    WG1D is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    WG1D is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with WG1D.  If not, see <http://www.gnu.org/licenses/>.

    contacts:
    ftomei@arpae.it
*/


#include "wgProject.h"
#include "utilities.h"

#include <iostream>
#include <QCoreApplication>
#include <QDir>


// uncomment to execute test:
//#define TEST_WG_CLIMATE
//#define TEST_WG_SEASONAL
//#define TEST_WG_SCENARIO
#define TEST_WG_WATERTABLE_DATA
//#define TEST_WG_WATERTABLE_DB

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

    std::cout << "WG1D - daily Weather Generator\n";
    std::cout << "execution mode: CLIMATE | SEASONAL FORECAST | SCENARIO\n";

    #ifdef TEST_WG_CLIMATE
        settingsFileName = dataPath + "TEST/testWG_Climate.ini";
    #else
        #ifdef TEST_WG_SEASONAL
            settingsFileName = dataPath + "TEST/testWG_Seasonal.ini";
            //settingsFileName = "//icolt-smr/CRITERIA1D/PROJECTS/icolt2024_MJJ/wg/WG_2024_MJJ_C1.ini";
        #else
            #ifdef TEST_WG_SCENARIO
                settingsFileName = dataPath + "TEST/testWG_Scenario.ini";
                //settingsFileName = dataPath + "TEST_scenario/testWG_Scenario.ini"; //
            #else
                #ifdef TEST_WG_WATERTABLE_DATA
                    settingsFileName = dataPath + "TEST_waterTable/testWG_waterTable_Data.ini";
                #else
                    #ifdef TEST_WG_WATERTABLE_DB
                        settingsFileName = "//icolt-smr/CRITERIA1D/PROJECTS/icolt2025_JJA/wg/WG_2025_JJA_C1.ini";
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

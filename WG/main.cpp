/*!
    \copyright 2016 Fausto Tomei

    This file is part of CRITERIA3D.
    CRITERIA3D has been developed under contract issued by ARPAE Emilia-Romagna

    CRITERIA3D is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    CRITERIA3D is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with CRITERIA3D.  If not, see <http://www.gnu.org/licenses/>.

    contacts:
    ftomei@arpae.it
*/

#include "wgProject.h"
#include <QCoreApplication>
#include <QDebug>

/*!
    \name TestWeatherGenerator (1D)
    Output: daily time series of Tmin, Tmax, Prec (csv files)
    Modality: CLIMATE or SEASONAL_FORECAST

    CLIMATE test: uncomment #define TEST_WG_CLIMATE
    It generates one year of synthetic data for two points
    -   climate data in data/input/climate
    -   wg settings in data/testWG_climate.ini

    SEASONAL_FORECAST test: uncomment #define TEST_WG_SEASONAL
    It generates seasonal forecast (JJA 2016) for two points
    Output is a time series of synthetic data for the JJA season
    and observed data of the previous 9 months
    Synthetic data are computed by wg climate + forecast anomalies
    Nr. of output data years depending on nr. of model members in xml files
    -   climate data in data/input/climate
    -   observed data in data/input/observed
    -   seasonal forecast (xml files) in data/input/seasonalForecast_2016_JJA
    -   wg settings in data/testWG_seasonal.ini
*/

// uncomment TEST_WG_CLIMATE or TEST_WG_SEASONAL to execute test
// #define TEST_WG_CLIMATE 1
// #define TEST_WG_SEASONAL 2


int main(int argc, char *argv[])
{
    QCoreApplication wg(argc, argv);

    QString appPath = wg.applicationDirPath();
    QString settingsFileName;

    qDebug() << "Weather Generator 1D";

    #ifdef TEST_WG_CLIMATE
        settingsFileName = appPath + "/../data/testWG_Climate.ini";
    #else
        #ifdef TEST_WG_SEASONAL
            settingsFileName = appPath + "/../data/testWG_Seasonal.ini";
        #else
            if (argc > 1)
                settingsFileName = argv[1];
            else
            {
                qDebug() << "ERROR: missing .ini file";
                qDebug() << "\nUSAGE\nWG.exe [settings.ini]\n";
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

    qDebug() << "\nEND\n";
}


#include <math.h>

#include "commonConstants.h"
#include "basicMath.h"
#include "statistics.h"
#include "wgClimate.h"
#include "weatherGenerator.h"
#include "timeUtility.h"
#include "crit3dDate.h"
#include <iostream>
#include <QFile>
#include <QTextStream>

using namespace std;


/*!
  * \brief Compute climate (monthly values)
  * \returns true if the input data are valid
  * \param  nrDays          [-] number of data (366 x n where n is the number of years)
  * \param  inputFirstDate  [Crit3DDate]
  * \param  *inputTMin      [°C] array(1..nrDays) of minimum temperature
  * \param  *inputTMax      [°C] array(1..nrDays) of maximum temperature
  * \param  *inputPrec      [mm] array(1..nrDays) of precipitation
*/
bool computeWGClimate(int nrDays, Crit3DDate inputFirstDate, const std::vector<float>& inputTMin,
                      const std::vector<float>& inputTMax, const std::vector<float>& inputPrec,
                      float precThreshold, float minDataPercentage, TweatherGenClimate* wGen,
                      bool isWriteOutput, const QString& outputFileName)
{
    double sumPrec[12] = {0};
    long nWetDays[12] = {0};
    long nDryDays[12] = {0};

    long nrData[12] = {0};
    double sumTMin[12] = {0};
    double sumTMax[12] = {0};
    double sumTmaxWet[12] = {0};
    double sumTmaxDry[12] = {0};
    double sumTminWet[12] = {0};
    double sumTminDry[12] = {0};
    double sum2TmaxWet[12] = {0};
    double sum2TmaxDry[12] = {0};
    double sum2TminWet[12] = {0};
    double sum2TminDry[12] = {0};

    float maxTmaxWet[12] = {-9999,-9999,-9999,-9999,-9999,-9999,-9999,-9999,-9999,-9999,-9999,-9999};
    float maxTmaxDry[12] = {-9999,-9999,-9999,-9999,-9999,-9999,-9999,-9999,-9999,-9999,-9999,-9999};
    float maxTminWet[12] = {-9999,-9999,-9999,-9999,-9999,-9999,-9999,-9999,-9999,-9999,-9999,-9999};
    float maxTminDry[12] = {-9999,-9999,-9999,-9999,-9999,-9999,-9999,-9999,-9999,-9999,-9999,-9999};

    float minTmaxWet[12] = {9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999};
    float minTmaxDry[12] = {9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999};
    float minTminWet[12] = {9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999};
    float minTminDry[12] = {9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999,9999};

    int nCheckedDays = NRDAYS_MAXDRYINCREASE * 2 + 1;
    std::vector<std::vector<int>> nConsecutiveWetDays(12, std::vector<int>(nCheckedDays, 0));
    std::vector<std::vector<int>> nConsecutiveDryDays(12, std::vector<int>(nCheckedDays, 0));
    std::vector<float> x_index(nCheckedDays, 0);
    for (int i=0; i < nCheckedDays; i++)
        x_index[i] = float(i);

    // initialize climate
    for (int m = 0; m < 12; m++)
    {
        wGen->monthly.sumPrec[m] = NODATA;
        wGen->monthly.fractionWetDays[m] = NODATA;
        wGen->monthly.probabilityWetWet[m] = NODATA;
        wGen->monthly.dryProbabilityIncrease[m] = NODATA;
        wGen->monthly.wetProbabilityIncrease[m] = NODATA;

        wGen->monthly.monthlyTmax[m] = NODATA;
        wGen->monthly.monthlyTmin[m] = NODATA;
        wGen->monthly.monthlyTmaxDry[m] = NODATA;
        wGen->monthly.monthlyTmaxWet[m] = NODATA;
        wGen->monthly.monthlyTminDry[m] = NODATA;
        wGen->monthly.monthlyTminWet[m] = NODATA;
        wGen->monthly.stDevTmaxDry[m] = NODATA;
        wGen->monthly.stDevTmaxWet[m] = NODATA;
        wGen->monthly.stDevTminDry[m] = NODATA;
        wGen->monthly.stDevTminWet[m] = NODATA;
        wGen->monthly.dw_Tmax[m] = NODATA;
        wGen->monthly.dw_Tmin[m] = NODATA;

        wGen->monthly.maxTminDry[m] = NODATA;
        wGen->monthly.maxTminWet[m] = NODATA;
        wGen->monthly.minTminDry[m] = NODATA;
        wGen->monthly.minTminWet[m] = NODATA;
        wGen->monthly.maxTmaxDry[m] = NODATA;
        wGen->monthly.maxTmaxWet[m] = NODATA;
        wGen->monthly.minTmaxDry[m] = NODATA;
        wGen->monthly.minTmaxWet[m] = NODATA;
    }

    // read data
    long nrValidData = 0;
    int consecutiveDryDays = 0;
    int consecutiveWetDays = 0;
    Crit3DDate myDate = inputFirstDate;
    int m_start = myDate.month - 1;

    for (int n = 0; n < nrDays; n++)
    {
        int m = myDate.month - 1;

        // the day is valid if all values are different from nodata
        if (int(inputTMin[n]) != int(NODATA)
            && int(inputTMax[n]) != int(NODATA)
            && int(inputPrec[n]) != int(NODATA))
        {
            nrValidData++;
            nrData[m]++;
            sumTMin[m] += double(inputTMin[n]);
            sumTMax[m] += double(inputTMax[n]);
            sumPrec[m] += double(inputPrec[n]);

            if (inputPrec[n] > precThreshold)
            {
                // wet
                ++nWetDays[m];
                sumTmaxWet[m] += double(inputTMax[n]);
                sumTminWet[m] += double(inputTMin[n]);
                sum2TmaxWet[m] += double(inputTMax[n] * inputTMax[n]);
                sum2TminWet[m] += double(inputTMin[n] * inputTMin[n]);

                if (consecutiveWetDays == 0)
                {
                    // new series
                    consecutiveDryDays = 0;
                    m_start = m;
                }

                ++consecutiveWetDays;

                // update wet periods
                for (int i = 0; i < nCheckedDays; i++)
                    if (i < consecutiveWetDays)
                    //if (i == consecutiveWetDays-1)
                        nConsecutiveWetDays[m_start][i]++;

                maxTmaxWet[m] = MAXVALUE(maxTmaxWet[m],inputTMax[n]);
                maxTminWet[m] = MAXVALUE(maxTminWet[m],inputTMin[n]);
                minTmaxWet[m] = MINVALUE(minTmaxWet[m],inputTMax[n]);
                minTminWet[m] = MINVALUE(minTminWet[m],inputTMin[n]);
            }
            else
            {
                // dry
                nDryDays[m]++;
                sumTmaxDry[m] += double(inputTMax[n]);
                sumTminDry[m] += double(inputTMin[n]);
                sum2TmaxDry[m] += double(inputTMax[n] * inputTMax[n]);
                sum2TminDry[m] += double(inputTMin[n] * inputTMin[n]);

                if (consecutiveDryDays == 0)
                {
                    // new series
                    consecutiveWetDays = 0;
                    m_start = m;
                }

                ++consecutiveDryDays;

                // update dry periods
                for (int i = 0; i < nCheckedDays; i++)
                    if (i < consecutiveDryDays)
                    //if (i == consecutiveDryDays-1)
                        nConsecutiveDryDays[m_start][i]++;

                maxTmaxDry[m] = MAXVALUE(maxTmaxDry[m],inputTMax[n]);
                maxTminDry[m] = MAXVALUE(maxTminDry[m],inputTMin[n]);
                minTmaxDry[m] = MINVALUE(minTmaxDry[m],inputTMax[n]);
                minTminDry[m] = MINVALUE(minTminDry[m],inputTMin[n]);
            }
        }

        ++myDate;
    }

    // check on data presence
    float dataPercentage = float(nrValidData) / float(nrDays);
    if (dataPercentage < minDataPercentage)
        return false;

    // compute Climate
    for (int m = 0; m < 12; m++)
    {
        if (nrData[m] > 0)
        {
            wGen->monthly.monthlyTmax[m] = sumTMax[m] / nrData[m];  // computes mean monthly values of maximum temperature
            wGen->monthly.monthlyTmin[m] = sumTMin[m] / nrData[m];  // computes mean monthly values of minimum temperature

            if (nDryDays[m] > 0)
            {
                wGen->monthly.monthlyTmaxDry[m] = sumTmaxDry[m] / nDryDays[m];
                wGen->monthly.monthlyTminDry[m] = sumTminDry[m] / nDryDays[m];
                wGen->monthly.stDevTmaxDry[m] = sqrt(MAXVALUE(nDryDays[m]*sum2TmaxDry[m] - sumTmaxDry[m]*sumTmaxDry[m], 0)
                                                     / (nDryDays[m]*(nDryDays[m]-1)));
                wGen->monthly.stDevTminDry[m] = sqrt(MAXVALUE(nDryDays[m]*sum2TminDry[m] - sumTminDry[m]*sumTminDry[m], 0)
                                                     / (nDryDays[m]*(nDryDays[m]-1)));
            }

            if (nWetDays[m] > 0)
            {
                wGen->monthly.monthlyTmaxWet[m] = sumTmaxWet[m] / nWetDays[m];
                wGen->monthly.monthlyTminWet[m] = sumTminWet[m] / nWetDays[m];
                wGen->monthly.stDevTmaxWet[m] = sqrt(MAXVALUE(nWetDays[m]*sum2TmaxWet[m] - sumTmaxWet[m]*sumTmaxWet[m], 0)
                                                     / (nWetDays[m]*(nWetDays[m]-1)));
                wGen->monthly.stDevTminWet[m] = sqrt(MAXVALUE(nWetDays[m]*sum2TminWet[m] - sumTminWet[m]*sumTminWet[m], 0)
                                                     / (nWetDays[m]*(nWetDays[m]-1)));
            }

            if (nDryDays[m] > 0 && nWetDays[m] > 0)
            {
                wGen->monthly.dw_Tmax[m] = (sumTmaxDry[m] / nDryDays[m]) - (sumTmaxWet[m] / nWetDays[m]);
                wGen->monthly.dw_Tmin[m] = (sumTminDry[m] / nDryDays[m]) - (sumTminWet[m] / nWetDays[m]);
            }
            else
            {
                wGen->monthly.dw_Tmax[m] = 0;
                wGen->monthly.dw_Tmin[m] = 0;
            }

            int daysInMonth = getDaysInMonth(m+1,2001);                 // year = 2001 is to avoid leap year

            wGen->monthly.sumPrec[m] = sumPrec[m] / nrData[m] * daysInMonth;
            wGen->monthly.fractionWetDays[m] = float(nWetDays[m]) / float(nrData[m]);

            // [-] probability of a dry day after a dry day
            std::vector<float> probabilityDryDay(nCheckedDays, 0.);
            for (int i=0; i < (nCheckedDays-1); i++)
            {
                if (nConsecutiveDryDays[m][i] > 0)
                {
                    probabilityDryDay[i] = float(nConsecutiveDryDays[m][i+1]) / float(nConsecutiveDryDays[m][i]);
                    probabilityDryDay[i] = std::min(1.f, probabilityDryDay[i]);
                }
            }

            // [-] probability of a wet day after a wet day
            std::vector<float> probabilityWetDay(nCheckedDays, 0.);
            for (int i=0; i < (nCheckedDays-1); i++)
            {
                if (nConsecutiveWetDays[m][i] > 0)
                {
                    probabilityWetDay[i] = float(nConsecutiveWetDays[m][i+1]) / float(nConsecutiveWetDays[m][i]);
                    probabilityWetDay[i] = std::min(1.f, probabilityWetDay[i]);
                }
            }
            wGen->monthly.probabilityWetWet[m] = probabilityWetDay[0];

            /*
            averagedVectorProbability(probabilityDryDay);
            normalizedDryPeriod[m] = computeNormalizedDryDayLength(probabilityDryDay);
            ratioDryDaysWetDays[m] = (float(nrData[m]) - float(nWetDays[m])) / float(nWetDays[m]);
            correctedProbabilityPwd[m] = (1 - wGen->monthly.probabilityWetWet[m]) * ratioDryDaysWetDays[m]/normalizedDryPeriod[m];
            */

            if (wGen->isDryWetPeriodsComputation)
            {
                float intercept, slope, r2;
                statistics::linearRegression(x_index, probabilityDryDay, NRDAYS_MAXDRYINCREASE+1, false, &intercept, &slope, &r2);
                if (r2 < 0.1 || slope < 0.001)
                    wGen->monthly.dryProbabilityIncrease[m] = 0.;
                else
                {
                    wGen->monthly.dryProbabilityIncrease[m] = slope;
                }

                wGen->monthly.wetProbabilityIncrease[m] = wGen->monthly.dryProbabilityIncrease[m] / wGen->monthly.fractionWetDays[m];
            }
            else
            {
                wGen->monthly.dryProbabilityIncrease[m] = 0.;
                wGen->monthly.wetProbabilityIncrease[m] = 0.;
            }
        }

        wGen->monthly.maxTminDry[m] = maxTminDry[m];
        wGen->monthly.maxTminWet[m] = maxTminWet[m];
        wGen->monthly.minTminDry[m] = minTminDry[m];
        wGen->monthly.minTminWet[m] = minTminWet[m];
        wGen->monthly.maxTmaxDry[m] = maxTmaxDry[m];
        wGen->monthly.maxTmaxWet[m] = maxTmaxWet[m];
        wGen->monthly.minTmaxDry[m] = minTmaxDry[m];
        wGen->monthly.minTmaxWet[m] = minTmaxWet[m];
    }

    if (isWriteOutput)
    {
        cout << "...Write WG climate file -->" << outputFileName.toStdString() << "\n";

        QFile file(outputFileName);
        file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);

        QTextStream stream( &file );
        stream << "----------------- CLIMATE ----------------\n";
        for (int m = 0; m < 12; m++)
        {
            stream << "month = " << m +1 << "\n";
            stream << "wGen->monthly.monthlyTmin = " << wGen->monthly.monthlyTmin[m] << "\n";
            stream << "wGen->monthly.monthlyTmax = " << wGen->monthly.monthlyTmax[m] << "\n";
            stream << "wGen->monthly.sumPrec = " << wGen->monthly.sumPrec[m] << "\n";
            stream << "wGen->monthly.fractionWetDays = " << wGen->monthly.fractionWetDays[m] << "\n";
            stream << "wGen->monthly.probabilityWetWet = " << wGen->monthly.probabilityWetWet[m] << "\n";
            stream << "wGen->monthly.dryProbabilityIncrease = " << wGen->monthly.dryProbabilityIncrease[m] << "\n";
            stream << "wGen->monthly.stdDevTmin (dry) = " << wGen->monthly.stDevTminDry[m] << "\n";
            stream << "wGen->monthly.stdDevTmin (wet) = " << wGen->monthly.stDevTminWet[m] << "\n";
            stream << "wGen->monthly.stdDevTmax (dry) = " << wGen->monthly.stDevTmaxDry[m] << "\n";
            stream << "wGen->monthly.stdDevTmax (wet) = " << wGen->monthly.stDevTmaxWet[m] << "\n";
            stream << "wGen->monthly.dw_Tmax = " << wGen->monthly.dw_Tmax[m] << "\n";
            stream << "wGen->monthly.dw_Tmin = " << wGen->monthly.dw_Tmin[m] << "\n";
            stream << "wGen->monthly.monthlyTminDry = " << wGen->monthly.monthlyTminDry[m] << "\n";
            stream << "wGen->monthly.monthlyTmaxDry = " << wGen->monthly.monthlyTmaxDry[m] << "\n";
            stream << "wGen->monthly.monthlyTminWet = " << wGen->monthly.monthlyTminWet[m] << "\n";
            stream << "wGen->monthly.monthlyTmaxWet = " << wGen->monthly.monthlyTmaxWet[m] << "\n";

            stream << "-------------------------------------------" << "\n";
        }
    }

    return true;
}


bool computeWG2DClimate(int nrDays, Crit3DDate inputFirstDate, float *inputTMin, float *inputTMax,
                        float *inputPrec, float precThreshold, float minPrecData,
                        TweatherGenClimate* wGen, bool writeOutput,bool outputForStats, QString outputFileName,
                        float* monthlyPrecipitation, float** consecutiveDry, float** consecutiveWet,
                        int nrConsecutiveDryDaysBins)
{
    long nValidData = 0;
    float dataPresence = 0;
    double sumTMin[12] = {0};
    double sumTMax[12] = {0};
    double sumPrec[12] = {0};
    double sumTMin2[12] = {0};
    double sumTMax2[12] = {0};
    long nWetDays[12] = {0};
    long nWetWetDays[12] = {0};
    long nDryDays[12] = {0};
    long nrData[12] = {0};
    double sumTmaxWet[12] = {0};
    double sumTmaxDry[12] = {0};
    double sumTminWet[12] = {0};
    double sumTminDry[12] = {0};
    double sumTmaxWet2[12] = {0};
    double sumTmaxDry2[12] = {0};
    double sumTminWet2[12] = {0};
    double sumTminDry2[12] = {0};
    int daysInMonth;
    bool isPreviousDayWet = false;

    // read data
    int m;
    Crit3DDate myDate = inputFirstDate;
    for (int n = 0; n < nrDays; n++)
    {
        m = myDate.month - 1;

        // the day is valid if all values are different from nodata
        if (int(inputTMin[n]) != int(NODATA)
            && int(inputTMax[n]) != int(NODATA)
            && int(inputPrec[n]) != int(NODATA))
        {
            nValidData++;
            nrData[m]++;
            sumTMin[m] += double(inputTMin[n]);
            sumTMin2[m] += double(inputTMin[n] * inputTMin[n]);
            sumTMax[m] += double(inputTMax[n]);
            sumTMax2[m] += double(inputTMax[n] * inputTMax[n]);
            sumPrec[m] += double(inputPrec[n]);

            if (inputPrec[n] > precThreshold)
            {
                if (isPreviousDayWet) nWetWetDays[m]++;
                nWetDays[m]++;
                sumTmaxWet[m] += double(inputTMax[n]);
                sumTminWet[m] += double(inputTMin[n]);
                sumTmaxWet2[m] += double(inputTMax[n] * inputTMax[n]);
                sumTminWet2[m] += double(inputTMin[n] * inputTMin[n]);
                isPreviousDayWet = true;
            }
            else
            {
                nDryDays[m]++;
                sumTmaxDry[m] += double(inputTMax[n]);
                sumTminDry[m] += double(inputTMin[n]);
                sumTmaxDry2[m] += double(inputTMax[n] * inputTMax[n]);
                sumTminDry2[m] += double(inputTMin[n] * inputTMin[n]);
                isPreviousDayWet = false;
            }
        }

        ++myDate;
    }

    dataPresence = float(nValidData) / float(nrDays);
    if (dataPresence < minPrecData)
        return false;

    // compute Climate
    for (m=0; m<12; m++)
    {
        if (nrData[m] > 0)
        {
            wGen->monthly.monthlyTmax[m] = sumTMax[m] / nrData[m];      // computes mean monthly values of maximum temperature
            wGen->monthly.monthlyTmin[m] = sumTMin[m] / nrData[m];      // computes mean monthly values of minimum temperature
            wGen->monthly.monthlyTmaxDry[m] = sumTmaxDry[m] / nDryDays[m];
            wGen->monthly.monthlyTmaxWet[m] = sumTmaxWet[m] / nWetDays[m];
            wGen->monthly.monthlyTminDry[m] = sumTminDry[m] / nDryDays[m];
            wGen->monthly.monthlyTminWet[m] = sumTminWet[m] / nWetDays[m];

            daysInMonth = getDaysInMonth(m+1,2001); // year = 2001 is to avoid leap year

            wGen->monthly.sumPrec[m] = sumPrec[m] / nrData[m] * daysInMonth;

            wGen->monthly.fractionWetDays[m] = float(nWetDays[m]) / float(nrData[m]);
            wGen->monthly.probabilityWetWet[m] = float(nWetWetDays[m]) / float(nWetDays[m]);

            if ((nDryDays[m] > 0) && (nWetDays[m] > 0))
            {
                wGen->monthly.dw_Tmax[m] = (sumTmaxDry[m] / nDryDays[m]) - (sumTmaxWet[m] / nWetDays[m]);
                wGen->monthly.dw_Tmin[m] = (sumTminDry[m] / nDryDays[m]) - (sumTminWet[m] / nWetDays[m]);
            }
            else
            {
                wGen->monthly.dw_Tmax[m] = 0;
                wGen->monthly.dw_Tmin[m] = 0;
            }

            wGen->monthly.stDevTmaxDry[m] = sqrt(MAXVALUE(nDryDays[m]*sumTmaxDry2[m]-(sumTmaxDry[m]*sumTmaxDry[m]), 0) / (nDryDays[m]*(nDryDays[m]-1)));
            wGen->monthly.stDevTmaxWet[m] = sqrt(MAXVALUE(nWetDays[m]*sumTmaxWet2[m]-(sumTmaxWet[m]*sumTmaxWet[m]), 0) / (nWetDays[m]*(nWetDays[m]-1)));
            wGen->monthly.stDevTminDry[m] = sqrt(MAXVALUE(nDryDays[m]*sumTminDry2[m]-(sumTminDry[m]*sumTminDry[m]), 0) / (nDryDays[m]*(nDryDays[m]-1)));
            wGen->monthly.stDevTminWet[m] = sqrt(MAXVALUE(nWetDays[m]*sumTminWet2[m]-(sumTminWet[m]*sumTminWet[m]), 0) / (nWetDays[m]*(nWetDays[m]-1)));
        }
        else
        {
            wGen->monthly.monthlyTmax[m] = NODATA;
            wGen->monthly.monthlyTmin[m] = NODATA;
            wGen->monthly.sumPrec[m] = NODATA;
            wGen->monthly.fractionWetDays[m] = NODATA;
            wGen->monthly.dw_Tmax[m] = NODATA;
            wGen->monthly.dw_Tmin[m] = NODATA;
            wGen->monthly.stDevTmaxDry[m] = NODATA;
            wGen->monthly.stDevTminDry[m] = NODATA;
            wGen->monthly.stDevTmaxWet[m] = NODATA;
            wGen->monthly.stDevTminWet[m] = NODATA;
        }

        monthlyPrecipitation[m] = wGen->monthly.sumPrec[m];
    }

    if (writeOutput)
    {
        if (!outputForStats)
        {
            cout << "...Write WG climate file -->" << outputFileName.toStdString() << "\n";

            QFile file(outputFileName);
            file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);

            QTextStream stream( &file );
            stream << "----------------- CLIMATE ----------------\n";
            for (m=0; m<12; m++)
            {
                stream << "month = " << m +1 << "\n";
                stream << "wGen->monthly.monthlyTmin = " << wGen->monthly.monthlyTmin[m] << "\n";
                stream << "wGen->monthly.monthlyTmax = " << wGen->monthly.monthlyTmax[m] << "\n";
                stream << "wGen->monthly.sumPrec = " << wGen->monthly.sumPrec[m] << "\n";
                stream << "wGen->monthly.fractionWetDays = " << wGen->monthly.fractionWetDays[m] << "\n";
                stream << "wGen->monthly.probabilityWetWet = " << wGen->monthly.probabilityWetWet[m] << "\n";
                stream << "wGen->monthly.dw_Tmax = " << wGen->monthly.dw_Tmax[m] << "\n";
                stream << "wGen->monthly.dw_Tmin = " << wGen->monthly.dw_Tmin[m] << "\n";
                stream << "wGen->monthly.monthlyTminDry = " << wGen->monthly.monthlyTminDry[m] << "\n";
                stream << "wGen->monthly.monthlyTmaxDry = " << wGen->monthly.monthlyTmaxDry[m] << "\n";
                stream << "wGen->monthly.monthlyTminWet = " << wGen->monthly.monthlyTminWet[m] << "\n";
                stream << "wGen->monthly.monthlyTmaxWet = " << wGen->monthly.monthlyTmaxWet[m] << "\n";
                stream << "wGen->monthly.stdDevTminDry = " << wGen->monthly.stDevTminDry[m] << "\n";
                stream << "wGen->monthly.stdDevTmaxDry = " << wGen->monthly.stDevTmaxDry[m] << "\n";
                stream << "wGen->monthly.stdDevTminWet = " << wGen->monthly.stDevTminWet[m] << "\n";
                stream << "wGen->monthly.stdDevTmaxWet = " << wGen->monthly.stDevTmaxWet[m] << "\n";

                stream << "-------------------------------------------" << "\n";
            }
        }
        else
        {
            cout << "...Write WG climate file -->" << outputFileName.toStdString() << "\n";

            QFile file(outputFileName);
            file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);

            QTextStream stream( &file );
            for (m=0; m<12; m++)
            {
                stream << m +1 << "\n";
                stream <<  wGen->monthly.monthlyTmin[m] << "\n";
                stream <<  wGen->monthly.monthlyTmax[m] << "\n";
                stream <<  wGen->monthly.sumPrec[m] << "\n";
                stream <<  wGen->monthly.fractionWetDays[m] << "\n";
                stream <<  wGen->monthly.probabilityWetWet[m] << "\n";
                stream <<  wGen->monthly.dw_Tmax[m] << "\n";
                stream <<  wGen->monthly.dw_Tmin[m] << "\n";
                stream <<  wGen->monthly.monthlyTminDry[m] << "\n";
                stream <<  wGen->monthly.monthlyTmaxDry[m] << "\n";
                stream <<  wGen->monthly.monthlyTminWet[m] << "\n";
                stream <<  wGen->monthly.monthlyTmaxWet[m] << "\n";
                stream << wGen->monthly.stDevTminDry[m] << "\n";
                stream << wGen->monthly.stDevTmaxDry[m] << "\n";
                stream << wGen->monthly.stDevTminWet[m] << "\n";
                stream << wGen->monthly.stDevTmaxWet[m] << "\n";

                for (int iBin=0;iBin<nrConsecutiveDryDaysBins;iBin++)
                {
                    stream << consecutiveDry[m][iBin] << "\t" ;
                }
                stream << "\n";
                for (int iBin=0;iBin<nrConsecutiveDryDaysBins;iBin++)
                {
                    stream << consecutiveWet[m][iBin] << "\t" ;
                }
                stream << "\n";
            }
        }
    }

    return true;
}


bool computeWG2DClimate(int nrDays, Crit3DDate inputFirstDate, float *inputTMin, float *inputTMax,
                      float *inputPrec, float precThreshold, float minPrecData,
                      TweatherGenClimate* wGen, bool writeOutput,bool outputForStats, QString outputFileName,
                      float* monthlyPrecipitation)
{
    long nValidData = 0;
    float dataPresence = 0;
    double sumTMin[12] = {0};
    double sumTMax[12] = {0};
    double sumPrec[12] = {0};
    double sumTMin2[12] = {0};
    double sumTMax2[12] = {0};
    long nWetDays[12] = {0};
    long nWetWetDays[12] = {0};
    long nDryDays[12] = {0};
    long nrData[12] = {0};
    double sumTmaxWet[12] = {0};
    double sumTmaxDry[12] = {0};
    double sumTminWet[12] = {0};
    double sumTminDry[12] = {0};
    double sumTmaxWet2[12] = {0};
    double sumTmaxDry2[12] = {0};
    double sumTminWet2[12] = {0};
    double sumTminDry2[12] = {0};
    int daysInMonth;
    bool isPreviousDayWet = false;

    // read data
    int m;
    Crit3DDate myDate = inputFirstDate;
    for (int n = 0; n < nrDays; n++)
    {
        m = myDate.month - 1;

        // the day is valid if all values are different from nodata
        if (int(inputTMin[n]) != int(NODATA)
            && int(inputTMax[n]) != int(NODATA)
            && int(inputPrec[n]) != int(NODATA))
        {
            nValidData++;
            nrData[m]++;
            sumTMin[m] += double(inputTMin[n]);
            sumTMin2[m] += double(inputTMin[n] * inputTMin[n]);
            sumTMax[m] += double(inputTMax[n]);
            sumTMax2[m] += double(inputTMax[n] * inputTMax[n]);
            sumPrec[m] += double(inputPrec[n]);

            if (inputPrec[n] > precThreshold)
            {
                if (isPreviousDayWet) nWetWetDays[m]++;
                nWetDays[m]++;
                sumTmaxWet[m] += double(inputTMax[n]);
                sumTminWet[m] += double(inputTMin[n]);
                sumTmaxWet2[m] += double(inputTMax[n] * inputTMax[n]);
                sumTminWet2[m] += double(inputTMin[n] * inputTMin[n]);
                isPreviousDayWet = true;
            }
            else
            {
                nDryDays[m]++;
                sumTmaxDry[m] += double(inputTMax[n]);
                sumTminDry[m] += double(inputTMin[n]);
                sumTmaxDry2[m] += double(inputTMax[n] * inputTMax[n]);
                sumTminDry2[m] += double(inputTMin[n] * inputTMin[n]);
                isPreviousDayWet = false;
            }
        }

        ++myDate;
    }

    dataPresence = float(nValidData) / float(nrDays);
    if (dataPresence < minPrecData)
        return false;

    // compute Climate
    for (m=0; m<12; m++)
    {
        if (nrData[m] > 0)
        {
            wGen->monthly.monthlyTmax[m] = sumTMax[m] / nrData[m]; //computes mean monthly values of maximum temperature
            wGen->monthly.monthlyTmin[m] = sumTMin[m] / nrData[m]; //computes mean monthly values of minimum temperature
            wGen->monthly.monthlyTmaxDry[m] = sumTmaxDry[m] / nDryDays[m];
            wGen->monthly.monthlyTmaxWet[m] = sumTmaxWet[m] / nWetDays[m];
            wGen->monthly.monthlyTminDry[m] = sumTminDry[m] / nDryDays[m];
            wGen->monthly.monthlyTminWet[m] = sumTminWet[m] / nWetDays[m];

            daysInMonth = getDaysInMonth(m+1,2001); // year = 2001 is to avoid leap year

            wGen->monthly.sumPrec[m] = sumPrec[m] / nrData[m] * daysInMonth;

            wGen->monthly.fractionWetDays[m] = float(nWetDays[m]) / float(nrData[m]);
            wGen->monthly.probabilityWetWet[m] = float(nWetWetDays[m]) / float(nWetDays[m]);

            if (nDryDays[m] > 0 && nWetDays[m] > 0)
            {
                wGen->monthly.dw_Tmax[m] = (sumTmaxDry[m] / nDryDays[m]) - (sumTmaxWet[m] / nWetDays[m]);
                wGen->monthly.dw_Tmin[m] = (sumTminDry[m] / nDryDays[m]) - (sumTminWet[m] / nWetDays[m]);
            }
            else
            {
                wGen->monthly.dw_Tmax[m] = 0;
                wGen->monthly.dw_Tmin[m] = 0;
            }

            wGen->monthly.stDevTmaxDry[m] = sqrt(MAXVALUE(nDryDays[m]*sumTmaxDry2[m]-(sumTmaxDry[m]*sumTmaxDry[m]), 0) / (nDryDays[m]*(nDryDays[m]-1)));
            wGen->monthly.stDevTmaxWet[m] = sqrt(MAXVALUE(nWetDays[m]*sumTmaxWet2[m]-(sumTmaxWet[m]*sumTmaxWet[m]), 0) / (nWetDays[m]*(nWetDays[m]-1)));
            wGen->monthly.stDevTminDry[m] = sqrt(MAXVALUE(nDryDays[m]*sumTminDry2[m]-(sumTminDry[m]*sumTminDry[m]), 0) / (nDryDays[m]*(nDryDays[m]-1)));
            wGen->monthly.stDevTminWet[m] = sqrt(MAXVALUE(nWetDays[m]*sumTminWet2[m]-(sumTminWet[m]*sumTminWet[m]), 0) / (nWetDays[m]*(nWetDays[m]-1)));
        }
        else
        {
            wGen->monthly.monthlyTmax[m] = NODATA;
            wGen->monthly.monthlyTmin[m] = NODATA;
            wGen->monthly.sumPrec[m] = NODATA;
            wGen->monthly.fractionWetDays[m] = NODATA;
            wGen->monthly.dw_Tmax[m] = NODATA;
            wGen->monthly.dw_Tmin[m] = NODATA;
            wGen->monthly.stDevTmaxDry[m] = NODATA;
            wGen->monthly.stDevTminDry[m] = NODATA;
            wGen->monthly.stDevTmaxWet[m] = NODATA;
            wGen->monthly.stDevTminWet[m] = NODATA;
        }
        monthlyPrecipitation[m] = wGen->monthly.sumPrec[m];
    }


    if (writeOutput)
    {
        if (!outputForStats)
        {
            cout << "...Write WG climate file -->" << outputFileName.toStdString() << "\n";

            QFile file(outputFileName);
            file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);

            QTextStream stream( &file );
            stream << "----------------- CLIMATE ----------------\n";
            for (m=0; m<12; m++)
            {
                stream << "month = " << m +1 << "\n";
                stream << "wGen->monthly.monthlyTmin = " << wGen->monthly.monthlyTmin[m] << "\n";
                stream << "wGen->monthly.monthlyTmax = " << wGen->monthly.monthlyTmax[m] << "\n";
                stream << "wGen->monthly.sumPrec = " << wGen->monthly.sumPrec[m] << "\n";
                stream << "wGen->monthly.fractionWetDays = " << wGen->monthly.fractionWetDays[m] << "\n";
                stream << "wGen->monthly.probabilityWetWet = " << wGen->monthly.probabilityWetWet[m] << "\n";
                stream << "wGen->monthly.dw_Tmax = " << wGen->monthly.dw_Tmax[m] << "\n";
                stream << "wGen->monthly.dw_Tmin = " << wGen->monthly.dw_Tmin[m] << "\n";
                stream << "wGen->monthly.monthlyTminDry = " << wGen->monthly.monthlyTminDry[m] << "\n";
                stream << "wGen->monthly.monthlyTmaxDry = " << wGen->monthly.monthlyTmaxDry[m] << "\n";
                stream << "wGen->monthly.monthlyTminWet = " << wGen->monthly.monthlyTminWet[m] << "\n";
                stream << "wGen->monthly.monthlyTmaxWet = " << wGen->monthly.monthlyTmaxWet[m] << "\n";
                stream << "wGen->monthly.stdDevTminDry = " << wGen->monthly.stDevTminDry[m] << "\n";
                stream << "wGen->monthly.stdDevTmaxDry = " << wGen->monthly.stDevTmaxDry[m] << "\n";
                stream << "wGen->monthly.stdDevTminWet = " << wGen->monthly.stDevTminWet[m] << "\n";
                stream << "wGen->monthly.stdDevTmaxWet = " << wGen->monthly.stDevTmaxWet[m] << "\n";

                stream << "-------------------------------------------" << "\n";
            }
        }
        else
        {
            cout << "...Write WG climate file -->" << outputFileName.toStdString() << "\n";

            QFile file(outputFileName);
            file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);

            QTextStream stream( &file );
            for (m=0; m<12; m++)
            {
                stream << m +1 << "\n";
                stream <<  wGen->monthly.monthlyTmin[m] << "\n";
                stream <<  wGen->monthly.monthlyTmax[m] << "\n";
                stream <<  wGen->monthly.sumPrec[m] << "\n";
                stream <<  wGen->monthly.fractionWetDays[m] << "\n";
                stream <<  wGen->monthly.probabilityWetWet[m] << "\n";
                stream <<  wGen->monthly.dw_Tmax[m] << "\n";
                stream <<  wGen->monthly.dw_Tmin[m] << "\n";
                stream <<  wGen->monthly.monthlyTminDry[m] << "\n";
                stream <<  wGen->monthly.monthlyTmaxDry[m] << "\n";
                stream <<  wGen->monthly.monthlyTminWet[m] << "\n";
                stream <<  wGen->monthly.monthlyTmaxWet[m] << "\n";
                stream << wGen->monthly.stDevTminDry[m] << "\n";
                stream << wGen->monthly.stDevTmaxDry[m] << "\n";
                stream << wGen->monthly.stDevTminWet[m] << "\n";
                stream << wGen->monthly.stDevTmaxWet[m] << "\n";
            }
        }
    }

    return true;
}


/*!
  * \brief Generates a climate starting from daily weather
  */
bool climateGenerator(int nrData, TinputObsData climateDailyObsData, Crit3DDate climateDateIni,
                      Crit3DDate climateDateFin, float precThreshold, float minDataPercentage,
                      TweatherGenClimate* wGen, bool isWriteOutput, const QString &outputFileName)
{
    int startIndex = difference(climateDailyObsData.inputFirstDate, climateDateIni);  // starts from 0
    int nrDays = difference(climateDateIni, climateDateFin)+1;

    TinputObsData dailyObsData;
    dailyObsData.inputFirstDate = climateDateIni;
    dailyObsData.inputTMin.resize(nrDays);
    dailyObsData.inputTMax.resize(nrDays);
    dailyObsData.inputPrecip.resize(nrDays);

    int index = 0;
    for (int i = 0; i < nrData; i++)
    {
        if (i >= startIndex && i < (startIndex+int(nrDays)))
        {
            dailyObsData.inputTMin[index] = climateDailyObsData.inputTMin[i];
            dailyObsData.inputTMax[index] = climateDailyObsData.inputTMax[i];
            dailyObsData.inputPrecip[index] = climateDailyObsData.inputPrecip[i];
            index++;
        }
    }

    bool result = computeWGClimate(nrDays, dailyObsData.inputFirstDate, dailyObsData.inputTMin,
                                    dailyObsData.inputTMax, dailyObsData.inputPrecip,
                                    precThreshold, minDataPercentage, wGen, isWriteOutput, outputFileName);

    dailyObsData.inputTMin.clear();
    dailyObsData.inputTMax.clear();
    dailyObsData.inputPrecip.clear();

    return result;
}


/*!
  * \brief Compute sample standard deviation
*/
float sampleStdDeviation(float values[], int nElement)
{
    float sum = 0;
    float sum2 = 0;
    int i;

    float stdDeviation = 0;

    if (nElement <= 1)
        stdDeviation = NODATA;
    else
    {
        for (i = 0; i < nElement; i++)
        {
            sum = sum + values[i];
            sum2 = sum2 + (values[i] * values[i]);
        }

        stdDeviation = sqrt( std::max(nElement * sum2 - (sum * sum), 0.f) / float(nElement * (nElement - 1)) );
    }
    return stdDeviation;
}

float computeNormalizedDryDayLength(std::vector<float>& probabiltyConsecutiveDays)
{
    int iterations = 31;
    float length = 1;
    float averageProbability = statistics::mean(probabiltyConsecutiveDays);
    float factor = 1;
    for (int i=1; i < iterations; i++)
    {
        for (int j=0; j < i; j++)
        {
            if (j < probabiltyConsecutiveDays.size())
                factor *= probabiltyConsecutiveDays[j];
            else
                factor *= averageProbability;
        }
        length += factor;
    }
    return length;
}


void averagedVectorProbability(std::vector<float>& probabiltyConsecutiveDays)
{
    float averageProbability = statistics::mean(probabiltyConsecutiveDays);
    for (int j=0; j < probabiltyConsecutiveDays.size(); j++)
    {
        if (probabiltyConsecutiveDays[j] < EPSILON)
        {
            probabiltyConsecutiveDays[j] = averageProbability;
            averageProbability = statistics::mean(probabiltyConsecutiveDays);
            probabiltyConsecutiveDays[j] = averageProbability;
            averageProbability = statistics::mean(probabiltyConsecutiveDays);
            probabiltyConsecutiveDays[j] = averageProbability;
            averageProbability = statistics::mean(probabiltyConsecutiveDays);
            probabiltyConsecutiveDays[j] = averageProbability;
        }
    }
    return;
}


float avgProbabilityVectorTail(std::vector<float>& probabilityConsecutiveDays)
{
    if (probabilityConsecutiveDays.size() <= NRDAYS_MAXDRYINCREASE)
        return NODATA;

    double sum = 0;
    double sumWeights = 0;
    double p = probabilityConsecutiveDays[NRDAYS_MAXDRYINCREASE];
    double weight = 1;
    for (int i = NRDAYS_MAXDRYINCREASE; i < (probabilityConsecutiveDays.size()-1); i++)
    {
        weight *= p;
        sum += weight * probabilityConsecutiveDays[i];
        sumWeights += weight;
    }

    if (sumWeights == 0)
        return NODATA;

    return float(sum / sumWeights);
}

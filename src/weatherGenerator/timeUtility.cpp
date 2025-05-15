#include <QDebug>
#include <QDate>
#include <QString>

#include "crit3dDate.h"
#include "timeUtility.h"
#include "commonConstants.h"
#include "basicMath.h"
#include "weatherGenerator.h"


int getMonthsInPeriod(int month1, int month2)
{
    int monthsInPeriod = 0;

    if (month2 >= month1)
    {
        // regular period
        monthsInPeriod = (month2 - month1 + 1);
    }
    else
    {
        // not regular period (between years)
        monthsInPeriod = (12 - month1 + 1) + month2;
    }

    return monthsInPeriod;
}


// it checks if observed data includes the last 9 months before wgDoy_1
// it updates wgDoy_1 and nr_DaysBefore if some days are missing (lower than NRDAYSTOLERANCE)
bool checkLastYearDate(TinputObsData* dailyObsData, int predictionYear, int &wgDoy1, int &nrDaysBeforeWGDay1)
{
    Crit3DDate predictionFirstDate = getDateFromDoy(predictionYear, wgDoy1);

    // check NODATA at the end of observed period
    bool isCheck = true;
    while (isCheck && dailyObsData->inputLastDate > dailyObsData->inputFirstDate)
    {
        int obsIndex = difference(dailyObsData->inputFirstDate, dailyObsData->inputLastDate);
        if ( isEqual(dailyObsData->inputTMin[obsIndex], NODATA)
            || isEqual(dailyObsData->inputTMax[obsIndex], NODATA)
            || isEqual(dailyObsData->inputPrecip[obsIndex], NODATA) )
        {
            qDebug() << "WARNING! Missing data:" << QString::fromStdString(dailyObsData->inputLastDate.toISOString());
            dailyObsData->inputLastDate = dailyObsData->inputLastDate.addDays(-1);
            dailyObsData->dataLength--;
        }
        else
            isCheck = false;
    }

    if (dailyObsData->inputLastDate.addDays(NRDAYSTOLERANCE+1) <  predictionFirstDate)
    {
        qDebug() << "\nObserved days missing are more than NRDAYSTOLERANCE" << NRDAYSTOLERANCE << "\n";
        return false;
    }

    int predictionMonth = predictionFirstDate.month;
    int monthIndex = 0;
    nrDaysBeforeWGDay1 = 0;

    for (int i = 0; i < 9; i++)
    {
        monthIndex = (predictionMonth -1) -i;
        if (monthIndex <= 0)
        {
            monthIndex = monthIndex + 12 ;
            predictionYear = predictionYear - 1;
        }
        nrDaysBeforeWGDay1 += getDaysInMonth(monthIndex, predictionYear);
    }

    // shift wgDoy1 if there are missing data
    if (dailyObsData->inputLastDate < predictionFirstDate)
    {
        int delta = difference(dailyObsData->inputLastDate, predictionFirstDate) - 1;
        wgDoy1 -= delta;
        nrDaysBeforeWGDay1 -= delta;
    }

    // use or not the observed data in the forecast period
    else if (USEDATA)
    {
        if (dailyObsData->inputLastDate > predictionFirstDate.addDays(80))
        {
            qDebug() << "Check your XML: you have already all observed data" << "\n";
            return false;
        }
        if (isLeapYear(predictionFirstDate.year))
        {
            wgDoy1 = (wgDoy1 + (difference(predictionFirstDate, dailyObsData->inputLastDate)) + 1 ) % 366;
        }
        else
        {
            wgDoy1 = (wgDoy1 + (difference(predictionFirstDate, dailyObsData->inputLastDate)) + 1 ) % 365;
        }

        nrDaysBeforeWGDay1 += (difference(predictionFirstDate, dailyObsData->inputLastDate)) + 1 ;
    }

    if ( difference(dailyObsData->inputFirstDate, predictionFirstDate) < nrDaysBeforeWGDay1 || dailyObsData->dataLength < (nrDaysBeforeWGDay1-NRDAYSTOLERANCE) )
    {
        // observed data does not include 9 months before wgDoy1 or more than NRDAYSTOLERANCE days missing
        return false;
    }

    return true;
}


bool getDoyFromSeason(const QString &season, int predictionYear, int &wgDoy1, int &wgDoy2)
{
    QString period[12] = {"JFM","FMA","MAM","AMJ","MJJ","JJA","JAS","ASO","SON","OND","NDJ","DJF"};
    int i = 0;
    bool isFound = false;

    for (i = 0; i < 12; i++)
    {
        if (season.compare(period[i]) == 0)
        {
            isFound = true;
            break;
        }
    }
    if (! isFound)
    {
        qDebug() << "Wrong season" ;
        return false;
    }

    int firstMonth = i + 1;             // first month of my season
    int lastMonth = (i + 3) % 12;       // last month of my season
    if (lastMonth == 0)
        lastMonth = 12;

    Crit3DDate predictionFirstDate = Crit3DDate(1, firstMonth, predictionYear);
    wgDoy1 = getDoyFromDate(predictionFirstDate);

    // season between 2 years
    if (season.compare(period[10]) == 0 || season.compare(period[11]) == 0)
    {
        predictionYear = predictionYear + 1 ;
    }

    int lastDayMonth = getDaysInMonth(lastMonth, predictionYear);
    Crit3DDate predictionLastDate = Crit3DDate(lastDayMonth, lastMonth, predictionYear);
    wgDoy2 = getDoyFromDate(predictionLastDate);

    return true;
}


void setCorrectWgDoy(int wgDoy1, int wgDoy2, int predictionYear, int myYear, int &fixedWgDoy1, int &fixedWgDoy2)
{
    // check if wgDoy1 and wgDoy2 have been computed starting from a leap year and adjust them for standard years
    if (wgDoy1 < wgDoy2)
    {
        if (isLeapYear(predictionYear) && !isLeapYear(myYear))
        {
            // if wgDoy1 or wgDoy2 > 29th Feb.
            if (wgDoy1 >= 60)
                fixedWgDoy1 = wgDoy1-1;

            if (wgDoy1 >= 60)
                fixedWgDoy2 = wgDoy2-1;
        }
        else if ( !isLeapYear(predictionYear) && isLeapYear(myYear))
        {
            // if wgDoy1 or wgDoy2 > 29th Feb.
            if (wgDoy1 >= 60)
                fixedWgDoy1 = wgDoy1+1;

            if (wgDoy1 >= 60)
                fixedWgDoy2 = wgDoy2+1;
        }
        else
        {
            fixedWgDoy1 = wgDoy1;
            fixedWgDoy2 = wgDoy2;
        }
    }
    else
    {
        if (isLeapYear(predictionYear) && !isLeapYear(myYear))
        {
            // if wgDoy1 > 29th Feb.
            if (wgDoy1 >= 60)
                fixedWgDoy1 = wgDoy1-1;
        }

        else if (isLeapYear(predictionYear+1) && !isLeapYear(myYear))
        {
            // if wgDoy2 > 29th Feb.
            if (wgDoy1 >= 60)
                fixedWgDoy2 = wgDoy2-1;
        }
        else
        {
            fixedWgDoy1 = wgDoy1;
            fixedWgDoy2 = wgDoy2;
        }
    }
}

void setAnomalyMonthScenario(QString startingSeason, int *anomalyMonth1, int *anomalyMonth2)
{
    if(startingSeason == "DJF")
    {
        anomalyMonth1[0] = 12; anomalyMonth2[0] = 2;
        anomalyMonth1[1] = 3; anomalyMonth2[1] = 5;
        anomalyMonth1[2] = 6; anomalyMonth2[2] = 8;
        anomalyMonth1[3] = 9; anomalyMonth2[3] = 11;
    }
    else if (startingSeason == "MAM")
    {
        anomalyMonth1[3] = 12; anomalyMonth2[3] = 2;
        anomalyMonth1[0] = 3; anomalyMonth2[0] = 5;
        anomalyMonth1[1] = 6; anomalyMonth2[1] = 8;
        anomalyMonth1[2] = 9; anomalyMonth2[2] = 11;
    }
    else if (startingSeason == "JJA")
    {
        anomalyMonth1[2] = 12; anomalyMonth2[2] = 2;
        anomalyMonth1[3] = 3; anomalyMonth2[3] = 5;
        anomalyMonth1[0] = 6; anomalyMonth2[0] = 8;
        anomalyMonth1[1] = 9; anomalyMonth2[1] = 11;

    }
    else
    {
        anomalyMonth1[1] = 12; anomalyMonth2[1] = 2;
        anomalyMonth1[2] = 3; anomalyMonth2[2] = 5;
        anomalyMonth1[3] = 6; anomalyMonth2[3] = 8;
        anomalyMonth1[0] = 9; anomalyMonth2[0] = 11;
    }
}

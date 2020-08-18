#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "readPragaFormatData.h"
#include "readErg5FilesC4C7.h"
#include "commonConstants.h"
#include "crit3dDate.h"

int readERG5CellListNumber(FILE *fp)
{
    int counter = 0;
    char dummy;

    do {
        dummy = getc(fp);
        if (dummy == '\n') counter++ ;
    } while (dummy != EOF);
    return counter ;
}

/*
int readERG5LineFileNumber(FILE *fp)
{
    int counter = -2;
    char dummy;

    do {
        dummy = getc(fp);
        if (dummy == '\n') counter++ ;
    } while (dummy != EOF);
    return counter ;
}
*/
void readTheCellNumber(FILE *fp, char* numCell)
{
    for (int i=0;i<6;i++)
    {
        numCell[i] = '\0';
    }
    numCell[0] = getc(fp);
    numCell[1] = getc(fp);
    numCell[2] = getc(fp);
    numCell[3] = getc(fp);
    numCell[4] = getc(fp);
    getc(fp);
}

void readEarliestLatestDateC4C7(FILE *fp,int* firstDate, int* lastDate)
{
    char dummy[10];
    int counter;
    // skip the first line
    do {
        dummy[0] = getc(fp);
    } while (dummy[0] != '\n');
    // read the first date
    for (int i=0; i<10; i++)
    {
        dummy[i] = '\0';
    }
    counter = 0;
    do {
        dummy[counter] = getc(fp);
    } while (dummy[counter++] != '.');
    while(getc(fp) != '\n')
    {
        getc(fp);
    }
    float a;
    a = atof(dummy);
    *firstDate = int (a);
    *lastDate = *firstDate;
    // read the last date
    char dummyOneDigit;
    do {

        for (int i=0; i<10; i++)
        {
            dummy[i] = '\0';
        }
        counter = 0;
        do {
            dummyOneDigit = dummy[counter] = getc(fp);
        } while (dummy[counter++] != '.' && dummyOneDigit != EOF);
        dummyOneDigit = getc(fp);
        while(dummyOneDigit != '\n' && dummyOneDigit != EOF)
        {
            dummyOneDigit = getc(fp);
        }
        if (dummyOneDigit != EOF)
        {
            a = atof(dummy);
            *lastDate = int(a);
        }
        else return;
    } while (dummyOneDigit != EOF);
}

bool readPragaERG5DailyDataC4C7(FILE *fp,bool* firstRead,int* currentDate,int *doy,int *day,int *month, int* year,double* tmin,double* tmax,double *prec)
{
    char dummy = '\0';
    int counter=0;
    char dayNumberChar[10],minTChar[10],maxTChar[10],precChar[10];
    int dayNumber;
    if (*firstRead)
    {
        // skip the one line header
        do {
                dummy = getc(fp);
        } while (dummy != '\n');
        *firstRead = false;
    }
    for (int i=0; i<10; i++)
    {
        dayNumberChar[i] = '\0';
        minTChar[i] = '\0';
        maxTChar[i] = '\0';
        precChar[i] = '\0';
    }

    counter = 0;
    while ((dummy = getc(fp)) != ',')
    {
        dayNumberChar[counter++] = dummy;
    }
    for (int i=0;i<4;i++)
    {
        while ((dummy = getc(fp)) != ',')
        {
            continue;
        }
    }
    counter = 0;
    while ((dummy = getc(fp)) != ',')
    {
        minTChar[counter++] = dummy;
    }
    while ((dummy = getc(fp)) != ',')
    {
        continue;
    }
    counter = 0;
    while ((dummy = getc(fp)) != ',')
    {
        maxTChar[counter++] = dummy;
    }
    for (int i=0;i<3;i++)
    {
        while ((dummy = getc(fp)) != ',')
        {
            continue;
        }
    }
    counter = 0;
    while ((dummy = getc(fp)) != ',')
    {
        precChar[counter++] = dummy;
    }
    *currentDate = dayNumber = int (atof(dayNumberChar));
    getTheNewDateShiftingDays(dayNumber,30,12,1899,day,month,year);
    //*doy = getDoyFromDate(day,month,year);
    //if (dayNumber < )
    *prec = atof(precChar);
    *tmin = atof(minTChar);
    *tmax = atof(maxTChar);

    do{
        dummy = getc(fp);
        if (dummy == EOF) return true;
    }while (dummy != '\n' && dummy != EOF);


    return true;
}


void getTheNewDateShiftingDays(int dayOffset, int day0, int month0, int year0, int* dayFinal, int* monthFinal, int* yearFinal)
{

    if (dayOffset >= 0)
    {
        // shift back the initial date to the first of January
        --dayOffset += getDoyFromDate(day0,month0,year0);
        *yearFinal = year0;
        if (dayOffset < 365 + isLeapYear(*yearFinal))
        {
            getDateFromDoy(++dayOffset,*yearFinal,monthFinal,dayFinal);
            return;
        }
        while(dayOffset >= 365 + isLeapYear(*yearFinal))
        {
            dayOffset -= 365 + isLeapYear(*yearFinal);
            (*yearFinal)++;
        }
        getDateFromDoy(++dayOffset,*yearFinal,monthFinal,dayFinal);

    }
    else
    {
        // shift ahead to the thirstyfirst of December of the same year
        dayOffset -= (365 + isLeapYear(year0) - getDoyFromDate(day0,month0,year0));
        *yearFinal = year0;
        while (fabs(dayOffset) >= 365 + isLeapYear(*yearFinal))
        {
            dayOffset += (365 + isLeapYear(*yearFinal));
            (*yearFinal)--;
        }
        getDateFromDoy(365 + isLeapYear(*yearFinal)+ dayOffset,*yearFinal,monthFinal,dayFinal);
    }
}

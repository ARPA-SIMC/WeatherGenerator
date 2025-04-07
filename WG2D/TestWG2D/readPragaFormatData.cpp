#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "readPragaFormatData.h"
#include "commonConstants.h"

int readPragaLineFileNumber(FILE *fp)
{
    int counter = -1;
    char dummy;

    do {
        dummy = getc(fp);
        if (dummy == '\n') counter++ ;
    } while (dummy != EOF);
    return counter ;
}

bool readPragaERG5DailyData(FILE *fp,bool* firstDay,int *doy,int *day,int *month, int* year,double* tmin,double* tmax,double* tmean, double* rhmax, double* rhmin, double* rhmean, double * rad,double *prec)
{
    char dummy ;
    int counter;
    char daychar[3],monthchar[3],yearchar[5],tminchar[6],tmaxchar[6],tmeanchar[6],precchar[6],
         rhminchar[4],rhmaxchar[4],rhmeanchar[4],radchar[5];

    if (*firstDay)
    {
        do {
            dummy = getc(fp);
        } while (dummy != '\n');
        if (dummy == '\n') *firstDay = false;
    }
    else
    {
        for (short i=0; i<2; i++)daychar[i] = getc(fp);
        getc(fp);
        for (short i=0; i<2; i++) monthchar[i] = getc(fp);
        getc(fp);
        for (short i=0; i<4; i++) yearchar[i] = getc(fp);
        getc(fp);
        daychar[2] = '\0';
        monthchar[2] = '\0';
        yearchar[4] = '\0';

        for (short i=0; i<4; i++)
        {
            rhminchar[i] = '\0';
            rhmeanchar[i] = '\0';
            rhmaxchar[i] = '\0';
        }


        for (short i=0; i<5; i++)
        {
            radchar[i] = '\0';
        }

        for (short i=0; i<6; i++)
        {
            tminchar[i] = '\0';
            tmeanchar[i] = '\0';
            tmaxchar[i] = '\0';
            precchar[i] = '\0';
        }
        counter = 0;
        do
        {
            dummy = getc(fp);
            tminchar[counter++] = dummy;
        } while (dummy != '\t');
        counter = 0;
        do
        {
            dummy = getc(fp);
            tmaxchar[counter++] = dummy;
        } while (dummy != '\t');
        counter = 0;
        do
        {
            dummy = getc(fp);
            tmeanchar[counter++] = dummy;
        } while (dummy != '\t');
        counter = 0;
        do
        {
            dummy = getc(fp);
            precchar[counter++] = dummy;
        } while (dummy != '\t');
        counter = 0;
        do
        {
            dummy = getc(fp);
            rhminchar[counter++] = dummy;
        } while (dummy != '\t');
        counter = 0;
        do
        {
            dummy = getc(fp);
            rhmeanchar[counter++] = dummy;
        } while (dummy != '\t');
        counter = 0;
        do
        {
            dummy = getc(fp);
            rhmaxchar[counter++] = dummy;
        } while (dummy != '\t');
        //counter = 0;
        do
        {
            dummy = getc(fp);
            //rhmaxchar[counter++] = dummy;
        } while (dummy != '\t');
        //counter = 0;
        do
        {
            dummy = getc(fp);
            //rhmaxchar[counter++] = dummy;
        } while (dummy != '\t');
        counter = 0;
        do
        {
            dummy = getc(fp);
            radchar[counter++] = dummy;
        } while (dummy != '\n');
        counter = 0;



        *day = atoi(daychar);
        *month = atoi(monthchar);
        *year = atoi (yearchar);
        *tmin = atof(tminchar);
        *tmax = atof(tmaxchar);
        *tmean = atof(tmeanchar);
        *prec = atof(precchar);
        *rhmin = (double)(atoi(rhminchar));
        *rhmax = (double)(atoi(rhmaxchar));
        *rhmean = (double)(atoi(rhmeanchar));
        *rad = atof(radchar);
        *doy = getDoyFromDate(*day,*month,*year);
    }
        return true;
}

bool readPragaERACLITODailyData(FILE *fp,bool* firstDay,int *doy,int *day,int *month, int* year,double* tmin,double* tmax,double* tmean,double *prec)
{
    char dummy = '\0';
    int counter;
    char daychar[3],monthchar[3],yearchar[5],tminchar[20],tmaxchar[20],tmeanchar[20],precchar[20];

    if (*firstDay)
    {
        for(int i=0;i<1;i++)
        {
            do {
                dummy = getc(fp);
            } while (dummy != '\n');
        }
        if (dummy == '\n') *firstDay = false;
    }
    else
    {
        //getc(fp);
        for (short i=0; i<2; i++)daychar[i] = getc(fp);
        getc(fp);
        for (short i=0; i<2; i++) monthchar[i] = getc(fp);
        getc(fp);
        for (short i=0; i<4; i++) yearchar[i] = getc(fp);
        getc(fp);
        daychar[2] = '\0';
        monthchar[2] = '\0';
        yearchar[4] = '\0';


        for (short i=0; i<20; i++)
        {
            tminchar[i] = '\0';
            tmeanchar[i] = '\0';
            tmaxchar[i] = '\0';
            precchar[i] = '\0';
        }
        counter = 0;
        do
        {
            dummy = getc(fp);
            tminchar[counter++] = dummy;
        } while (dummy != '\t');
        counter = 0;
        do
        {
            dummy = getc(fp);
            tmaxchar[counter++] = dummy;
        } while (dummy != '\t');
        counter = 0;
        do
        {
            dummy = getc(fp);
            tmeanchar[counter++] = dummy;
        } while (dummy != '\t');
        counter = 0;
        do
        {
            dummy = getc(fp);
            precchar[counter++] = dummy;
        } while (dummy != '\n');
        counter = 0;


        *day = atoi(daychar);
        *month = atoi(monthchar);
        *year = atoi (yearchar);
        *tmin = atof(tminchar);
        *tmax = atof(tmaxchar);
        *tmean = atof(tmeanchar);
        *prec = atof(precchar);
        *doy = getDoyFromDate(*day,*month,*year);
    }
        return true;
}


int getDoyFromDate(int day, int month, int year)
{
        char leap = 0 ;
        int doy;
        int monthList[12];


        if (year%4 == 0)
        {
            leap = 1;
            if (year%100 == 0 && year%400 != 0) leap = 0 ;
        }
        monthList[0]=31 ;
        monthList[1]=28 + leap ;
        monthList[2]=31 ;
        monthList[3]=30 ;
        monthList[4]=31 ;
        monthList[5]=30 ;
        monthList[6]=31 ;
        monthList[7]=31 ;
        monthList[8]=30 ;
        monthList[9]=31 ;
        monthList[10]=30;
        monthList[11]=31 ;
        if (month < 1 || month > 12) return(PARAMETER_ERROR);
        if (day < 1 || day > monthList[month-1]) return(PARAMETER_ERROR);
        doy = 0 ;
        for (short i = 0 ; i < month-1 ; i++){
            doy += monthList[i] ;
        }
        doy += day ;
        return doy ;
}


bool getDateFromDoy(int doy,int year,int* month, int* day)
{
    //if (doy < 1) return false;
    //if (doy > 366) return false;
    char leap = 0 ;
    int monthList[12];


    if (year%4 == 0)
    {
        leap = 1;
        if (year%100 == 0 && year%400 != 0) leap = 0 ;
    }
    monthList[0]= 31;
    monthList[1]= monthList[0] + 28 + leap;
    monthList[2]= monthList[1] + 31 ;
    monthList[3]= monthList[2] + 30 ;
    monthList[4]= monthList[3] + 31 ;
    monthList[5]= monthList[4] + 30 ;
    monthList[6]= monthList[5] + 31 ;
    monthList[7]= monthList[6] + 31 ;
    monthList[8]= monthList[7] + 30 ;
    monthList[9]= monthList[8] + 31 ;
    monthList[10]= monthList[9] + 30;
    monthList[11]= monthList[10] + 31;
    if (doy > monthList[11])
    {
        return false;
    }
    else if (doy > monthList[10])
    {
        *month = 12;
        *day = doy - monthList[10];
    }
    else if (doy > monthList[9])
    {
        *month = 11;
        *day = doy - monthList[9];
    }
    else if (doy > monthList[8])
    {
        *month = 10;
        *day = doy - monthList[8];
    }
    else if (doy > monthList[7])
    {
        *month = 9;
        *day = doy - monthList[7];
    }
    else if (doy > monthList[6])
    {
        *month = 8;
        *day = doy - monthList[6];
    }
    else if (doy > monthList[5])
    {
        *month = 7;
        *day = doy - monthList[5];
    }
    else if (doy > monthList[4])
    {
        *month = 6;
        *day = doy - monthList[4];
    }
    else if (doy > monthList[3])
    {
        *month = 5;
        *day = doy - monthList[3];
    }
    else if (doy > monthList[2])
    {
        *month = 4;
        *day = doy - monthList[2];
    }
    else if (doy > monthList[1])
    {
        *month = 3;
        *day = doy - monthList[1];
    }
    else if (doy > monthList[0])
    {
        *month = 2;
        *day = doy - monthList[0];
    }
    else
    {
        *month = 1;
        *day = doy;
    }
    return true;
}


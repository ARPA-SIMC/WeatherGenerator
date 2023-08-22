#ifndef READPRAGAFORMATDATA_H
#define READPRAGAFORMATDATA_H

    #include <stdio.h>

    int readPragaLineFileNumber(FILE *fp);
    bool readPragaERG5DailyData(FILE *fp,bool* firstDay,int*doy,int *day,int *month, int* year,double* tmin,double* tmax,double* tmean, double* rhmax, double* rhmin, double* rhmean, double * rad,double *prec);
    bool readPragaERACLITODailyData(FILE *fp,bool* firstDay,int*doy,int *day,int *month, int* year,double* tmin,double* tmax,double* tmean,double *prec);

    int getDoyFromDate(int day, int month, int year);
    bool getDateFromDoy(int doy,int year,int* month, int* day);

#endif // READPRAGAFORMATDATA_H

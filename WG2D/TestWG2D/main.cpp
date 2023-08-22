// ****************************************************************************************************************
// This is a smart interface to use the 2D weather generator
// the input file must be within the subfolder inputData and called 0000.txt (first station), 0001.txt and so forth
// check the right format for input in the proposed default files

// please set the following variables before running the weather generator
#define NR_STATIONS 10 // number of stations/cells. Do not exceed 10000 stations, corresponding to 9999.txt input file
#define NR_SIMULATION_YEARS 58 // integer number please do not exceed 1000 years.Be careful with memory issue by default 10 stations are inserted.
#define STARTING_YEAR  1 // modify if necessary
#define PREC_THRESHOLD 0.25  // choose the threshold in mm
#define TEMPERATURE_AVERAGE_METHOD 0 // 0 for ROLLING AVERAGE (default ) otherwise 1 FOURIER HARMONICS AVERAGE (the first 3 harmonics are used)
#define COMPUTE_MONTHLY_STATISTICS true  // aut true aut false

//************************************************************************************************************

#include <QString>
#include <QFile>
#include <QDateTime>
#include <QTextStream>
#include <QCoreApplication>
#include <iostream>
#include <malloc.h>

#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <chrono>

#include "commonConstants.h"
#include "furtherMathFunctions.h"
#include "wg2D.h"
#include "readPragaFormatData.h"
#include "crit3dDate.h"
#include "statistics.h"

#include "utilities.h"
#include "dbMeteoGrid.h"
#include "meteo.h"
#include "meteoPoint.h"


static Crit3DMeteoGridDbHandler* meteoGridDbHandlerWG2D;
static weatherGenerator2D WG2D;

void printSimulationResults(ToutputWeatherData* output,int nrStations,int lengthArray);

void obsDataMeteoPointFormat(int nrStations, int nrData, float*** weatherArray, int** dateArray,TObsDataD** observedDataDaily)
{
    for(int i=0;i<nrStations;i++)
    {
        for (int j=0;j<nrData;j++)
        {
            observedDataDaily[i][j].date.day = dateArray[j][0];
            observedDataDaily[i][j].date.month = dateArray[j][1];
            observedDataDaily[i][j].date.year = dateArray[j][2];
            observedDataDaily[i][j].tMin = weatherArray[i][j][0];
            observedDataDaily[i][j].tMax = weatherArray[i][j][1];
            observedDataDaily[i][j].prec = weatherArray[i][j][2];
        }

    }
}

void logInfo(QString myStr)
{
     std::cout << myStr.toStdString() << std::endl;
}


int main()
{
    FILE* fp;
    int numberMeteoLines;
    double precipitationThreshold = PREC_THRESHOLD;
    int nrStations = NR_STATIONS;
    int distributionType = 1; // 1 multiexponential 2 multigamma
    int yearsOfSimulations = NR_SIMULATION_YEARS;
    int lengthDataSeries;
    float *** weatherArray = nullptr;
    int ** dateArray = nullptr;
    fp = fopen("inputData/0000.txt", "r");
    if (fp == nullptr)
    {
        printf("Error! File not found\n");
        return -1;
    }

    numberMeteoLines = readPragaLineFileNumber(fp);
    fclose(fp);
    int doy,day,month,year;
    double prec,minT,maxT,meanT;
    doy = day = month = year = NODATA;
    prec = minT = maxT = meanT = NODATA;
    bool firstDay = true;
    lengthDataSeries = numberMeteoLines;
    int nrVariables = 3;
    int nrDate = 3;

    weatherArray = (float ***)calloc(nrStations, sizeof(float**));
    for (int i=0;i<nrStations;i++)
    {
        weatherArray[i] = (float **)calloc(lengthDataSeries, sizeof(float*));
        for (int j=0;j<lengthDataSeries;j++)
        {
            weatherArray[i][j] = (float *)calloc(nrVariables, sizeof(float));
        }
    }
    dateArray = (int **)calloc(lengthDataSeries, sizeof(int*));
    for (int j=0;j<lengthDataSeries;j++)
    {
        dateArray[j] = (int *)calloc(nrDate, sizeof(int));
    }
    srand(time(nullptr));
    static char pathAndFileName[18];
    for (int j=0;j<18;j++)
    {
        pathAndFileName[j] = '\0';
    }
    pathAndFileName[0] = 'i';
    pathAndFileName[1] = 'n';
    pathAndFileName[2] = 'p';
    pathAndFileName[3] = 'u';
    pathAndFileName[4] = 't';
    pathAndFileName[5] = 'D';
    pathAndFileName[6] = 'a';
    pathAndFileName[7] = 't';
    pathAndFileName[8] = 'a';
    pathAndFileName[9] = '/';
    pathAndFileName[14] = '.';
    pathAndFileName[15] = 't';
    pathAndFileName[16] = 'x';
    pathAndFileName[17] = 't';
    for (int i=0;i<nrStations;i++)
    {
       char buffer[4];
       buffer[0] = buffer[1] = buffer[2] = buffer[3] = '0';
       itoa(i,buffer,10);

       if (i > 999)
       {
           pathAndFileName[10] = buffer[0];
           pathAndFileName[11] = buffer[1];
           pathAndFileName[12] = buffer[2];
           pathAndFileName[13] = buffer[3];
       }
       else if (i > 99)
       {
           pathAndFileName[10] = '0';
           pathAndFileName[11] = buffer[0];
           pathAndFileName[12] = buffer[1];
           pathAndFileName[13] = buffer[2];
       }
       else if (i > 9)
       {
           pathAndFileName[10] = '0';
           pathAndFileName[11] = '0';
           pathAndFileName[12] = buffer[0];
           pathAndFileName[13] = buffer[1];
       }
       else if (i < 10)
       {
           pathAndFileName[10] = '0';
           pathAndFileName[11] = '0';
           pathAndFileName[12] = '0';
           pathAndFileName[13] = buffer[0];
       }
       else
       {
           printf("wrong value of nrStation\n");
           return 0;
       }
       printf("%s \n",pathAndFileName);
       fp = fopen(pathAndFileName,"r");
       if (fp == nullptr)
       {
           printf("Error! File not found\n");
           return -1;
       }

       firstDay = true;
       readPragaERACLITODailyData(fp,&firstDay,&doy,&day,&month,&year,&minT,&maxT,&meanT,&prec);
       for (int j=0;j<numberMeteoLines;j++)
       {
           readPragaERACLITODailyData(fp,&firstDay,&doy,&day,&month,&year,&minT,&maxT,&meanT,&prec);
           weatherArray[i][j][0] = minT;
           weatherArray[i][j][1] = maxT;
           weatherArray[i][j][2] = prec;
           dateArray[j][0] = day;
           dateArray[j][1] = month;
           dateArray[j][2] = year;
       }
       fclose(fp);
    }
    TObsDataD** observedDataDaily = (TObsDataD **)calloc(nrStations, sizeof(TObsDataD*));
    for (int i=0;i<nrStations;i++)
    {
        observedDataDaily[i] = (TObsDataD *)calloc(lengthDataSeries, sizeof(TObsDataD));
    }
    obsDataMeteoPointFormat(nrStations,lengthDataSeries,weatherArray,dateArray,observedDataDaily);
    WG2D.initializeData(lengthDataSeries,nrStations);
    WG2D.setObservedData(observedDataDaily);


    bool computePrecipitation = true;
    bool computeTemperature = true;
    printf("weather generator\n");
    int startingYear = STARTING_YEAR;
    int lengthArraySimulation;
    lengthArraySimulation = 365 * yearsOfSimulations;
    ToutputWeatherData* results;
    results = (ToutputWeatherData *)calloc(nrStations, sizeof(ToutputWeatherData));
    for (int iStation=0;iStation<nrStations;iStation++)
    {
        results[iStation].daySimulated = (int *)calloc(lengthArraySimulation, sizeof(int));
        results[iStation].monthSimulated = (int *)calloc(lengthArraySimulation, sizeof(int));
        results[iStation].yearSimulated = (int *)calloc(lengthArraySimulation, sizeof(int));
        results[iStation].doySimulated = (int *)calloc(lengthArraySimulation, sizeof(int));
        results[iStation].minT = (double *)calloc(lengthArraySimulation, sizeof(double));
        results[iStation].maxT = (double *)calloc(lengthArraySimulation, sizeof(double));
        results[iStation].precipitation = (double *)calloc(lengthArraySimulation, sizeof(double));
    }
    WG2D.initializeParameters(precipitationThreshold, yearsOfSimulations, distributionType,
                              computePrecipitation, computeTemperature,COMPUTE_MONTHLY_STATISTICS,TaverageTempMethod(TEMPERATURE_AVERAGE_METHOD));
    WG2D.computeWeatherGenerator2D();
    results = WG2D.getWeatherGeneratorOutput(startingYear);
    printSimulationResults(results,nrStations,lengthArraySimulation);

    //free memory
    for (int i=0;i<nrStations;i++)
    {
        free(observedDataDaily[i]);
        for (int j=0;j<lengthDataSeries;j++)
        {
            free(weatherArray[i][j]);
        }
        free(weatherArray[i]);
    }
    for (int j=0;j<lengthDataSeries;j++)
    {
        free(dateArray[j]);
    }
    free(observedDataDaily);
    free(weatherArray);
    free(dateArray);

    return 0;
}

void printSimulationResults(ToutputWeatherData* output,int nrStations,int lengthArray)
{
    QString outputName;
    for (int iStation=0; iStation<nrStations;iStation++)
    {
        outputName = "outputData/wgStation_" + QString::number(iStation) + ".csv";
        QFile file(outputName);
        file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
        QTextStream stream( &file );
        for (int m=0; m < lengthArray; m++)
        {
            stream <<  output[iStation].daySimulated[m] << "/" << output[iStation].monthSimulated[m] << "/" << output[iStation].yearSimulated[m] << "," << output[iStation].doySimulated[m] << "," << output[iStation].minT[m]<< "," << output[iStation].maxT[m]<< "," << output[iStation].precipitation[m] << "\n";
        }
        file.close();
    }
}


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
#include "readErg5FilesC4C7.h"
#include "crit3dDate.h"
#include "statistics.h"

#include "utilities.h"
#include "dbMeteoGrid.h"
#include "meteo.h"
#include "meteoPoint.h"


#define NR_SIMULATION_YEARS 10
// [ 1 - 10 ]
#define NR_STATIONS 5
#define DEFAULT_TEST 0
#define RECLAMATION_CONSORTIA_TEST 1
#define KIND_TEST RECLAMATION_CONSORTIA_TEST
// added for simulating C4 and C7
#define STARTING_YEAR  2501
#define PREC_THRESHOLD 0.25
#define NRCONSORTIUM 4
#define CONNECTDATABASE false
#define ROLLING_AVERAGE 0
#define FOURIER_HARMONICS_AVERAGE 1

static Crit3DMeteoGridDbHandler* meteoGridDbHandlerWG2D;
static weatherGenerator2D WG2D;

void printSimulationResults(ToutputWeatherData* output,int nrStations,int lengthArray,int nrConsortium);

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

bool saveOnMeteoGridDB(QString* errorString, int consortium)
{
    //QString xmlName = QFileDialog::getOpenFileName(nullptr, "Open XML grid", "", "XML files (*.xml)");
    QString path;
    QString xmlName;
    if (! searchDataPath(&path)) return -1;
    if (consortium == 7)
        xmlName = path + "METEOGRID/DBGridXML_WG2D.xml";
    else if (consortium == 4)
        xmlName = path + "METEOGRID/DBGridXML_C4_WG2D.xml";
    else
        xmlName = path + "METEOGRID/DBGridXML_C4_WG2D.xml";

    meteoGridDbHandlerWG2D = new Crit3DMeteoGridDbHandler();

    // todo
    //meteoGridDbHandlerWG2D->meteoGrid()->setGisSettings(this->gisSettings);

    if (! meteoGridDbHandlerWG2D->parseXMLGrid(xmlName, errorString)) return false;

    if (! meteoGridDbHandlerWG2D->openDatabase(errorString))return false;

    if (! meteoGridDbHandlerWG2D->loadCellProperties(errorString)) return false;

    //if (! meteoGridDbHandlerWG2D->updateGridDate(errorString)) return false;

    logInfo("Meteo Grid = " + xmlName);

    return true;
}

int main()
{

    FILE* fp;
    /*fp = fopen("randomNumbers2.txt","r");
    char vectorDummy[20];
    double randomSeries[10000];

    int counter = 0;
    char dummy;

    for (int j=0;j<10000;j++)
    {
        counter = 0;
        for (int i=0;i<20;i++)
        {
            vectorDummy[i] = '\0';
        }
        do
        {
            dummy = getc(fp);
            if (dummy != '\n')
                vectorDummy[counter]= dummy;
            counter++;
        } while (dummy != '\n');
        randomSeries[j] = atof(vectorDummy);
    }

    fclose(fp);
    WG2D.initializeRandomNumbers(randomSeries);*/
    int numberMeteoLines;
    double precipitationThreshold = PREC_THRESHOLD;
    int nrC = NRCONSORTIUM;
    int nrStations = NR_STATIONS;
    int distributionType = 1; // 1 multiexponential 2 multigamma 3 Weibull
    int yearsOfSimulations = NR_SIMULATION_YEARS;
    int lengthDataSeries;
    bool connectDatabase = CONNECTDATABASE;
    float *** weatherArray = nullptr;
    int ** dateArray = nullptr;
    int* cellCode = nullptr;
    bool kindOfTest;
    kindOfTest = KIND_TEST;
    if (kindOfTest == DEFAULT_TEST)
    {
        fp = fopen("inputData/argelato_1961_2018.txt", "r");
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
        //float *** weatherArray = nullptr;
        //int ** dateArray = nullptr;

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
        for (int i=0;i<nrStations;i++)
        {
            if (i==0)
            {
                   fp = fopen("inputData/argelato_1961_2018.txt","r");
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
            else if (i==1)
            {
                fp = fopen("inputData/baricella_1961_2018.txt","r");
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
                }
                fclose(fp);
            }
            else if (i==2)
            {
                fp = fopen("inputData/bologna_1961_2018.txt","r");
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
                }
                fclose(fp);
            }
            else if (i==3)
            {
                fp = fopen("inputData/budrio_1961_2018.txt","r");
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
                }
                fclose(fp);
            }
            else if (i==4)
            {
                fp = fopen("inputData/casalfiumanese_1961_2018.txt","r");
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
                }
                fclose(fp);
            }
            else if (i==5)
            {
                fp = fopen("inputData/castenaso_1961_2018.txt","r");
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
                }
                fclose(fp);
            }
            else if (i==6)
            {
                fp = fopen("inputData/mezzolara_1961_2018.txt","r");
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
                }
                fclose(fp);
            }
            else if (i==7)
            {
                fp = fopen("inputData/montecalderaro_1961_2018.txt","r");
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
                }
                fclose(fp);
            }
            else if (i==8)
            {
                fp = fopen("inputData/pievedicento_1961_2018.txt","r");
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
                }
                fclose(fp);
            }
            else if (i==9)
            {
                fp = fopen("inputData/sanlazzaro_1961_2018.txt","r");
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
                }
                fclose(fp);
            }
            else
            {
                for (int j=0;j<numberMeteoLines;j++)
                {
                    weatherArray[i][j][0] = weatherArray[i%10][j][0] + (1.0*rand())/RAND_MAX - 0.5;
                    weatherArray[i][j][1] = weatherArray[i%10][j][1] + (1.0*rand())/RAND_MAX - 0.5;
                    weatherArray[i][j][2] = MAXVALUE(0,weatherArray[i%10][j][2] + (1.0*rand())/RAND_MAX - 0.5);
                }
            }

        }
    }
    else
    {
        // open the list of cells

        if (nrC == 7)
            fp = fopen("inputDataC7/list_C7.txt","r");
        else if (nrC == 4)
            fp = fopen("inputDataC4/list_C4.txt","r");
        else if (nrC == 0)
            fp = fopen("inputDataERG5/listErg5.txt","r");
        else
            return 0;
        int numberOfCells;
        numberOfCells = readERG5CellListNumber(fp);
        fclose(fp);
        // find the cell with the earliest date and with latest date
        int earliestDate, latestDate;
        earliestDate = latestDate = NODATA;
        QString nameOfFile;
        if (nrC == 7)
            fp = fopen("inputDataC7/list_C7.txt","r");
        else if (nrC == 4)
            fp = fopen("inputDataC4/list_C4.txt","r");
        else if (nrC == 0)
            fp = fopen("inputDataERG5/listErg5.txt","r");
        char* numCell = (char *)calloc(6, sizeof(char));
        cellCode = (int *) calloc(numberOfCells, sizeof(int));
        FILE* fp1;
        for (int i=0; i<numberOfCells; i++)
        {
            readTheCellNumber(fp,numCell);
            cellCode[i] = atoi(numCell);
            QString stringNumber(numCell);

            if (nrC == 7)
                nameOfFile = "inputDataC7/" + stringNumber + ".txt";
            else if (nrC == 4)
                nameOfFile = "inputDataC4/" + stringNumber + ".txt";
            else if (nrC == 0)
                nameOfFile = "inputDataERG5/" + stringNumber + ".txt";

            std::string stringNameOfFile;
            stringNameOfFile = nameOfFile.toStdString();
            const char * charNameOfFile = stringNameOfFile.c_str();
            fp1 = fopen(charNameOfFile,"r");
            int firstDate,lastDate;
            firstDate = lastDate = NODATA;
            readEarliestLatestDateC4C7(fp1,&firstDate,&lastDate);
            if (i == 0)
            {
                earliestDate = firstDate;
                latestDate = lastDate;
            }
            else
            {
                if (firstDate < earliestDate)
                    earliestDate = firstDate;
                if (lastDate < latestDate)
                    latestDate = lastDate;
            }

            fclose(fp1);
        }
        free(numCell);
        fclose(fp);
        nrStations = numberOfCells;
        numberMeteoLines = latestDate - earliestDate + 1;

        int doy,day,month,year;
        double prec,minT,maxT,meanT;
        doy = day = month = year = NODATA;
        prec = minT = maxT = meanT = NODATA;
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
        if (nrC == 7)
            fp = fopen("inputDataC7/list_C7.txt","r");
        else if (nrC == 4)
            fp = fopen("inputDataC4/list_C4.txt","r");
        else if (nrC == 0)
            fp = fopen("inputDataERG5/listErg5.txt","r");

        for (int i=0; i<numberOfCells; i++)
        {

            printf("loading data of cell %d\n",i);
            readTheCellNumber(fp,numCell);
            //printf("%c%c%c%c%c\n",numCell[0],numCell[1],numCell[2],numCell[3],numCell[4]);
            QString stringNumber(numCell);
            if (nrC == 7)
                nameOfFile = "inputDataC7/" + stringNumber + ".txt";
            else if (nrC == 4)
                nameOfFile = "inputDataC4/" + stringNumber + ".txt";
            else if (nrC == 0)
                nameOfFile = "inputDataERG5/" + stringNumber + ".txt";
            std::string stringNameOfFile;
            stringNameOfFile = nameOfFile.toStdString();
            const char * charNameOfFile = stringNameOfFile.c_str();
            fp1 = fopen(charNameOfFile,"r");
            bool firstRead = true;
            bool checkFirstRead;
            for (int j=0;j<numberMeteoLines;j++)
            {
                int currentDate;
                checkFirstRead = firstRead;
                readPragaERG5DailyDataC4C7(fp1,&firstRead,&currentDate,&doy,&day,&month,&year,&minT,&maxT,&prec);

                if (checkFirstRead != firstRead)
                {
                    while (currentDate > earliestDate+j)
                    {
                        weatherArray[i][j][0] = NODATA;
                        weatherArray[i][j][1] = NODATA;
                        weatherArray[i][j][2] = NODATA;
                        if (i == 0)
                        {
                            int yearInitial =1899;
                            int monthInitial = 12;
                            int dayInitial = 30;
                            int dayFinal,monthFinal,yearFinal;
                            dayFinal = monthFinal = yearFinal = NODATA;
                            getTheNewDateShiftingDays(earliestDate+j,dayInitial,monthInitial,yearInitial,&dayFinal,&monthFinal,&yearFinal);
                            dateArray[j][0] = dayFinal;
                            dateArray[j][1] = monthFinal;
                            dateArray[j][2] = yearFinal;
                            //printf ("%d %d %d ",dateArray[j][0],dateArray[j][1],dateArray[j][2]);
                            //printf ("%f %f %f \n",weatherArray[i][j][0],weatherArray[i][j][1],weatherArray[i][j][2]);
                            //getchar();
                        }
                        j++;
                    }

                }
                //getchar();
                weatherArray[i][j][0] = minT;
                weatherArray[i][j][1] = maxT;
                weatherArray[i][j][2] = prec;
                if (i == 0)
                {
                    dateArray[j][0] = day;
                    dateArray[j][1] = month;
                    dateArray[j][2] = year;
                }
                //printf ("%d %d %d ",dateArray[j][0],dateArray[j][1],dateArray[j][2]);
                //printf ("%.1f %.1f %.1f \n",weatherArray[i][j][0],weatherArray[i][j][1],weatherArray[i][j][2]);
                //getchar();
            }
            //getchar();
        }
    }

    free(cellCode);

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
    //if (nrC == 4) startingYear += 1000;
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
                              computePrecipitation, computeTemperature,true);
    WG2D.computeWeatherGenerator2D();
    results = WG2D.getWeatherGeneratorOutput(startingYear);
    printSimulationResults(results,nrStations,lengthArraySimulation,nrC);
    if (!connectDatabase) return 0;
    printf("non arriva qui");
    if (kindOfTest == KIND_TEST)
    {
        std::vector<TObsDataD> outputDataD;
        QString errorString;
        QString myError;
        std::string id;
        int nrActivePoints = nrStations;
        int nrYearSimulations = NR_SIMULATION_YEARS;
        int nrLeapYears = 0;
        for (int iYear = startingYear;iYear<startingYear+nrYearSimulations;iYear++ )
        {
            nrLeapYears += isLeapYear(iYear);
        }
        outputDataD.resize(lengthArraySimulation+nrLeapYears);
        // fill the new database
        QDate firstDayOutput(startingYear,1,1);
        QDate lastDayOutput(startingYear+nrYearSimulations-1,12,31);

        if (! saveOnMeteoGridDB(&errorString,nrC))
        {
            std::cout << errorString.toStdString() << std::endl;
            return -1;
        }
        QList<meteoVariable> listMeteoVariable = {dailyAirTemperatureMin,dailyAirTemperatureMax,dailyPrecipitation};
        counter = 0;
        for (int row = 0; row < meteoGridDbHandlerWG2D->gridStructure().header().nrRows; row++)
        {
            for (int col = 0; col < meteoGridDbHandlerWG2D->gridStructure().header().nrCols; col++)
            {

                //meteoGridDbHandlerWG2D->meteoGrid()->getMeteoPointActiveId(row, col, &id);
                bool isConsortiumCell = false;
                int iCells = 0;
                if (meteoGridDbHandlerWG2D->meteoGrid()->getMeteoPointActiveId(row, col, &id))
                {
                    while (iCells < nrActivePoints)
                    {
                        int idCellInt;
                        idCellInt = atoi(id.c_str());
                        if (idCellInt == cellCode[iCells] ) isConsortiumCell = true;
                         iCells++;
                    }
                }
               if (meteoGridDbHandlerWG2D->meteoGrid()->getMeteoPointActiveId(row, col, &id) && counter<nrActivePoints && isConsortiumCell)
               {

                   printf("%d  %d\n",row,col);
                   meteoGridDbHandlerWG2D->meteoGrid()->meteoPointPointer(row,col)->initializeObsDataD(lengthArraySimulation+nrLeapYears, getCrit3DDate(firstDayOutput));
                   int k = NODATA;
                   for (int j=0;j<lengthArraySimulation;j++)
                   {
                       if (j == 0) k = j;
                       outputDataD[k].date.day = results[counter].daySimulated[j];
                       outputDataD[k].date.month = results[counter].monthSimulated[j];
                       outputDataD[k].date.year = results[counter].yearSimulated[j];
                       outputDataD[k].tMin = results[counter].minT[j];
                       outputDataD[k].tMax = results[counter].maxT[j];
                       outputDataD[k].prec = results[counter].precipitation[j];
                       //printf("%d  %d  %d\n",outputDataD[k].date.day,outputDataD[k].date.month,outputDataD[k].date.year);
                       //printf("%f  %f  %f\n",outputDataD[k].tMin,outputDataD[k].tMax,outputDataD[k].prec);
                       if (results[counter].daySimulated[j] == 28 && results[counter].monthSimulated[j] == 2)
                       {
                            if (isLeapYear(results[counter].yearSimulated[j]))
                            {
                                ++k;
                                outputDataD[k].date.day = results[counter].daySimulated[j] + 1;
                                outputDataD[k].date.month = results[counter].monthSimulated[j];
                                outputDataD[k].date.year = results[counter].yearSimulated[j];
                                outputDataD[k].tMin = results[counter].minT[j];
                                outputDataD[k].tMax = results[counter].maxT[j];
                                outputDataD[k].prec = results[counter].precipitation[j];
                                //printf("%d  %d  %d\n",outputDataD[k].date.day,outputDataD[k].date.month,outputDataD[k].date.year);
                                //printf("%f  %f  %f\n",outputDataD[k].tMin,outputDataD[k].tMax,outputDataD[k].prec);
                            }
                       }

                       k++;


                       //getchar();
                   }
                   //getchar();
                   meteoGridDbHandlerWG2D->meteoGrid()->meteoPointPointer(row,col)->obsDataD = outputDataD;
                   /*for (int j=0;j<lengthArraySimulation+nrYearSimulations;j++)
                   {
                       qDebug() << getQDate(meteoGridDbHandlerWG2D->meteoGrid()->meteoPointPointer(row,col)->obsDataD[j].date);
                       //printf("%f  %f  %f\npress enter to continue",meteoGridDbHandlerWG2D->meteoGrid()->meteoPointPointer(row,col)->obsDataD[j].tMin,meteoGridDbHandlerWG2D->meteoGrid()->meteoPointPointer(row,col)->obsDataD[j].tMax,meteoGridDbHandlerWG2D->meteoGrid()->meteoPointPointer(row,col)->obsDataD[j].prec );

                   }*/
                   meteoGridDbHandlerWG2D->saveCellGridDailyData(&myError, QString::fromStdString(id),row,col,firstDayOutput,lastDayOutput,listMeteoVariable);
                   counter++;
               }
               meteoGridDbHandlerWG2D->meteoGrid()->meteoPointPointer(row,col)->obsDataD.clear();
            }
            //std::cout << row << "\n";
        }
        meteoGridDbHandlerWG2D->closeDatabase();



    }

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

void printSimulationResults(ToutputWeatherData* output,int nrStations,int lengthArray, int nrC)
{
    QString outputName;
    for (int iStation=0; iStation<nrStations;iStation++)
    {
        outputName = "outputData/wgStation_" + QString::number(iStation) + ".csv";
        QFile file(outputName);
        file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
        QTextStream stream( &file );
        for (int m=0; m<lengthArray; m++)
        {
            stream <<  output[iStation].daySimulated[m] << "/" << output[iStation].monthSimulated[m] << "/" << output[iStation].yearSimulated[m] << "," << output[iStation].doySimulated[m] << "," << output[iStation].minT[m]<< "," << output[iStation].maxT[m]<< "," << output[iStation].precipitation[m]<<endl;
        }
        file.close();
    }
}

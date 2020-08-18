#include <QCoreApplication>

#include "commonConstants.h"
#include "utilities.h"
#include "dbMeteoGrid.h"
#include "meteo.h"
#include "meteoPoint.h"
#include "wg2D.h"
#include "crit3dDate.h"
#include "weatherGenerator.h"
#include "wgClimate.h"


#include <QDebug>
#include <iostream>
#include <QFileDialog>
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QString>
#include <malloc.h>
#include <time.h>

#define NR_SIMULATION_YEARS 1
//#define MAX_NR_POINTS 5

// [ 1 - 10 ]
//#define NR_STATIONS 10
#define STARTING_YEAR 2501
#define PREC_THRESHOLD 0.25
void printSimulationResults(double **observed, double **simulated, int nrStations, QString variable, int month);
static Crit3DMeteoGridDbHandler* meteoGridDbHandler;
static Crit3DMeteoGridDbHandler* meteoGridDbHandlerWG2D;
static weatherGenerator2D WG2D;

void logInfo(QString myStr)
{
     std::cout << myStr.toStdString() << std::endl;
}


bool loadMeteoGridDB(QString* errorString)
{
    //QString xmlName = QFileDialog::getOpenFileName(nullptr, "Open XML grid", "", "XML files (*.xml)");

    QString path;
    if (! searchDataPath(&path)) return -1;
    QString xmlName = path + "METEOGRID/DBGridXML_Eraclito4.xml";

    meteoGridDbHandler = new Crit3DMeteoGridDbHandler();

    // todo
    //meteoGridDbHandler->meteoGrid()->setGisSettings(this->gisSettings);

    if (! meteoGridDbHandler->parseXMLGrid(xmlName, errorString)) return false;

    if (! meteoGridDbHandler->openDatabase(errorString))return false;

    if (! meteoGridDbHandler->loadCellProperties(errorString)) return false;

    if (! meteoGridDbHandler->updateGridDate(errorString)) return false;

    logInfo("Meteo Grid = " + xmlName);

    return true;
}

bool loadMeteoGridDBWG2D(QString* errorString)
{
    //QString xmlName = QFileDialog::getOpenFileName(nullptr, "Open XML grid", "", "XML files (*.xml)");

    QString path;
    if (! searchDataPath(&path)) return -1;
    QString xmlName = path + "METEOGRID/DBGridXML_C7_WG2D.xml";

    meteoGridDbHandlerWG2D = new Crit3DMeteoGridDbHandler();

    // todo
    //meteoGridDbHandler->meteoGrid()->setGisSettings(this->gisSettings);

    if (! meteoGridDbHandlerWG2D->parseXMLGrid(xmlName, errorString)) return false;

    if (! meteoGridDbHandlerWG2D->openDatabase(errorString))return false;

    if (! meteoGridDbHandlerWG2D->loadCellProperties(errorString)) return false;

    if (! meteoGridDbHandlerWG2D->updateGridDate(errorString)) return false;

    logInfo("Meteo Grid = " + xmlName);

    return true;
}

/*
bool saveOnMeteoGridDB(QString* errorString)
{
    //QString xmlName = QFileDialog::getOpenFileName(nullptr, "Open XML grid", "", "XML files (*.xml)");
    QString path;
    if (! searchDataPath(&path)) return -1;
    QString xmlName = path + "METEOGRID/DBGridXML_Eraclito_WG2D.xml";

    meteoGridDbHandlerWG2D = new Crit3DMeteoGridDbHandler();

    // todo
    //meteoGridDbHandler->meteoGrid()->setGisSettings(this->gisSettings);

    if (! meteoGridDbHandlerWG2D->parseXMLGrid(xmlName, errorString)) return false;

    if (! meteoGridDbHandlerWG2D->openDatabase(errorString))return false;

    if (! meteoGridDbHandlerWG2D->loadCellProperties(errorString)) return false;

    //if (! meteoGridDbHandlerWG2D->updateGridDate(errorString)) return false;

    logInfo("Meteo Grid = " + xmlName);

    return true;
}*/

int main(int argc, char *argv[])
{
    int startingYear = STARTING_YEAR;
    printf("insert the starting year for the synthethic series:\n");
    //scanf("%d",&startingYear);
    startingYear = 2001;
    int nrYearSimulations = NR_SIMULATION_YEARS;
    printf("insert the number of years of the the synthethic series:\n");
    //scanf("%d",&nrYearSimulations);
    nrYearSimulations = 30;
    time_t rawtime;
    struct tm * timeinfo;
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    printf ( "Current local time and date: %s", asctime (timeinfo) );

    QApplication myApp(argc, argv);
    QString appPath = myApp.applicationDirPath() + "/";

    QString myError;
    //Crit3DMeteoPoint* meteoPointTemp = new Crit3DMeteoPoint;
    meteoVariable variable;
    QDate firstDay(1961,1,1);
    QDate lastDay(1990,12,31);
    QDate currentDay;
    QDate firstDateDB(1,1,1);
    TObsDataD** obsDataD = nullptr;

    QString errorString;
    if (! loadMeteoGridDB(&errorString))
    {
        std::cout << errorString.toStdString() << std::endl;
        return -1;
    }

    std::string id;
    int nrActivePoints = 0;
    int lengthSeries = 0;
    std::vector<float> dailyVariable;


    for (int row = 0; row < meteoGridDbHandler->gridStructure().header().nrRows; row++)
    {

        for (int col = 0; col < meteoGridDbHandler->gridStructure().header().nrCols; col++)
        {

           if (meteoGridDbHandler->meteoGrid()->getMeteoPointActiveId(row, col, &id))
           {
               ++nrActivePoints;
               if (nrActivePoints == 1)
               {
                   variable = dailyAirTemperatureMin;
                   dailyVariable = meteoGridDbHandler->loadGridDailyVar(&myError, QString::fromStdString(id),
                                                                        variable, firstDay, lastDay, &firstDateDB);
                   lengthSeries = int(dailyVariable.size());
               }
           }
        }
    }

    #ifdef MAX_NR_POINTS
        nrActivePoints = MINVALUE(nrActivePoints, MAX_NR_POINTS);
    #endif

    printf("%d  %d\n", lengthSeries,nrActivePoints);
    obsDataD = (TObsDataD **)calloc(nrActivePoints, sizeof(TObsDataD*));
    for (int i=0;i<nrActivePoints;i++)
    {
        obsDataD[i] = (TObsDataD *)calloc(lengthSeries, sizeof(TObsDataD));
    }

    for (int i=0;i<nrActivePoints;i++)
    {
        currentDay = firstDay;
        for (int j=0;j<lengthSeries;j++)
        {
            obsDataD[i][j].date.day = currentDay.day();
            obsDataD[i][j].date.month = currentDay.month();
            obsDataD[i][j].date.year = currentDay.year();
            currentDay = currentDay.addDays(1);
        }
    }

    printf("loading data...\n");

    int counter = 0;
    for (int row = 0; row < meteoGridDbHandler->gridStructure().header().nrRows; row++)
    {

        for (int col = 0; col < meteoGridDbHandler->gridStructure().header().nrCols; col++)
        {

           if (meteoGridDbHandler->meteoGrid()->getMeteoPointActiveId(row, col, &id) && counter<nrActivePoints)
           {

               variable = dailyAirTemperatureMin;
               dailyVariable = meteoGridDbHandler->loadGridDailyVar(&myError, QString::fromStdString(id),
                                                                    variable, firstDay, lastDay, &firstDateDB);
               for (int iLength=0; iLength<lengthSeries; iLength++) obsDataD[counter][iLength].tMin = dailyVariable[iLength];
               variable = dailyAirTemperatureMax;
               dailyVariable = meteoGridDbHandler->loadGridDailyVar(&myError, QString::fromStdString(id),
                                                                    variable, firstDay, lastDay, &firstDateDB);
               for (int iLength=0; iLength<lengthSeries; iLength++) obsDataD[counter][iLength].tMax = dailyVariable[iLength];
               variable = dailyPrecipitation;
               dailyVariable = meteoGridDbHandler->loadGridDailyVar(&myError, QString::fromStdString(id),
                                                                    variable, firstDay, lastDay, &firstDateDB);
               for (int iLength=0; iLength<lengthSeries; iLength++) obsDataD[counter][iLength].prec = dailyVariable[iLength];
               counter++;
           }
        }
        //std::cout << row << "\n";
    }

    dailyVariable.clear();
    meteoGridDbHandler->closeDatabase();

    printf("weather generator\n");

    int lengthArraySimulation;
    lengthArraySimulation = 365 * nrYearSimulations;

    int nrLeapYears = 0;
    for (int iYear = startingYear;iYear<startingYear+nrYearSimulations;iYear++ )
    {
        nrLeapYears += isLeapYear(iYear);
    }
    //outputDataD.resize(lengthArraySimulation+nrLeapYears);
    TObsDataD** outputDataD;
    outputDataD = (TObsDataD **)calloc(nrActivePoints, sizeof(TObsDataD*));
    for (int i=0;i<nrActivePoints;i++)
    {
        outputDataD[i] = (TObsDataD *)calloc(lengthArraySimulation, sizeof(TObsDataD));
    }
    QDate firstDayOutput(startingYear,1,1);
    QDate lastDayOutput(startingYear+nrYearSimulations-1,12,31);
    if (! loadMeteoGridDBWG2D(&errorString))
    {
        std::cout << errorString.toStdString() << std::endl;
        return -1;
    }
    for (int i=0;i<nrActivePoints;i++)
    {
        currentDay = firstDayOutput;
        for (int j=0;j<lengthArraySimulation;j++)
        {
            outputDataD[i][j].date.day = currentDay.day();
            outputDataD[i][j].date.month = currentDay.month();
            outputDataD[i][j].date.year = currentDay.year();
            currentDay = currentDay.addDays(1);
        }
    }
    printf("loading data...\n");

    counter = 0;
    for (int row = 0; row < meteoGridDbHandlerWG2D->gridStructure().header().nrRows; row++)
    {

        for (int col = 0; col < meteoGridDbHandlerWG2D->gridStructure().header().nrCols; col++)
        {

           if (meteoGridDbHandlerWG2D->meteoGrid()->getMeteoPointActiveId(row, col, &id) && counter<nrActivePoints)
           {

               variable = dailyAirTemperatureMin;
               dailyVariable = meteoGridDbHandlerWG2D->loadGridDailyVar(&myError, QString::fromStdString(id),
                                                                    variable, firstDayOutput, lastDayOutput, &firstDateDB);
               for (int iLength=0; iLength<lengthArraySimulation; iLength++) outputDataD[counter][iLength].tMin = dailyVariable[iLength];
               variable = dailyAirTemperatureMax;
               dailyVariable = meteoGridDbHandlerWG2D->loadGridDailyVar(&myError, QString::fromStdString(id),
                                                                    variable, firstDayOutput, lastDayOutput, &firstDateDB);
               for (int iLength=0; iLength<lengthArraySimulation; iLength++) outputDataD[counter][iLength].tMax = dailyVariable[iLength];
               variable = dailyPrecipitation;
               dailyVariable = meteoGridDbHandlerWG2D->loadGridDailyVar(&myError, QString::fromStdString(id),
                                                                    variable, firstDayOutput, lastDayOutput, &firstDateDB);
               for (int iLength=0; iLength<lengthArraySimulation; iLength++) outputDataD[counter][iLength].prec = dailyVariable[iLength];
               counter++;
           }
        }
        //std::cout << row << "\n";
    }
    printf("%d  %d\n", lengthArraySimulation,nrActivePoints);
    //getchar();
    dailyVariable.clear();

    meteoGridDbHandlerWG2D->closeDatabase();

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    printf ( "Start statistics\nCurrent local time and date: %s", asctime (timeinfo) );

    // compute statistics
    QString outputFileName;
    Crit3DDate inputFirstDate;
    float* inputTMin;
    float* inputTMax;
    float* inputPrec;
    inputTMin = (float*)calloc(lengthSeries, sizeof(float));
    inputTMax = (float*)calloc(lengthSeries, sizeof(float));
    inputPrec = (float*)calloc(lengthSeries, sizeof(float));
    // compute climate statistics from observed data
    for (int iStation=0;iStation<nrActivePoints;iStation++)
    {
        outputFileName = "wgClimate_station_" + QString::number(iStation) + ".txt";
        inputFirstDate.day = obsDataD[iStation][0].date.day;
        inputFirstDate.month = obsDataD[iStation][0].date.month;
        inputFirstDate.year = obsDataD[iStation][0].date.year;
        int nrDays = nrActivePoints;
        for (int i=0;i<nrDays;i++)
        {
            inputTMin[i] = obsDataD[iStation][i].tMin;
            inputTMax[i] = obsDataD[iStation][i].tMax;
            inputPrec[i] = obsDataD[iStation][i].prec;
        }
        // float
        float minPrecData = NODATA;
        TweatherGenClimate weatherGenClimate;
        float* monthlyPrec = (float *)calloc(12, sizeof(float));
        computeWG2DClimate(nrDays,inputFirstDate,inputTMin,inputTMax,inputPrec,PREC_THRESHOLD,minPrecData,&weatherGenClimate,true,outputFileName,monthlyPrec);
        free(monthlyPrec);
    }
    free(inputTMin);
    free(inputTMax);
    free(inputPrec);

    //float* inputTMin;
    //float* inputTMax;
    //float* inputPrec;
    inputTMin = (float*)calloc(lengthArraySimulation, sizeof(float));
    inputTMax = (float*)calloc(lengthArraySimulation, sizeof(float));
    inputPrec = (float*)calloc(lengthArraySimulation, sizeof(float));
    // compute climate statistics from observed data
    for (int iStation=0;iStation<nrActivePoints;iStation++)
    {
        outputFileName = "wgSimulation_station_" + QString::number(iStation) + ".txt";
        inputFirstDate.day = outputDataD[iStation][0].date.day;
        inputFirstDate.month = outputDataD[iStation][0].date.month;
        inputFirstDate.year = outputDataD[iStation][0].date.year;
        int nrDays = nrActivePoints;
        for (int i=0;i<nrDays;i++)
        {
            inputTMin[i] = outputDataD[iStation][i].tMin;
            inputTMax[i] = outputDataD[iStation][i].tMax;
            inputPrec[i] = outputDataD[iStation][i].prec;
        }
        // float
        float minPrecData = NODATA;
        TweatherGenClimate weatherGenClimate;
        float* monthlyPrec = (float *)calloc(12, sizeof(float));
        computeWG2DClimate(nrDays,inputFirstDate,inputTMin,inputTMax,inputPrec,PREC_THRESHOLD,minPrecData,&weatherGenClimate,true,outputFileName,monthlyPrec);
        free (monthlyPrec);
    }
    free(inputTMin);
    free(inputTMax);
    free(inputPrec);

    /*
    double** correlationMatrix;
    correlationMatrix = (double **)calloc(nrActivePoints, sizeof(double*));
    for (int i=0; i<nrActivePoints; i++)
    {
        correlationMatrix[i] = (double *)calloc(nrActivePoints, sizeof(double));
    }
    double** correlationMatrixSimulation;
    correlationMatrixSimulation = (double **)calloc(nrActivePoints, sizeof(double*));
    for (int i=0; i<nrActivePoints; i++)
    {
        correlationMatrixSimulation[i] = (double *)calloc(nrActivePoints, sizeof(double));
    }
    int counterSimulation = 0;
    // 1 correlation matrices
    QString variableToPrint;



    for (int iMonth = 0; iMonth<12 ; iMonth++)
    {
        counterSimulation = counter = 0;
        for (int i=0; i<lengthSeries; i++)
        {
            if (obsDataD[0][i].date.month == iMonth+1) counter++;
        }
        for (int i=0; i<lengthArraySimulation; i++)
        {
            if (outputDataD[0][i].date.month == iMonth+1) counterSimulation++;
        }
        //printf("%f\n", counterSimulation);
        double** arrayVariable;
        arrayVariable = (double **)calloc(nrActivePoints, sizeof(double*));
        for (int i=0; i<nrActivePoints; i++)
        {
            arrayVariable[i] = (double *)calloc(counter, sizeof(double));
        }
        double** arrayVariableSimulation;
        arrayVariableSimulation = (double **)calloc(nrActivePoints, sizeof(double*));
        for (int i=0; i<nrActivePoints; i++)
        {
            arrayVariableSimulation[i] = (double *)calloc(counterSimulation, sizeof(double));
        }
        int cObs,cSim;

        for (int i=0; i<nrActivePoints; i++)
        {
            cObs = cSim = 0;
            for (int j=0; j<counter; j++)
            {
                while (obsDataD[i][cObs].date.month != iMonth + 1)
                {
                    cObs++;
                }
                arrayVariable[i][j] = obsDataD[i][cObs].tMax;
                cObs++;
            }
            for (int j=0; j<counterSimulation; j++)
            {
                while (outputDataD[i][cSim].date.month != iMonth + 1)
                {
                    cSim++;
                }
                //printf("%d\n",cSim);
                arrayVariableSimulation[i][j] = outputDataD[i][cSim].tMax;
                cSim++;
            }
        }
        statistics::correlationsMatrix(nrActivePoints,arrayVariable,counter,correlationMatrix);
        statistics::correlationsMatrix(nrActivePoints,arrayVariableSimulation,counterSimulation,correlationMatrixSimulation);
        variableToPrint = "Tmax";
        printSimulationResults(correlationMatrix,correlationMatrixSimulation,nrActivePoints,variableToPrint, iMonth+1);

        for (int i=0; i<nrActivePoints; i++)
        {
            cSim = cObs = 0;
            for (int j=0; j<counter; j++)
            {
                while (obsDataD[i][cObs].date.month != iMonth + 1)
                {
                    cObs++;
                }
                arrayVariable[i][j] = obsDataD[i][cObs].tMin;
                cObs++;
            }
            for (int j=0; j<counterSimulation; j++)
            {
                while (outputDataD[i][cSim].date.month != iMonth + 1)
                {
                    cSim++;
                }
                arrayVariableSimulation[i][j] = outputDataD[i][cSim].tMin;
                cSim++;
            }

        }
        statistics::correlationsMatrix(nrActivePoints,arrayVariable,counter,correlationMatrix);
        statistics::correlationsMatrix(nrActivePoints,arrayVariableSimulation,counterSimulation,correlationMatrixSimulation);
        variableToPrint = "Tmin";
        printSimulationResults(correlationMatrix,correlationMatrixSimulation,nrActivePoints,variableToPrint, iMonth+1);
        cSim = cObs = 0;
        for (int i=0; i<nrActivePoints; i++)
        {
            cSim = cObs = 0;
            for (int j=0; j<counter; j++)
            {
                while (obsDataD[i][cObs].date.month != iMonth + 1)
                {
                    cObs++;
                }
                arrayVariable[i][j] = obsDataD[i][cObs].prec;
                cObs++;
            }
            for (int j=0; j<counterSimulation; j++)
            {
                while (outputDataD[i][cSim].date.month != iMonth + 1)
                {
                    cSim++;
                }
                arrayVariableSimulation[i][j] = outputDataD[i][cSim].prec;
                cSim++;
            }
        }
        statistics::correlationsMatrix(nrActivePoints,arrayVariable,counter,correlationMatrix);
        statistics::correlationsMatrix(nrActivePoints,arrayVariableSimulation,counterSimulation,correlationMatrixSimulation);
        variableToPrint = "Prec";
        printSimulationResults(correlationMatrix,correlationMatrixSimulation,nrActivePoints,variableToPrint, iMonth+1);
        for (int i=0; i<nrActivePoints; i++)
        {
            free(arrayVariable[i]);
            free(arrayVariableSimulation[i]);
        }
        free(arrayVariable);
        free(arrayVariableSimulation);
    }
    */

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    printf ( "Current local time and date: %s", asctime (timeinfo) );


    return 0;
}

void printSimulationResults(double** observed,double** simulated,int nrStations, QString variable, int month)
{
    //FILE* fp;
    QString outputName;
    outputName = "correlationMatrix" + variable + "month" + QString::number(month)+".csv";
    QFile file(outputName);
    file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
    QTextStream stream( &file );

    for (int i=0; i<nrStations;i++)
    {
        for (int m=i; m<nrStations; m++)
        {
            stream <<  observed[i][m] << "," << simulated[i][m] << "," << observed[i][m]-simulated[i][m] <<endl;
        }

    }
    file.close();
}
/*
void weatherGenerator2D::precipitationCorrelationMatricesSimulation()
{
    int counter =0;
    TcorrelationVar amount,occurrence;
    TcorrelationMatrix* correlationMatrixSimulation = nullptr;
    correlationMatrixSimulation = (TcorrelationMatrix*)calloc(12, sizeof(TcorrelationMatrix));
    for (int iMonth=0;iMonth<12;iMonth++)
    {
        correlationMatrixSimulation[iMonth].amount = (double**)calloc(nrStations, sizeof(double*));
        correlationMatrixSimulation[iMonth].occurrence = (double**)calloc(nrStations, sizeof(double*));
        for (int i=0;i<nrStations;i++)
        {
            correlationMatrixSimulation[iMonth].amount[i]= (double*)calloc(nrStations, sizeof(double));
            correlationMatrixSimulation[iMonth].occurrence[i]= (double*)calloc(nrStations, sizeof(double));
            for (int ii=0;ii<nrStations;ii++)
            {
                correlationMatrixSimulation[iMonth].amount[i][ii]= NODATA;
                correlationMatrixSimulation[iMonth].occurrence[i][ii]= NODATA;
            }
        }
    }
    for (int iMonth=0;iMonth<12;iMonth++)
    {
        correlationMatrixSimulation[iMonth].month = iMonth + 1 ; // define the month of the correlation matrix;
        for (int k=0; k<nrStations;k++) // correlation matrix diagonal elements;
        {
            correlationMatrixSimulation[iMonth].amount[k][k] = 1.;
            correlationMatrixSimulation[iMonth].occurrence[k][k]= 1.;
        }

        for (int j=0; j<nrStations-1;j++)
        {
            for (int i=j+1; i<nrStations;i++)
            {
                counter = 0;
                amount.meanValue1=0.;
                amount.meanValue2=0.;
                amount.covariance = amount.variance1 = amount.variance2 = 0.;
                occurrence.meanValue1=0.;
                occurrence.meanValue2=0.;
                occurrence.covariance = occurrence.variance1 = occurrence.variance2 = 0.;

                for (int k=0; k<365*parametersModel.yearOfSimulation;k++) // compute the monthly means
                {
                    int doy,dayCurrent,monthCurrent;
                    dayCurrent = monthCurrent = 0;
                    doy = k%365 + 1;
                    weatherGenerator2D::dateFromDoy(doy,2001,&dayCurrent,&monthCurrent);
                    if (monthCurrent == (iMonth+1))
                    {
                        if (((outputWeatherData[j].precipitation[k] - NODATA) > EPSILON) && ((outputWeatherData[i].precipitation[k] - NODATA) > EPSILON))
                        {
                            counter++;
                            if (outputWeatherData[j].precipitation[k] > parametersModel.precipitationThreshold)
                            {
                                amount.meanValue1 += outputWeatherData[j].precipitation[k] ;
                                occurrence.meanValue1++ ;
                            }
                            if (outputWeatherData[i].precipitation[k] > parametersModel.precipitationThreshold)
                            {
                                amount.meanValue2 += outputWeatherData[i].precipitation[k];
                                occurrence.meanValue2++ ;
                            }
                        }
                    }
                }
                if (counter != 0)
                {
                    amount.meanValue1 /= counter;
                    occurrence.meanValue1 /= counter;
                }

                if (counter != 0)
                {
                    amount.meanValue2 /= counter;
                    occurrence.meanValue2 /= counter;
                }
                // compute the monthly rho off-diagonal elements
                for (int k=0; k<365*parametersModel.yearOfSimulation;k++)
                {
                    int doy,dayCurrent,monthCurrent;
                    dayCurrent = monthCurrent = 0;
                    doy = k%365+1;
                    weatherGenerator2D::dateFromDoy(doy,2001,&dayCurrent,&monthCurrent);
                    if (monthCurrent == (iMonth+1))
                    {
                        if ((outputWeatherData[j].precipitation[k] != NODATA) && (outputWeatherData[i].precipitation[k] != NODATA))
                        {
                            double value1,value2;
                            if (outputWeatherData[j].precipitation[k] <= parametersModel.precipitationThreshold) value1 = 0.;
                            else value1 = outputWeatherData[j].precipitation[k];
                            if (outputWeatherData[i].precipitation[k] <= parametersModel.precipitationThreshold) value2 = 0.;
                            else value2 = outputWeatherData[i].precipitation[k];

                            amount.covariance += (value1 - amount.meanValue1)*(value2 - amount.meanValue2);
                            amount.variance1 += (value1 - amount.meanValue1)*(value1 - amount.meanValue1);
                            amount.variance2 += (value2 - amount.meanValue2)*(value2 - amount.meanValue2);

                            if (outputWeatherData[j].precipitation[k] <= parametersModel.precipitationThreshold) value1 = 0.;
                            else value1 = 1.;
                            if (outputWeatherData[i].precipitation[k] <= parametersModel.precipitationThreshold) value2 = 0.;
                            else value2 = 1.;

                            occurrence.covariance += (value1 - occurrence.meanValue1)*(value2 - occurrence.meanValue2);
                            occurrence.variance1 += (value1 - occurrence.meanValue1)*(value1 - occurrence.meanValue1);
                            occurrence.variance2 += (value2 - occurrence.meanValue2)*(value2 - occurrence.meanValue2);
                        }
                    }
                }
                correlationMatrixSimulation[iMonth].amount[j][i]= amount.covariance / sqrt(amount.variance1*amount.variance2);
                correlationMatrixSimulation[iMonth].amount[i][j] = correlationMatrixSimulation[iMonth].amount[j][i];
                correlationMatrixSimulation[iMonth].occurrence[j][i]= occurrence.covariance / sqrt(occurrence.variance1*occurrence.variance2);
                correlationMatrixSimulation[iMonth].occurrence[i][j] = correlationMatrixSimulation[iMonth].occurrence[j][i];
            }
        }

    }
    FILE* fp;
    fp = fopen("correlationMatrices.txt","w");
    for (int iMonth=0;iMonth<12;iMonth++)
    {
        fprintf(fp,"month %d \nsimulated - observed\n",iMonth+1);
        for (int i=0;i<nrStations;i++)
        {
            for (int j=0;j<nrStations;j++)
            {
                fprintf(fp,"%.2f ", correlationMatrixSimulation[iMonth].amount[j][i]-correlationMatrix[iMonth].amount[j][i]);
            }
            fprintf(fp,"\n");
        }
    }
    fclose(fp);
    for (int iMonth=0;iMonth<12;iMonth++)
    {
        for (int i=0;i<nrStations;i++)
        {
            free(correlationMatrixSimulation[iMonth].amount[i]);
            free(correlationMatrixSimulation[iMonth].occurrence[i]);
        }
    }
    free(correlationMatrixSimulation);
}*/

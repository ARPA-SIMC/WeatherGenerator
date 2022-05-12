#include "commonConstants.h"
#include "utilities.h"
#include "dbMeteoGrid.h"
#include "meteo.h"
#include "meteoPoint.h"
#include "wg2D.h"
#include "crit3dDate.h"
#include "gammaFunction.h"

#include <QDebug>
#include <iostream>
#include <QFile>
#include <QCoreApplication>
#include <QTextStream>
#include <QString>
//#include <QFileDialog>
#include <malloc.h>
#include <time.h>

#define NR_SIMULATION_YEARS 10
//#define MAX_NR_POINTS 10

// [ 1 - 10 ]
//#define NR_STATIONS 10
#define STARTING_YEAR 1
#define PREC_THRESHOLD 0.25
#define DISTRIBUTION_MULTIEXPONENTIAL 1
#define DISTRIBUTION_MULTIGAMMA 2
#define DISTRIBUTION_MULTIWEIBULL 3

void printSimulationResults(ToutputWeatherData* output,int nrStations,int lengthArray);
static Crit3DMeteoGridDbHandler* meteoGridDbHandler;
static Crit3DMeteoGridDbHandler* meteoGridDbHandlerWG2D;
static weatherGenerator2D WG2D;

int readERG5CellListNumber(FILE *fp)
{
    int counter = 0;
    char dummy;

    do {
        dummy = getc(fp);
        if (dummy == '\n' || dummy == EOF) counter++ ;
    } while (dummy != EOF);
    return counter ;
}

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
    //QString xmlName = path + "METEOGRID/DBGridXML_ERG5_v2.1.xml";
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

bool saveOnMeteoGridDB(QString* errorString)
{
    //QString xmlName = QFileDialog::getOpenFileName(nullptr, "Open XML grid", "", "XML files (*.xml)");
    QString path;
    if (! searchDataPath(&path)) return -1;
    QString xmlName = path + "METEOGRID/DBGridXML_Output_WG2D_tomei_server.xml";

    meteoGridDbHandlerWG2D = new Crit3DMeteoGridDbHandler();

    // todo
    //meteoGridDbHandler->meteoGrid()->setGisSettings(this->gisSettings);

    if (! meteoGridDbHandlerWG2D->parseXMLGrid(xmlName, errorString)) return false;

    if (! meteoGridDbHandlerWG2D->openDatabase(errorString))return false;

    if (! meteoGridDbHandlerWG2D->loadCellProperties(errorString)) return false;

    //if (! meteoGridDbHandlerWG2D->updateGridDate(errorString)) return false;

    logInfo("Meteo Grid = " + xmlName);

    return true;
}

int main(int argc, char *argv[])
{
    float x = 2.5;
    float y;
    double alpha = 1.5;
    double beta = 0.5;
    double pZero = 0;
    //y = generalizedGammaCDF(x,beta,alpha,pZero);
    //printf("%f  %f\n",x,y);
    x = inverseGeneralizedGammaCDF(generalizedGammaCDF(x,beta,alpha,pZero),alpha,beta,0.000000001,pZero);
    //
    //y = incompleteGamma(alpha, double(x) / beta);
    //x = inverseGammaCumulativeDistributionFunction(y,alpha,beta,0.000000001);
    printf("%f\n",x);
    return 0;
    //printf("Do you want to write the output on database? type : 1 (yes) , or 0 (no) \n");
    int writeOnDB;
    //scanf("%d",&writeOnDB);
    writeOnDB = 0;
    int startingYear = STARTING_YEAR;
    printf("insert the starting year for the synthethic series:\n");
    scanf("%d",&startingYear);
    //startingYear = 2001;
    int nrYearSimulations = NR_SIMULATION_YEARS;
    printf("insert the number of years of the the synthethic series:\n");
    scanf("%d",&nrYearSimulations);
    //nrYearSimulations = 10;
    time_t rawtime;
    struct tm * timeinfo;
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    printf ( "Current local time and date: %s", asctime (timeinfo) );

    QCoreApplication myApp(argc, argv);
    QString appPath = myApp.applicationDirPath() + "/";

    QString myError;
    //Crit3DMeteoPoint* meteoPointTemp = new Crit3DMeteoPoint;
    meteoVariable variable;
    QDate firstDay(1961,1,1);
    QDate lastDay(2020,12,31);
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
    FILE* fp;
    //fp = fopen("./inputData/list_enza_secchia_panaro.txt","r"); // !! take out
    fp = fopen("./inputData/list_C7_shortlisted_few_sites.txt","r"); // !! take out
    //fp = fopen("./inputData/list_C7_russi.txt","r"); // !! take out
    //fp = fopen("./inputData/list_C7_shortlisted.txt","r"); // !! take out
    //fp = fopen("./inputData/list_C4.txt","r"); // !! take out
    //fp = fopen("./inputData/list_C7_Russi.txt","r"); // !! take out
    //fp = fopen("./inputData/list_farini_00296.txt","r"); // !! take out
    //fp = fopen("./inputData/centro.txt","r"); // !! take out
    //fp = fopen("./inputData/fausto.txt","r");

    int numberOfCells; // !! take out
    numberOfCells = readERG5CellListNumber(fp); // !! take out
    fclose(fp); // !! take out

    //fp = fopen("./inputData/list_enza_secchia_panaro.txt","r"); // !! take out
    fp = fopen("./inputData/list_C7_shortlisted_few_sites.txt","r"); // !! take out
    //fp = fopen("./inputData/list_C7_russi.txt","r"); // !! take out
    //fp = fopen("./inputData/list_C7_shortlisted.txt","r"); // !! take out
    //fp = fopen("./inputData/list_C4.txt","r"); // !! take out
    //fp = fopen("./inputData/list_C7_Russi.txt","r"); // !! take out
    //fp = fopen("./inputData/list_farini_00296.txt","r"); // !! take out
    //fp = fopen("./inputData/centro.txt","r"); // !! take out
    //fp = fopen("./inputData/fausto.txt","r");

    int* cellCode = nullptr; // !! take out
    char* numCell = (char *)calloc(6, sizeof(char)); // !! take out
    cellCode = (int *) calloc(numberOfCells, sizeof(int)); // !! take out
    for (int i=0; i<numberOfCells; i++) // !! take out
    { // !! take out
        readTheCellNumber(fp,numCell);// !! take out
        cellCode[i] = atoi(numCell);// !! take out
    }// !! take out
    fclose(fp);


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
    if (nrActivePoints > numberOfCells) nrActivePoints = numberOfCells; // !! take out
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

    printf("load data...\n");

    int counter = 0;
    for (int row = 0; row < meteoGridDbHandler->gridStructure().header().nrRows; row++)
    {

        for (int col = 0; col < meteoGridDbHandler->gridStructure().header().nrCols; col++)
        {
            bool isConsortiumCell = false; // !! take out
            int iCells = 0; // !! take out
            if (meteoGridDbHandler->meteoGrid()->getMeteoPointActiveId(row, col, &id)) // !! take out
            { // !! take out

                while (iCells < nrActivePoints) // !! take out
                {
                    int idCellInt;  // !! take out
                    idCellInt = atoi(id.c_str());// !! take out
                    if (idCellInt == cellCode[iCells] ) isConsortiumCell = true; // !! take out
                     iCells++; // !! take out
                } // !! take out
            } // !! take out

           if (meteoGridDbHandler->meteoGrid()->getMeteoPointActiveId(row, col, &id) && counter<nrActivePoints && isConsortiumCell) // !! to modify
           {

               printf(" %d %d \n",row,col);
               variable = dailyAirTemperatureMin;
               dailyVariable = meteoGridDbHandler->loadGridDailyVar(&myError, QString::fromStdString(id),
                                                                    variable, firstDay, lastDay, &firstDateDB);
               for (int iLength=0; iLength<lengthSeries; iLength++) obsDataD[counter][iLength].tMin = dailyVariable[iLength];
               //for (int iLength=0; iLength<lengthSeries; iLength++) printf("temp %f\n",dailyVariable[iLength]);
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
    free(meteoGridDbHandler);
    WG2D.initializeData(lengthSeries,nrActivePoints);
    WG2D.setObservedData(obsDataD);
    free(obsDataD);
    ToutputWeatherData* results;
    bool computePrecipitation = true;
    bool computeTemperature = true;
    printf("weather generator\n");

    int lengthArraySimulation;
    static int distributionType = DISTRIBUTION_MULTIWEIBULL;
    lengthArraySimulation = 365 * nrYearSimulations;
    bool computeStats = true;
    WG2D.initializeParameters(PREC_THRESHOLD, nrYearSimulations, distributionType,
                              computePrecipitation, computeTemperature,computeStats,ROLLING_AVERAGE);

    WG2D.computeWeatherGenerator2D();

    results = WG2D.getWeatherGeneratorOutput(startingYear);
    //printSimulationResults(results,nrActivePoints,lengthArraySimulation);
    if (writeOnDB != 1)
    {
        return 0;
    }

    std::vector<TObsDataD> outputDataD;
    int nrLeapYears = 0;
    for (int iYear = startingYear;iYear<startingYear+nrYearSimulations;iYear++ )
    {
        nrLeapYears += isLeapYear(iYear);
    }
    outputDataD.resize(lengthArraySimulation+nrLeapYears);

    // fill the new database
    QDate firstDayOutput(startingYear,1,1);
    QDate lastDayOutput(startingYear+nrYearSimulations-1,12,31);

    if (! saveOnMeteoGridDB(&errorString))
    {
        std::cout << errorString.toStdString() << std::endl;
        return -1;
    }
    QList<meteoVariable> listMeteoVariable = {dailyAirTemperatureMin,dailyAirTemperatureMax,dailyPrecipitation};
    counter = 0;
    Crit3DMeteoSettings *meteoSettings;
    for (int row = 0; row < meteoGridDbHandlerWG2D->gridStructure().header().nrRows; row++)
    {
        for (int col = 0; col < meteoGridDbHandlerWG2D->gridStructure().header().nrCols; col++)
        {

            bool isConsortiumCell = false; // !! take out
            int iCells = 0; // !! take out
            if (meteoGridDbHandlerWG2D->meteoGrid()->getMeteoPointActiveId(row, col, &id)) // !! take out
            { // !! take out

                while (iCells < nrActivePoints) // !! take out
                {
                    int idCellInt;  // !! take out
                    idCellInt = atoi(id.c_str());// !! take out
                    if (idCellInt == cellCode[iCells] ) isConsortiumCell = true; // !! take out
                     iCells++; // !! take out
                } // !! take out
            } // !! take out
           if (meteoGridDbHandlerWG2D->meteoGrid()->getMeteoPointActiveId(row, col, &id) && counter<nrActivePoints && isConsortiumCell)
           {
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
                        }
                   }
                   k++;
               }

               meteoGridDbHandlerWG2D->meteoGrid()->meteoPointPointer(row,col)->obsDataD = outputDataD;
               /*for (int j=0;j<lengthArraySimulation+nrYearSimulations;j++)
               {
                   qDebug() << getQDate(meteoGridDbHandlerWG2D->meteoGrid()->meteoPointPointer(row,col)->obsDataD[j].date);
                   //printf("%f  %f  %f\npress enter to continue",meteoGridDbHandlerWG2D->meteoGrid()->meteoPointPointer(row,col)->obsDataD[j].tMin,meteoGridDbHandlerWG2D->meteoGrid()->meteoPointPointer(row,col)->obsDataD[j].tMax,meteoGridDbHandlerWG2D->meteoGrid()->meteoPointPointer(row,col)->obsDataD[j].prec );

               }*/
               counter++;
               if (!meteoGridDbHandlerWG2D->saveCellGridDailyData(&myError, QString::fromStdString(id), row, col, firstDayOutput,
                                                                            lastDayOutput, listMeteoVariable, meteoSettings))
               {
                   //printf(myError.toStdString().c_str());
                   printf("\n");
               }
               else
               {
                   printf("saved table nr. %d\n",counter);
               }
               /*
               if (!meteoGridDbHandlerWG2D->deleteAndWriteCellGridDailyData(myError, QString::fromStdString(id), row, col, firstDayOutput,
                                                                            lastDayOutput, listMeteoVariable, meteoSettings))
               {
                   printf(myError.toStdString().c_str());
                   printf("\n");
               }
               else
               {
                   printf("saved table nr. %d\n",counter);
               }
               */
               //meteoGridDbHandlerWG2D->saveCellGridDailyData(&myError,QString::fromStdString(id),row,col,firstDayOutput,lastDayOutput,listMeteoVariable,meteoSettings);
           }
           meteoGridDbHandlerWG2D->meteoGrid()->meteoPointPointer(row,col)->obsDataD.clear();
        }
    }
    meteoGridDbHandlerWG2D->closeDatabase();
    free(meteoGridDbHandlerWG2D);
    return 0;
}

void printSimulationResults(ToutputWeatherData* output,int nrStations,int lengthArray)
{
    QString outputName;
    for (int iStation=0; iStation<nrStations;iStation++)
    {
        outputName = "wgStation_" + QString::number(iStation) + ".csv";
        QFile file(outputName);
        file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
        QTextStream stream( &file );
        for (int m=0; m<lengthArray; m++)
        {
            if (output[iStation].precipitation[m] < 0.2)
            {
                stream <<  output[iStation].daySimulated[m] << "/" << output[iStation].monthSimulated[m] << "/" << output[iStation].yearSimulated[m] << "," << output[iStation].doySimulated[m] << "," << output[iStation].minT[m]<< "," << output[iStation].maxT[m]<< "," << output[iStation].precipitation[m]<< "," << output[iStation].minTClimate[m]<< "," << output[iStation].maxTClimate[m]<<endl;

            }
            else
            {
                stream <<  output[iStation].daySimulated[m] << "/" << output[iStation].monthSimulated[m] << "/" << output[iStation].yearSimulated[m] << "," << output[iStation].doySimulated[m] << "," << output[iStation].minT[m]<< "," << output[iStation].maxT[m]<< "," << output[iStation].precipitation[m]<< "," << -10 << "," << -10 <<endl;
            }
        }
        file.close();
    }

}


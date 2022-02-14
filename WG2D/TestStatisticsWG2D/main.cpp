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
#include <iostream>
#include <QFile>
#include <QTextStream>
#include <QString>
#include <malloc.h>
#include <time.h>


struct TmonthlyData{
    int month[12];
    double averageTmin[12];
    double averageTmax[12];
    double sumPrec[12];
    double stdDevTmin[12];
    double stdDevTmax[12];
    double fractionWetDays[12];
    double fractionWetWet[12];
    double dewPointTmax[12];
    double dryAverageTmin[12];
    double dryAverageTmax[12];
    double wetAverageTmin[12];
    double wetAverageTmax[12];
    double dryStdDevTmin[12];
    double dryStdDevTmax[12];
    double wetStdDevTmin[12];
    double wetStdDevTmax[12];



};

struct TdataAnalysis{
    double** daysWithMoreThan50mm;
    double** daysWithMoreThan40mm;
    double** daysWithMoreThan30mm;
    double** daysWithMoreThan20mm;
    double** daysWithMoreThan10mm;
    double** daysWithMoreThan05mm;
    double** daysWithMoreThan01mm;
    double** daysWithLessThan01mm;
    double** cumulatedET0;
    double** cumulatedBIC;
    double*** cumulatedET0Year;
    double*** cumulatedBICYear;
    double** cumulatedStdDevET0;
    double** cumulatedStdDevBIC;
};

void readFileContents(FILE *fp,int site,TmonthlyData* monthlyData);
void computeBias(TmonthlyData *observed, TmonthlyData *simulated, TmonthlyData* bias, TmonthlyData *monthlyMax, TmonthlyData *monthlyMin, TmonthlyData *monthlyAverage, int nrSites);
int readERG5CellListNumber(FILE *fp);
void readTheCellNumber(FILE *fp, char* numCell);
void logInfo(QString myStr);
bool loadMeteoGridDB(Crit3DMeteoGridDbHandler *myHandler, QString* errorString, QString xmlName);

static weatherGenerator2D WG2D;

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


bool loadMeteoGridDB(Crit3DMeteoGridDbHandler* myHandler, QString* errorString, QString xmlName)
{
    //QString xmlName = QFileDialog::getOpenFileName(nullptr, "Open XML grid", "", "XML files (*.xml)");

    QString path;
    if (! searchDataPath(&path)) return -1;
    //QString xmlName = path + "METEOGRID/DBGridXML_Eraclito4.xml";
    //QString xmlName = path + "METEOGRID/DBGridXML_ERG5_v2.1.xml";
    xmlName = path + xmlName;

    // todo
    //meteoGridDbHandler->meteoGrid()->setGisSettings(this->gisSettings);

    if (! myHandler->parseXMLGrid(xmlName, errorString)) return false;

    if (! myHandler->openDatabase(errorString))return false;

    if (! myHandler->loadCellProperties(errorString)) return false;

    if (! myHandler->updateGridDate(errorString)) return false;

    logInfo("Meteo Grid = " + xmlName);

    return true;
}


bool saveOnMeteoGridDB(Crit3DMeteoGridDbHandler* myHandler, QString* errorString)
{
    //QString xmlName = QFileDialog::getOpenFileName(nullptr, "Open XML grid", "", "XML files (*.xml)");
    QString path;
    if (! searchDataPath(&path)) return -1;
    QString xmlName = path + "METEOGRID/DBGridXML_Output_WG2D.xml";

    // todo
    //meteoGridDbHandler->meteoGrid()->setGisSettings(this->gisSettings);

    if (! myHandler->parseXMLGrid(xmlName, errorString)) return false;

    if (! myHandler->openDatabase(errorString))return false;

    if (! myHandler->loadCellProperties(errorString)) return false;

    //if (! meteoGridDbHandlerWG2D->updateGridDate(errorString)) return false;

    logInfo("Meteo Grid = " + xmlName);

    return true;
}

int main(int argc, char *argv[])
{


    Crit3DMeteoGridDbHandler* meteoGridDbHandler = new Crit3DMeteoGridDbHandler();
    Crit3DMeteoGridDbHandler* meteoGridDbHandlerWG2D = new Crit3DMeteoGridDbHandler();

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
    QString xmlName;
    xmlName = "METEOGRID/DBGridXML_eraclito4.xml";
    //xmlName = "METEOGRID/DBGridXML_ERG5_v2.1.xml";
    //xmlName = "METEOGRID/DBGridXML_Output_WG2D.xml";

    QString errorString;
    if (! loadMeteoGridDB(meteoGridDbHandler, &errorString,xmlName))
    {
        std::cout << errorString.toStdString() << std::endl;
        return -1;
    }
    std::string id;
    int nrActivePoints = 0;
    int lengthSeries = 0;
    std::vector<float> dailyVariable;
    FILE* fp;
    //fp = fopen("../test_WG2D_Eraclito/inputData/list_enza_secchia_panaro_30_sites.txt","r"); // !! take out
    fp = fopen("../test_WG2D_Eraclito/inputData/list_C7_shortlisted_few_sites.txt","r"); // !! take out
    int numberOfCells; // !! take out
    numberOfCells = readERG5CellListNumber(fp); // !! take out
    fclose(fp); // !! take out

    TdataAnalysis obsDataAnalysis;
    TdataAnalysis simDataAnalysis;
    obsDataAnalysis.daysWithLessThan01mm =  (double **) calloc(numberOfCells, sizeof(double*));
    obsDataAnalysis.daysWithMoreThan01mm =  (double **) calloc(numberOfCells, sizeof(double*));
    obsDataAnalysis.daysWithMoreThan05mm =  (double **) calloc(numberOfCells, sizeof(double*));
    obsDataAnalysis.daysWithMoreThan10mm =  (double **) calloc(numberOfCells, sizeof(double*));
    obsDataAnalysis.daysWithMoreThan20mm =  (double **) calloc(numberOfCells, sizeof(double*));
    obsDataAnalysis.daysWithMoreThan30mm =  (double **) calloc(numberOfCells, sizeof(double*));
    obsDataAnalysis.daysWithMoreThan40mm =  (double **) calloc(numberOfCells, sizeof(double*));
    obsDataAnalysis.daysWithMoreThan50mm =  (double **) calloc(numberOfCells, sizeof(double*));
    obsDataAnalysis.cumulatedBIC = (double **) calloc(numberOfCells, sizeof(double*));
    obsDataAnalysis.cumulatedET0 = (double **) calloc(numberOfCells, sizeof(double*));
    obsDataAnalysis.cumulatedStdDevBIC = (double **) calloc(numberOfCells, sizeof(double*));
    obsDataAnalysis.cumulatedStdDevET0 = (double **) calloc(numberOfCells, sizeof(double*));
    obsDataAnalysis.cumulatedBICYear = (double ***) calloc(numberOfCells, sizeof(double**));
    obsDataAnalysis.cumulatedET0Year = (double ***) calloc(numberOfCells, sizeof(double**));
    simDataAnalysis.daysWithLessThan01mm =  (double **) calloc(numberOfCells, sizeof(double*));
    simDataAnalysis.daysWithMoreThan01mm =  (double **) calloc(numberOfCells, sizeof(double*));
    simDataAnalysis.daysWithMoreThan05mm =  (double **) calloc(numberOfCells, sizeof(double*));
    simDataAnalysis.daysWithMoreThan10mm =  (double **) calloc(numberOfCells, sizeof(double*));
    simDataAnalysis.daysWithMoreThan20mm =  (double **) calloc(numberOfCells, sizeof(double*));
    simDataAnalysis.daysWithMoreThan30mm =  (double **) calloc(numberOfCells, sizeof(double*));
    simDataAnalysis.daysWithMoreThan40mm =  (double **) calloc(numberOfCells, sizeof(double*));
    simDataAnalysis.daysWithMoreThan50mm =  (double **) calloc(numberOfCells, sizeof(double*));
    simDataAnalysis.cumulatedBIC = (double **) calloc(numberOfCells, sizeof(double*));
    simDataAnalysis.cumulatedET0 = (double **) calloc(numberOfCells, sizeof(double*));
    simDataAnalysis.cumulatedStdDevBIC = (double **) calloc(numberOfCells, sizeof(double*));
    simDataAnalysis.cumulatedStdDevET0 = (double **) calloc(numberOfCells, sizeof(double*));
    simDataAnalysis.cumulatedBICYear = (double ***) calloc(numberOfCells, sizeof(double**));
    simDataAnalysis.cumulatedET0Year = (double ***) calloc(numberOfCells, sizeof(double**));

    for (int i=0;i<numberOfCells;i++)
    {
        obsDataAnalysis.daysWithLessThan01mm[i] =  (double *) calloc(12, sizeof(double));
        obsDataAnalysis.daysWithMoreThan01mm[i] =  (double *) calloc(12, sizeof(double));
        obsDataAnalysis.daysWithMoreThan05mm[i] =  (double *) calloc(12, sizeof(double));
        obsDataAnalysis.daysWithMoreThan10mm[i] =  (double *) calloc(12, sizeof(double));
        obsDataAnalysis.daysWithMoreThan20mm[i] =  (double *) calloc(12, sizeof(double));
        obsDataAnalysis.daysWithMoreThan30mm[i] =  (double *) calloc(12, sizeof(double));
        obsDataAnalysis.daysWithMoreThan40mm[i] =  (double *) calloc(12, sizeof(double));
        obsDataAnalysis.daysWithMoreThan50mm[i] =  (double *) calloc(12, sizeof(double));
        obsDataAnalysis.cumulatedBIC[i] =  (double *) calloc(12, sizeof(double));
        obsDataAnalysis.cumulatedET0[i] =  (double *) calloc(12, sizeof(double));
        obsDataAnalysis.cumulatedStdDevBIC[i] =  (double *) calloc(12, sizeof(double));
        obsDataAnalysis.cumulatedStdDevET0[i] =  (double *) calloc(12, sizeof(double));
        obsDataAnalysis.cumulatedBICYear[i] =  (double **) calloc(12, sizeof(double*));
        obsDataAnalysis.cumulatedET0Year[i] =  (double **) calloc(12, sizeof(double*));
        simDataAnalysis.daysWithLessThan01mm[i] =  (double *) calloc(12, sizeof(double));
        simDataAnalysis.daysWithMoreThan01mm[i] =  (double *) calloc(12, sizeof(double));
        simDataAnalysis.daysWithMoreThan05mm[i] =  (double *) calloc(12, sizeof(double));
        simDataAnalysis.daysWithMoreThan10mm[i] =  (double *) calloc(12, sizeof(double));
        simDataAnalysis.daysWithMoreThan20mm[i] =  (double *) calloc(12, sizeof(double));
        simDataAnalysis.daysWithMoreThan30mm[i] =  (double *) calloc(12, sizeof(double));
        simDataAnalysis.daysWithMoreThan40mm[i] =  (double *) calloc(12, sizeof(double));
        simDataAnalysis.daysWithMoreThan50mm[i] =  (double *) calloc(12, sizeof(double));
        simDataAnalysis.cumulatedBIC[i] =  (double *) calloc(12, sizeof(double));
        simDataAnalysis.cumulatedET0[i] =  (double *) calloc(12, sizeof(double));
        simDataAnalysis.cumulatedStdDevBIC[i] =  (double *) calloc(12, sizeof(double));
        simDataAnalysis.cumulatedStdDevET0[i] =  (double *) calloc(12, sizeof(double));
        simDataAnalysis.cumulatedBICYear[i] =  (double **) calloc(12, sizeof(double*));
        simDataAnalysis.cumulatedET0Year[i] =  (double **) calloc(12, sizeof(double*));

        for (int j=0;j<12;j++)
        {
            obsDataAnalysis.daysWithLessThan01mm[i][j] =  0;
            obsDataAnalysis.daysWithMoreThan01mm[i][j] =  0;
            obsDataAnalysis.daysWithMoreThan05mm[i][j] =  0;
            obsDataAnalysis.daysWithMoreThan10mm[i][j] =  0;
            obsDataAnalysis.daysWithMoreThan20mm[i][j] =  0;
            obsDataAnalysis.daysWithMoreThan30mm[i][j] =  0;
            obsDataAnalysis.daysWithMoreThan40mm[i][j] =  0;
            obsDataAnalysis.daysWithMoreThan50mm[i][j] =  0;
            obsDataAnalysis.cumulatedBIC[i][j] = 0;
            obsDataAnalysis.cumulatedET0[i][j] = 0;
            obsDataAnalysis.cumulatedStdDevBIC[i][j] = 0;
            obsDataAnalysis.cumulatedStdDevET0[i][j] = 0;
            simDataAnalysis.daysWithLessThan01mm[i][j] =  0;
            simDataAnalysis.daysWithMoreThan01mm[i][j] =  0;
            simDataAnalysis.daysWithMoreThan05mm[i][j] =  0;
            simDataAnalysis.daysWithMoreThan10mm[i][j] =  0;
            simDataAnalysis.daysWithMoreThan20mm[i][j] =  0;
            simDataAnalysis.daysWithMoreThan30mm[i][j] =  0;
            simDataAnalysis.daysWithMoreThan40mm[i][j] =  0;
            simDataAnalysis.daysWithMoreThan50mm[i][j] =  0;
            simDataAnalysis.cumulatedBIC[i][j] = 0;
            simDataAnalysis.cumulatedET0[i][j] = 0;
            simDataAnalysis.cumulatedStdDevBIC[i][j] = 0;
            simDataAnalysis.cumulatedStdDevET0[i][j] = 0;
        }
    }


    //fp = fopen("../test_WG2D_Eraclito/inputData/list_enza_secchia_panaro_30_sites.txt","r"); // !! take out
    fp = fopen("../test_WG2D_Eraclito/inputData/list_C7_shortlisted_few_sites.txt","r"); // !! take out

    int* cellCode = nullptr; // !! take out
    char* numCell = (char *)calloc(6, sizeof(char)); // !! take out
    cellCode = (int *) calloc(numberOfCells, sizeof(int)); // !! take out
    for (int i=0; i<numberOfCells; i++) // !! take out
    { // !! take out
        readTheCellNumber(fp,numCell);// !! take out
        cellCode[i] = atoi(numCell);// !! take out
    }// !! take out
    fclose(fp);
    printf("numCells %d\n",numberOfCells);
    for (int i=0;i<numberOfCells;i++)
    {
        for (int j=0;j<12;j++)
        {
            obsDataAnalysis.cumulatedBICYear[i][j] =  (double *) calloc(1 + lastDay.year() - firstDay.year(), sizeof(double));
            obsDataAnalysis.cumulatedET0Year[i][j] =  (double *) calloc(1 + lastDay.year() - firstDay.year(), sizeof(double));
            for (int k=0; k<1 + lastDay.year() - firstDay.year();k++)
            {
                obsDataAnalysis.cumulatedBICYear[i][j][k] = 0;
                obsDataAnalysis.cumulatedET0Year[i][j][k] = 0;
            }
        }
    }

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

    for (int i=0;i<nrActivePoints;i++)
    {
        int validDays[12] = {0};
        currentDay = firstDay;
        for (int j=0;j<lengthSeries;j++)
        {
            if (obsDataD[i][j].prec >= 50)
            {
                ++obsDataAnalysis.daysWithMoreThan50mm[i][currentDay.month()-1];
            }
            if (obsDataD[i][j].prec >= 40 && obsDataD[i][j].prec < 50)
            {
                ++obsDataAnalysis.daysWithMoreThan40mm[i][currentDay.month()-1];
            }
            if (obsDataD[i][j].prec >= 30 && obsDataD[i][j].prec < 40)
            {
                ++obsDataAnalysis.daysWithMoreThan30mm[i][currentDay.month()-1];
            }
            if (obsDataD[i][j].prec >= 20 && obsDataD[i][j].prec < 30)
            {
                ++obsDataAnalysis.daysWithMoreThan20mm[i][currentDay.month()-1];
            }
            if (obsDataD[i][j].prec >= 10 && obsDataD[i][j].prec < 20)
            {
                ++obsDataAnalysis.daysWithMoreThan10mm[i][currentDay.month()-1];
            }
            if (obsDataD[i][j].prec >= 5 && obsDataD[i][j].prec < 10)
            {
                ++obsDataAnalysis.daysWithMoreThan05mm[i][currentDay.month()-1];
            }
            if (obsDataD[i][j].prec >= 1 && obsDataD[i][j].prec < 5)
            {
                ++obsDataAnalysis.daysWithMoreThan01mm[i][currentDay.month()-1];
            }
            if (obsDataD[i][j].prec >= 0.25 && obsDataD[i][j].prec < 1)
            {
                ++obsDataAnalysis.daysWithLessThan01mm[i][currentDay.month()-1];
            }

            double dailyET0;
            int yearObsData=0;
            for (int iMonth=0; iMonth<12;iMonth++)
                validDays[iMonth] = 0;
            if (obsDataD[i][j].prec != NODATA && obsDataD[i][j].tMin != NODATA && obsDataD[i][j].tMax != NODATA)
            {
                ++validDays[currentDay.month()-1];
                dailyET0 = ET0_Hargreaves(DEFAULT_TRANSMISSIVITY_SAMANI,44.5,currentDay.dayOfYear(),obsDataD[i][j].tMax,obsDataD[i][j].tMin);
                obsDataAnalysis.cumulatedET0[i][currentDay.month()-1] += dailyET0;
                obsDataAnalysis.cumulatedBIC[i][currentDay.month()-1] += obsDataD[i][j].prec - dailyET0;
                yearObsData = currentDay.year() - firstDay.year();
                obsDataAnalysis.cumulatedET0Year[i][currentDay.month()-1][yearObsData] += dailyET0;
                obsDataAnalysis.cumulatedBICYear[i][currentDay.month()-1][yearObsData] += obsDataD[i][j].prec - dailyET0;

                //dailyET0 = ET0_Hargreaves(DEFAULT_TRANSMISSIVITY_SAMANI,44.5,currentDay.dayOfYear(),obsDataD[i][j].tMax,obsDataD[i][j].tMin);
                /*obsDataAnalysis.cumulatedET0[i][currentDay.month()-1] += (obsDataD[i][j].tMax - obsDataD[i][j].tMin);
                obsDataAnalysis.cumulatedBIC[i][currentDay.month()-1] += obsDataD[i][j].tMin;
                yearObsData = currentDay.year() - firstDay.year();
                obsDataAnalysis.cumulatedET0Year[i][currentDay.month()-1][yearObsData] += (obsDataD[i][j].tMax - obsDataD[i][j].tMin);
                obsDataAnalysis.cumulatedBICYear[i][currentDay.month()-1][yearObsData] += obsDataD[i][j].tMin;
                */
            }
            currentDay = currentDay.addDays(1);
        }
        for (int jMonth=0;jMonth<12;jMonth++)
        {

            int denominator = 1 + lastDay.year() - firstDay.year();
            obsDataAnalysis.daysWithMoreThan50mm[i][jMonth] /= denominator;
            obsDataAnalysis.daysWithMoreThan40mm[i][jMonth] /= denominator;
            obsDataAnalysis.daysWithMoreThan30mm[i][jMonth] /= denominator;
            obsDataAnalysis.daysWithMoreThan20mm[i][jMonth] /= denominator;
            obsDataAnalysis.daysWithMoreThan10mm[i][jMonth] /= denominator;
            obsDataAnalysis.daysWithMoreThan05mm[i][jMonth] /= denominator;
            obsDataAnalysis.daysWithMoreThan01mm[i][jMonth] /= denominator;
            obsDataAnalysis.daysWithLessThan01mm[i][jMonth] /= denominator;            
            //printf("active point %d,month %d\t,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f\n press enter to continue\n",i,jMonth+1,obsDataAnalysis.daysWithMoreThan50mm[i][jMonth],obsDataAnalysis.daysWithMoreThan40mm[i][jMonth],obsDataAnalysis.daysWithMoreThan30mm[i][jMonth],obsDataAnalysis.daysWithMoreThan20mm[i][jMonth],obsDataAnalysis.daysWithMoreThan10mm[i][jMonth],obsDataAnalysis.daysWithMoreThan05mm[i][jMonth],obsDataAnalysis.daysWithMoreThan01mm[i][jMonth],obsDataAnalysis.daysWithLessThan01mm[i][jMonth]);
            obsDataAnalysis.cumulatedET0[i][jMonth] /= denominator;
            obsDataAnalysis.cumulatedBIC[i][jMonth] /= denominator;
            obsDataAnalysis.cumulatedStdDevET0[i][jMonth] =statistics::standardDeviation(obsDataAnalysis.cumulatedET0Year[i][jMonth],denominator);
            obsDataAnalysis.cumulatedStdDevBIC[i][jMonth] =statistics::standardDeviation(obsDataAnalysis.cumulatedBICYear[i][jMonth],denominator);
            //obsDataAnalysis.cumulatedStdDevET0[i][jMonth] =

        }
        /*for (int jMonth=0;jMonth<12;jMonth++)
        {
            double mean=0;
            double stdDev=0;
            int validRecord = 0;
            for (int j=0;j<lengthSeries;j++)
            {

                if (obsDataD[i][j].date.month == jMonth+1)
                {
                    if (obsDataD[i][j].prec != NODATA && obsDataD[i][j].tMin != NODATA && obsDataD[i][j].tMax != NODATA)
                    {
                        validRecord++;
                        mean += obsDataD[i][j].tMax - obsDataD[i][j].tMin;
                    }

                }
            }
            mean /= validRecord;
            for (int j=0;j<lengthSeries;j++)
            {

                if (obsDataD[i][j].date.month == jMonth+1)
                {
                    if (obsDataD[i][j].prec != NODATA && obsDataD[i][j].tMin != NODATA && obsDataD[i][j].tMax != NODATA)
                    {
                        //validRecord++;
                        stdDev += (obsDataD[i][j].tMax - obsDataD[i][j].tMin - mean)*(obsDataD[i][j].tMax - obsDataD[i][j].tMin - mean);
                    }

                }
            }
            stdDev = pow(stdDev,0.5)/(validRecord-1);
            obsDataAnalysis.cumulatedStdDevET0[i][jMonth] = stdDev;
        }*/
    }

    dailyVariable.clear();
    meteoGridDbHandler->closeDatabase();

// read second DB
    printf("second DB\n");
    QDate firstDayWG2D(2001,1,1);
    QDate lastDayWG2D(2200,12,31);

    TObsDataD** outputDataD = nullptr;
    xmlName = "METEOGRID/DBGridXML_Output_WG2D.xml";

    if (! loadMeteoGridDB(meteoGridDbHandlerWG2D, &errorString,xmlName))
    {
        std::cout << errorString.toStdString() << std::endl;
        return -1;
    }
    printf("second DB loading\n");

    //std::string id;
    nrActivePoints = 0;
    lengthSeries = 0;
    std::vector<float> dailyVariableWG2D;
    //FILE* fp;
    //fp = fopen("../test_WG2D_Eraclito/inputData/list_enza_secchia_panaro_30_sites.txt","r"); // !! take out
    fp = fopen("../test_WG2D_Eraclito/inputData/list_C7_shortlisted_few_sites.txt","r"); // !! take out
    //int numberOfCells; // !! take out
    numberOfCells = readERG5CellListNumber(fp); // !! take out
    fclose(fp); // !! take out
    for (int i=0;i<numberOfCells;i++)
    {
        for (int j=0;j<12;j++)
        {
            simDataAnalysis.cumulatedBICYear[i][j] =  (double *) calloc(1 + lastDayWG2D.year() - firstDayWG2D.year(), sizeof(double));
            simDataAnalysis.cumulatedET0Year[i][j] =  (double *) calloc(1 + lastDayWG2D.year() - firstDayWG2D.year(), sizeof(double));
            for (int k=0; k<1 + lastDayWG2D.year() - firstDayWG2D.year();k++)
            {
                simDataAnalysis.cumulatedBICYear[i][j][k] = 0;
                simDataAnalysis.cumulatedET0Year[i][j][k] = 0;
            }
        }
    }

    //fp = fopen("../test_WG2D_Eraclito/inputData/list_enza_secchia_panaro_30_sites.txt","r"); // !! take out
    fp = fopen("../test_WG2D_Eraclito/inputData/list_C7_shortlisted_few_sites.txt","r"); // !! take out

    //int* cellCode = nullptr; // !! take out
    //char* numCell = (char *)calloc(6, sizeof(char)); // !! take out
    //cellCode = (int *) calloc(numberOfCells, sizeof(int)); // !! take out
    for (int i=0; i<numberOfCells; i++) // !! take out
    { // !! take out
        readTheCellNumber(fp,numCell);// !! take out
        cellCode[i] = atoi(numCell);// !! take out
    }// !! take out
    fclose(fp);
    printf("numCells %d\n",numberOfCells);

    for (int row = 0; row < meteoGridDbHandlerWG2D->gridStructure().header().nrRows; row++)
    {

        for (int col = 0; col < meteoGridDbHandlerWG2D->gridStructure().header().nrCols; col++)
        {

           if (meteoGridDbHandlerWG2D->meteoGrid()->getMeteoPointActiveId(row, col, &id))
           {
               ++nrActivePoints;
               int idCellInt;
               idCellInt = atoi(id.c_str());
               if (idCellInt == cellCode[0])
               {
                   variable = dailyAirTemperatureMin;
                   dailyVariableWG2D = meteoGridDbHandlerWG2D->loadGridDailyVar(&myError, QString::fromStdString(id),
                                                                        variable, firstDayWG2D, lastDayWG2D, &firstDateDB);
                   lengthSeries = int(dailyVariableWG2D.size());
               }
           }
        }
    }

    #ifdef MAX_NR_POINTS
        nrActivePoints = MINVALUE(nrActivePoints, MAX_NR_POINTS);
    #endif
    if (nrActivePoints > numberOfCells) nrActivePoints = numberOfCells; // !! take out
    printf("%d  %d\n", lengthSeries,nrActivePoints);
    outputDataD = (TObsDataD **)calloc(nrActivePoints, sizeof(TObsDataD*));
    for (int i=0;i<nrActivePoints;i++)
    {
        outputDataD[i] = (TObsDataD *)calloc(lengthSeries, sizeof(TObsDataD));
    }

    for (int i=0;i<nrActivePoints;i++)
    {
        currentDay = firstDayWG2D;
        for (int j=0;j<lengthSeries;j++)
        {
            outputDataD[i][j].date.day = currentDay.day();
            outputDataD[i][j].date.month = currentDay.month();
            outputDataD[i][j].date.year = currentDay.year();
            currentDay = currentDay.addDays(1);
            //printf("month %d,%d,%d\n",outputDataD[i][j].date.day,outputDataD[i][j].date.month,outputDataD[i][j].date.year);
        }
        //getchar();
    }

    printf("load data...\n");

    counter = 0;
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

           if (meteoGridDbHandlerWG2D->meteoGrid()->getMeteoPointActiveId(row, col, &id) && counter<nrActivePoints && isConsortiumCell) // !! to modify
           {

               printf(" %d %d \n",row,col);
               variable = dailyAirTemperatureMin;
               dailyVariableWG2D = meteoGridDbHandlerWG2D->loadGridDailyVar(&myError, QString::fromStdString(id),
                                                                    variable, firstDayWG2D, lastDayWG2D, &firstDateDB);
               for (int iLength=0; iLength<lengthSeries; iLength++) outputDataD[counter][iLength].tMin = dailyVariableWG2D[iLength];
               //for (int iLength=0; iLength<lengthSeries; iLength++) printf("temp %f\n",dailyVariable[iLength]);
               variable = dailyAirTemperatureMax;
               dailyVariableWG2D = meteoGridDbHandlerWG2D->loadGridDailyVar(&myError, QString::fromStdString(id),
                                                                    variable, firstDayWG2D, lastDayWG2D, &firstDateDB);
               for (int iLength=0; iLength<lengthSeries; iLength++) outputDataD[counter][iLength].tMax = dailyVariableWG2D[iLength];
               variable = dailyPrecipitation;
               dailyVariableWG2D = meteoGridDbHandlerWG2D->loadGridDailyVar(&myError, QString::fromStdString(id),
                                                                    variable, firstDayWG2D, lastDayWG2D, &firstDateDB);
               for (int iLength=0; iLength<lengthSeries; iLength++) outputDataD[counter][iLength].prec = dailyVariableWG2D[iLength];
               counter++;
           }
        }
        //std::cout << row << "\n";
    }

    for (int i=0;i<nrActivePoints;i++)
    {
        int validDays[12] = {0};
        currentDay = firstDayWG2D;
        for (int j=0;j<lengthSeries;j++)
        {
            int month;
            month = outputDataD[i][j].date.month - 1;
            //printf("mese %d\n",month);
            if (outputDataD[i][j].prec >= 50)
            {
                ++simDataAnalysis.daysWithMoreThan50mm[i][month];
            }

            else if (outputDataD[i][j].prec >= 40 && outputDataD[i][j].prec < 50)
            {
                ++simDataAnalysis.daysWithMoreThan40mm[i][month];
            }

            else if (outputDataD[i][j].prec >= 30 && outputDataD[i][j].prec < 40)
            {
                ++simDataAnalysis.daysWithMoreThan30mm[i][month];
            }

            else if (outputDataD[i][j].prec >= 20 && outputDataD[i][j].prec < 30)
            {
                ++simDataAnalysis.daysWithMoreThan20mm[i][month];
            }
            else if (outputDataD[i][j].prec >= 10 && outputDataD[i][j].prec < 20)
            {
                ++simDataAnalysis.daysWithMoreThan10mm[i][month];
            }
            else if (outputDataD[i][j].prec >= 5 && outputDataD[i][j].prec < 10)
            {
                ++simDataAnalysis.daysWithMoreThan05mm[i][month];
            }
            else if (outputDataD[i][j].prec >= 1 && outputDataD[i][j].prec < 5)
            {
                ++simDataAnalysis.daysWithMoreThan01mm[i][month];
            }
            else if (outputDataD[i][j].prec >= 0.25 && outputDataD[i][j].prec < 1)
            {
                ++simDataAnalysis.daysWithLessThan01mm[i][month];
            }
            double dailyET0;
            for (int iMonth=0;iMonth<12;iMonth++)
                validDays[iMonth] = 0;
            if (outputDataD[i][j].prec != NODATA && outputDataD[i][j].tMin != NODATA && outputDataD[i][j].tMax != NODATA)
            {
                ++validDays[currentDay.month()-1];

                dailyET0 = ET0_Hargreaves(DEFAULT_TRANSMISSIVITY_SAMANI,44.5,currentDay.dayOfYear(),outputDataD[i][j].tMax,outputDataD[i][j].tMin);
                simDataAnalysis.cumulatedET0[i][currentDay.month()-1] += dailyET0;
                simDataAnalysis.cumulatedBIC[i][currentDay.month()-1] += outputDataD[i][j].prec - dailyET0;
                simDataAnalysis.cumulatedET0Year[i][currentDay.month()-1][currentDay.year()-firstDayWG2D.year()] += dailyET0;
                simDataAnalysis.cumulatedBICYear[i][currentDay.month()-1][currentDay.year()-firstDayWG2D.year()] += outputDataD[i][j].prec - dailyET0;

                //dailyET0 = ET0_Hargreaves(DEFAULT_TRANSMISSIVITY_SAMANI,44.5,currentDay.dayOfYear(),outputDataD[i][j].tMax,outputDataD[i][j].tMin);
                /*simDataAnalysis.cumulatedET0[i][currentDay.month()-1] += (outputDataD[i][j].tMax-outputDataD[i][j].tMin);
                simDataAnalysis.cumulatedBIC[i][currentDay.month()-1] += outputDataD[i][j].tMin;
                simDataAnalysis.cumulatedET0Year[i][currentDay.month()-1][currentDay.year()-firstDayWG2D.year()] += (outputDataD[i][j].tMax-outputDataD[i][j].tMin);
                simDataAnalysis.cumulatedBICYear[i][currentDay.month()-1][currentDay.year()-firstDayWG2D.year()] += outputDataD[i][j].tMin;
                */
            }
            currentDay = currentDay.addDays(1);
        }

        for (int jMonth=0;jMonth<12;jMonth++)
        {

            int denominator = 1 + lastDayWG2D.year() - firstDayWG2D.year();
            simDataAnalysis.daysWithMoreThan50mm[i][jMonth] /= denominator;
            simDataAnalysis.daysWithMoreThan40mm[i][jMonth] /= denominator;
            simDataAnalysis.daysWithMoreThan30mm[i][jMonth] /= denominator;
            simDataAnalysis.daysWithMoreThan20mm[i][jMonth] /= denominator;
            simDataAnalysis.daysWithMoreThan10mm[i][jMonth] /= denominator;
            simDataAnalysis.daysWithMoreThan05mm[i][jMonth] /= denominator;
            simDataAnalysis.daysWithMoreThan01mm[i][jMonth] /= denominator;
            simDataAnalysis.daysWithLessThan01mm[i][jMonth] /= denominator;
            //printf("active point %d\t,month %d,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f\n press enter to continue\n",i,jMonth+1,simDataAnalysis.daysWithMoreThan50mm[i][jMonth],simDataAnalysis.daysWithMoreThan40mm[i][jMonth],simDataAnalysis.daysWithMoreThan30mm[i][jMonth],simDataAnalysis.daysWithMoreThan20mm[i][jMonth],simDataAnalysis.daysWithMoreThan10mm[i][jMonth],obsDataAnalysis.daysWithMoreThan05mm[i][jMonth],simDataAnalysis.daysWithMoreThan01mm[i][jMonth],simDataAnalysis.daysWithLessThan01mm[i][jMonth]);
            simDataAnalysis.cumulatedET0[i][jMonth] /= denominator;
            simDataAnalysis.cumulatedBIC[i][jMonth] /= denominator;
            simDataAnalysis.cumulatedStdDevET0[i][jMonth] = statistics::standardDeviation(simDataAnalysis.cumulatedET0Year[i][jMonth],denominator);
            simDataAnalysis.cumulatedStdDevBIC[i][jMonth] = statistics::standardDeviation(simDataAnalysis.cumulatedBICYear[i][jMonth],denominator);
        }
        /*
        for (int jMonth=0;jMonth<12;jMonth++)
        {
            double mean=0;
            double stdDev=0;
            int validRecord = 0;
            for (int j=0;j<lengthSeries;j++)
            {

                if (outputDataD[i][j].date.month == jMonth+1)
                {
                    if (outputDataD[i][j].prec != NODATA && outputDataD[i][j].tMin != NODATA && outputDataD[i][j].tMax != NODATA)
                    {
                        validRecord++;
                        mean += outputDataD[i][j].tMax - outputDataD[i][j].tMin;
                    }

                }
            }
            mean /= validRecord;
            for (int j=0;j<lengthSeries;j++)
            {

                if (outputDataD[i][j].date.month == jMonth+1)
                {
                    if (outputDataD[i][j].prec != NODATA && outputDataD[i][j].tMin != NODATA && outputDataD[i][j].tMax != NODATA)
                    {
                        //validRecord++;
                        stdDev += (outputDataD[i][j].tMax - outputDataD[i][j].tMin - mean)*(outputDataD[i][j].tMax - outputDataD[i][j].tMin - mean);
                    }

                }
            }
            stdDev = pow(stdDev,0.5)/(validRecord-1);
            simDataAnalysis.cumulatedStdDevET0[i][jMonth] = stdDev;
        }*/


    }
    fp = fopen("extremes_precipitation_analysis.txt","w");
    fprintf(fp,">50mm,>40mm,>30mm,>20mm,>10mm,>5mm,>1mm,<1mm\n");
    for (int i=0;i<nrActivePoints;i++)
    {
        fprintf(fp,"site %d\n",i);
        for (int jMonth=0;jMonth<12;jMonth++)
        {
            fprintf(fp,"month %d\n",jMonth+1);
            fprintf(fp,"%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f\n",obsDataAnalysis.daysWithMoreThan50mm[i][jMonth],obsDataAnalysis.daysWithMoreThan40mm[i][jMonth],obsDataAnalysis.daysWithMoreThan30mm[i][jMonth],obsDataAnalysis.daysWithMoreThan20mm[i][jMonth],obsDataAnalysis.daysWithMoreThan10mm[i][jMonth],obsDataAnalysis.daysWithMoreThan05mm[i][jMonth],obsDataAnalysis.daysWithMoreThan01mm[i][jMonth],obsDataAnalysis.daysWithLessThan01mm[i][jMonth]);
            fprintf(fp,"%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f\n",simDataAnalysis.daysWithMoreThan50mm[i][jMonth],simDataAnalysis.daysWithMoreThan40mm[i][jMonth],simDataAnalysis.daysWithMoreThan30mm[i][jMonth],simDataAnalysis.daysWithMoreThan20mm[i][jMonth],simDataAnalysis.daysWithMoreThan10mm[i][jMonth],simDataAnalysis.daysWithMoreThan05mm[i][jMonth],simDataAnalysis.daysWithMoreThan01mm[i][jMonth],simDataAnalysis.daysWithLessThan01mm[i][jMonth]);
            fprintf(fp,"%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f\n",simDataAnalysis.daysWithMoreThan50mm[i][jMonth]-obsDataAnalysis.daysWithMoreThan50mm[i][jMonth],simDataAnalysis.daysWithMoreThan40mm[i][jMonth]-obsDataAnalysis.daysWithMoreThan40mm[i][jMonth],simDataAnalysis.daysWithMoreThan30mm[i][jMonth]-obsDataAnalysis.daysWithMoreThan30mm[i][jMonth],simDataAnalysis.daysWithMoreThan20mm[i][jMonth]-obsDataAnalysis.daysWithMoreThan20mm[i][jMonth],simDataAnalysis.daysWithMoreThan10mm[i][jMonth]-obsDataAnalysis.daysWithMoreThan10mm[i][jMonth],simDataAnalysis.daysWithMoreThan05mm[i][jMonth]-obsDataAnalysis.daysWithMoreThan05mm[i][jMonth],simDataAnalysis.daysWithMoreThan01mm[i][jMonth]-obsDataAnalysis.daysWithMoreThan01mm[i][jMonth],simDataAnalysis.daysWithLessThan01mm[i][jMonth] - obsDataAnalysis.daysWithLessThan01mm[i][jMonth]);
        }
    }
    fclose(fp);
    fp = fopen("confrontoETO_BIC.txt","w");
    fprintf(fp,"ETO,BIC\n");
    for (int i=0;i<nrActivePoints;i++)
    //for (int i=0;i<1;i++)
    {
        fprintf(fp,"site %d\n",i);
        for (int jMonth=0;jMonth<12;jMonth++)
        {
            fprintf(fp,"month %d\n",jMonth+1);
            fprintf(fp,"average,obs, %f,%f\n",obsDataAnalysis.cumulatedET0[i][jMonth],obsDataAnalysis.cumulatedBIC[i][jMonth]);
            fprintf(fp,"average,sim, %f,%f\n",simDataAnalysis.cumulatedET0[i][jMonth],simDataAnalysis.cumulatedBIC[i][jMonth]);
            fprintf(fp,"average,sim-obs, %f,%.1f\n",simDataAnalysis.cumulatedET0[i][jMonth]-obsDataAnalysis.cumulatedET0[i][jMonth],simDataAnalysis.cumulatedBIC[i][jMonth]-obsDataAnalysis.cumulatedBIC[i][jMonth]);
            fprintf(fp,"stdDev,obs, %f,%f\n",obsDataAnalysis.cumulatedStdDevET0[i][jMonth],obsDataAnalysis.cumulatedStdDevBIC[i][jMonth]);
            fprintf(fp,"stdDev,sim, %f,%f\n",simDataAnalysis.cumulatedStdDevET0[i][jMonth],simDataAnalysis.cumulatedStdDevBIC[i][jMonth]);
            fprintf(fp,"stdDev,sim-obs, %f,%f\n",simDataAnalysis.cumulatedStdDevET0[i][jMonth]-obsDataAnalysis.cumulatedStdDevET0[i][jMonth],simDataAnalysis.cumulatedStdDevBIC[i][jMonth]-obsDataAnalysis.cumulatedStdDevBIC[i][jMonth]);
            fprintf(fp,"distributionET0,obs");
            for (int kYear=0;kYear<1+lastDay.year()-firstDay.year();kYear++)
                fprintf(fp,",%.1f",obsDataAnalysis.cumulatedET0Year[i][jMonth][kYear]);
            fprintf(fp,"\n");
            fprintf(fp,"distributionBIC,obs");
            for (int kYear=0;kYear<1+lastDay.year()-firstDay.year();kYear++)
                fprintf(fp,",%.1f",obsDataAnalysis.cumulatedBICYear[i][jMonth][kYear]);
            fprintf(fp,"\n");
            fprintf(fp,"distributionET0,sim");
            for (int kYear=0;kYear<1+lastDayWG2D.year()-firstDayWG2D.year();kYear++)
                fprintf(fp,",%.1f",simDataAnalysis.cumulatedET0Year[i][jMonth][kYear]);
            fprintf(fp,"\n");
            fprintf(fp,"distributionBIC,sim");
            for (int kYear=0;kYear<1+lastDayWG2D.year()-firstDayWG2D.year();kYear++)
                fprintf(fp,",%.1f",simDataAnalysis.cumulatedBICYear[i][jMonth][kYear]);
            fprintf(fp,"\n");
        }
    }
    fclose(fp);

    dailyVariableWG2D.clear();
    meteoGridDbHandlerWG2D->closeDatabase();
    free(cellCode);

    // end of read second DB


    //

    int nrSites;
    //printf("insert the number of sites\n");
    //scanf("%d",&nrSites);
    nrSites = numberOfCells;
    TmonthlyData *monthlyDataClimate = (TmonthlyData*)calloc(nrSites, sizeof(TmonthlyData));
    TmonthlyData *monthlyDataSimulation = (TmonthlyData*)calloc(nrSites, sizeof(TmonthlyData));
    TmonthlyData *monthlyBias = (TmonthlyData*)calloc(nrSites, sizeof(TmonthlyData));
    TmonthlyData monthlyMaxBias,monthlyMinBias,monthlyAverageBias;
    QString outputFileNameSiteClimate;
    QString outputFileNameSiteSimulation;
    FILE *fpClimate;
    FILE *fpSimulation;
    FILE *fpStats;

    for (int iSite=0; iSite<nrSites;iSite++)
    {
        outputFileNameSiteClimate = "../test_WG2D_Eraclito/outputData/wgClimate_station_" + QString::number(iSite) + ".txt";
        //std::cout << "...read WG2D climate file -->  " << outputFileNameSiteClimate.toStdString() << "\n";
        QByteArray temp;
        temp = outputFileNameSiteClimate.toLocal8Bit();
        const char* fileName;
        fileName = temp.data();
        fpClimate = fopen(fileName,"r");
        readFileContents(fpClimate,iSite,monthlyDataClimate);
        fclose(fpClimate);



        outputFileNameSiteSimulation = "../test_WG2D_Eraclito/outputData/wgSimulation_station_" + QString::number(iSite) + ".txt";
        //std::cout << "...read WG2D simulation file -->  " << outputFileNameSiteSimulation.toStdString() << "\n";

        temp = outputFileNameSiteSimulation.toLocal8Bit();
        fileName = temp.data();
        fpSimulation = fopen(fileName,"r");
        readFileContents(fpSimulation,iSite,monthlyDataSimulation);
        fclose(fpSimulation);

    }


    // determine the bias

    computeBias(monthlyDataClimate,monthlyDataSimulation,monthlyBias,&monthlyMaxBias,&monthlyMinBias,&monthlyAverageBias,nrSites);
    const char* fileName;
    fileName = "monthlyStatisticsPrecipitation.txt";
    fpStats = fopen(fileName,"w");

    fprintf(fpStats,"bias cumulated prec (mm)\n");
    fprintf(fpStats,"month\tmin\t   max\t   average\n");
    for (int j=0;j<12;j++)
    {
        fprintf(fpStats,"%d\t%f\t%f\t%f\n",j+1,monthlyMinBias.sumPrec[j],monthlyMaxBias.sumPrec[j],monthlyAverageBias.sumPrec[j]);
    }
    fprintf(fpStats,"\nbias of probability of wet days (percentage)\n");
    fprintf(fpStats,"month\tmin\t   max\t   average\n");
    for (int j=0;j<12;j++)
    {
        fprintf(fpStats,"%d %f %f %f\n",j+1,100 * monthlyMinBias.fractionWetDays[j],100 * monthlyMaxBias.fractionWetDays[j], 100 * monthlyAverageBias.fractionWetDays[j]);
    }
    fprintf(fpStats,"\nbias probability wetwet (percentage)\n");
    fprintf(fpStats,"month\tmin\t   max\t   average\n");
    for (int j=0;j<12;j++)
    {
        fprintf(fpStats,"%d\t%f\t%f\t%f\n",j+1,100 * monthlyMinBias.fractionWetWet[j],100 * monthlyMaxBias.fractionWetWet[j],100 * monthlyAverageBias.fractionWetWet[j]);
    }

    free(monthlyDataClimate);
    free(monthlyDataSimulation);
    free(monthlyBias);

    return 0;
}

void readFileContents(FILE *fp,int site,TmonthlyData *monthlyData)
{
    for (int iMonth=0;iMonth<12;iMonth++)
    {
        for (int iVariable=0;iVariable<19;iVariable++)
        {
            char dummy;
            char variable[10000];
            double variableNumberFormat;
            for (int i=0;i<10000;i++)
            {
                variable[i] = '\0';
            }
            int counter = 0;
            dummy = getc(fp);
            while (dummy != '\n' && dummy != EOF)
            {
                variable[counter] = dummy;
                counter++;
                dummy = getc(fp);
            }
            variableNumberFormat = atof(variable);
            //printf("%f\n",variableNumberFormat);
            if (iVariable == 0) monthlyData[site].month[iMonth] = (int)(variableNumberFormat);
            if (iVariable == 1) monthlyData[site].averageTmin[iMonth] = variableNumberFormat;
            if (iVariable == 2) monthlyData[site].averageTmax[iMonth] = variableNumberFormat;
            if (iVariable == 3) monthlyData[site].sumPrec[iMonth] = variableNumberFormat;
            if (iVariable == 4) monthlyData[site].stdDevTmin[iMonth] = variableNumberFormat;
            if (iVariable == 5) monthlyData[site].stdDevTmax[iMonth] = variableNumberFormat;
            if (iVariable == 6) monthlyData[site].fractionWetDays[iMonth] = variableNumberFormat;
            if (iVariable == 7) monthlyData[site].fractionWetWet[iMonth] = variableNumberFormat;
            if (iVariable == 8) monthlyData[site].dewPointTmax[iMonth] = variableNumberFormat;
            if (iVariable == 9) monthlyData[site].dryAverageTmin[iMonth] = variableNumberFormat;
            if (iVariable == 10) monthlyData[site].dryAverageTmax[iMonth] = variableNumberFormat;
            if (iVariable == 11) monthlyData[site].wetAverageTmin[iMonth] = variableNumberFormat;
            if (iVariable == 12) monthlyData[site].wetAverageTmax[iMonth] = variableNumberFormat;
            if (iVariable == 13) monthlyData[site].dryStdDevTmin[iMonth] = variableNumberFormat;
            if (iVariable == 14) monthlyData[site].dryStdDevTmax[iMonth] = variableNumberFormat;
            if (iVariable == 15) monthlyData[site].wetStdDevTmin[iMonth] = variableNumberFormat;
            if (iVariable == 16) monthlyData[site].wetStdDevTmax[iMonth] = variableNumberFormat;
            if (iVariable == 17) continue;
            if (iVariable == 18) continue;
        }
    }
}

void computeBias(TmonthlyData *observed, TmonthlyData *simulated, TmonthlyData* bias, TmonthlyData* monthlyMax, TmonthlyData* monthlyMin, TmonthlyData* monthlyAverage, int nrSites)
{
    for (int j=0;j<12;j++)
    {
        monthlyMax->month[j] = NODATA;
        monthlyMax->averageTmax[j] = NODATA;
        monthlyMax->averageTmin[j] = NODATA;
        monthlyMax->sumPrec[j] = NODATA;
        monthlyMax->stdDevTmax[j] = NODATA;
        monthlyMax->stdDevTmin[j] = NODATA;
        monthlyMax->dewPointTmax[j] = NODATA;
        monthlyMax->dryAverageTmax[j] = NODATA;
        monthlyMax->dryAverageTmin[j] = NODATA;
        monthlyMax->wetAverageTmax[j] = NODATA;
        monthlyMax->wetAverageTmin[j] = NODATA;
        monthlyMax->fractionWetDays[j] = NODATA;
        monthlyMax->fractionWetWet[j] = NODATA;
        monthlyMin->month[j] = 9999;
        monthlyMin->averageTmax[j] = 9999;
        monthlyMin->averageTmin[j] = 9999;
        monthlyMin->sumPrec[j] = 9999;
        monthlyMin->stdDevTmax[j] = 9999;
        monthlyMin->stdDevTmin[j] = 9999;
        monthlyMin->dewPointTmax[j] = 9999;
        monthlyMin->dryAverageTmax[j] = 9999;
        monthlyMin->dryAverageTmin[j] = 9999;
        monthlyMin->wetAverageTmax[j] = 9999;
        monthlyMin->wetAverageTmin[j] = 9999;
        monthlyMin->fractionWetDays[j] = 9999;
        monthlyMin->fractionWetWet[j] = 9999;
        monthlyAverage->month[j] = 0;
        monthlyAverage->averageTmax[j] = 0;
        monthlyAverage->averageTmin[j] = 0;
        monthlyAverage->sumPrec[j] = 0;
        monthlyAverage->stdDevTmax[j] = 0;
        monthlyAverage->stdDevTmin[j] = 0;
        monthlyAverage->dewPointTmax[j] = 0;
        monthlyAverage->dryAverageTmax[j] = 0;
        monthlyAverage->dryAverageTmin[j] = 0;
        monthlyAverage->wetAverageTmax[j] = 0;
        monthlyAverage->wetAverageTmin[j] = 0;
        monthlyAverage->fractionWetDays[j] = 0;
        monthlyAverage->fractionWetWet[j] = 0;


    }
    for (int i=0; i<nrSites;i++)
    {
        for (int j=0;j<12;j++)
        {
            bias[i].month[j] = observed[i].month[j];
            bias[i].averageTmax[j] = simulated[i].averageTmax[j] - observed[i].averageTmax[j];
            bias[i].averageTmin[j] = simulated[i].averageTmin[j] - observed[i].averageTmin[j];
            bias[i].sumPrec[j] = simulated[i].sumPrec[j] - observed[i].sumPrec[j];
            bias[i].stdDevTmax[j] = simulated[i].stdDevTmax[j] - observed[i].stdDevTmax[j];
            bias[i].stdDevTmin[j] = simulated[i].stdDevTmin[j] - observed[i].stdDevTmin[j];
            bias[i].dewPointTmax[j] = simulated[i].dewPointTmax[j] - observed[i].dewPointTmax[j];
            bias[i].dryAverageTmax[j] = simulated[i].dryAverageTmax[j] - observed[i].dryAverageTmax[j];
            bias[i].dryAverageTmin[j] = simulated[i].dryAverageTmin[j] - observed[i].dryAverageTmin[j];
            bias[i].wetAverageTmax[j] = simulated[i].wetAverageTmax[j] - observed[i].wetAverageTmax[j];
            bias[i].wetAverageTmin[j] = simulated[i].wetAverageTmin[j] - observed[i].wetAverageTmin[j];
            bias[i].fractionWetDays[j] = simulated[i].fractionWetDays[j] - observed[i].fractionWetDays[j];
            bias[i].fractionWetWet[j] = simulated[i].fractionWetWet[j] - observed[i].fractionWetWet[j];

            monthlyAverage->month[j] =  bias[i].month[j];
            monthlyAverage->averageTmax[j] += bias[i].averageTmax[j]/nrSites;
            monthlyAverage->averageTmin[j] += bias[i].averageTmin[j]/nrSites;
            monthlyAverage->sumPrec[j] += bias[i].sumPrec[j]/nrSites;
            monthlyAverage->stdDevTmax[j] += bias[i].stdDevTmax[j]/nrSites;
            monthlyAverage->stdDevTmin[j] += bias[i].stdDevTmin[j]/nrSites;
            monthlyAverage->dewPointTmax[j] += bias[i].dewPointTmax[j]/nrSites;
            monthlyAverage->dryAverageTmax[j] += bias[i].dryAverageTmax[j]/nrSites;
            monthlyAverage->dryAverageTmin[j] += bias[i].dryAverageTmin[j]/nrSites;
            monthlyAverage->wetAverageTmax[j] += bias[i].wetAverageTmax[j]/nrSites;
            monthlyAverage->wetAverageTmin[j] += bias[i].wetAverageTmin[j]/nrSites;
            monthlyAverage->fractionWetDays[j] += bias[i].fractionWetDays[j]/nrSites;
            monthlyAverage->fractionWetWet[j] += bias[i].fractionWetWet[j]/nrSites;


            monthlyMax->month[j] = bias[i].month[j];
            monthlyMax->averageTmax[j] = MAXVALUE(monthlyMax->averageTmax[j],simulated[i].averageTmax[j] - observed[i].averageTmax[j]);
            monthlyMax->averageTmin[j] = MAXVALUE(monthlyMax->averageTmin[j],simulated[i].averageTmin[j] - observed[i].averageTmin[j]);
            monthlyMax->sumPrec[j] = MAXVALUE(monthlyMax->sumPrec[j],simulated[i].sumPrec[j] - observed[i].sumPrec[j]);
            monthlyMax->stdDevTmax[j] = MAXVALUE(monthlyMax->stdDevTmax[j],simulated[i].stdDevTmax[j] - observed[i].stdDevTmax[j]);
            monthlyMax->stdDevTmin[j] = MAXVALUE(monthlyMax->stdDevTmin[j],simulated[i].stdDevTmin[j] - observed[i].stdDevTmin[j]);
            monthlyMax->dewPointTmax[j] = MAXVALUE(monthlyMax->dewPointTmax[j],simulated[i].dewPointTmax[j] - observed[i].dewPointTmax[j]);
            monthlyMax->dryAverageTmax[j] = MAXVALUE(monthlyMax->dryAverageTmax[j],simulated[i].dryAverageTmax[j] - observed[i].dryAverageTmax[j]);
            monthlyMax->dryAverageTmin[j] = MAXVALUE(monthlyMax->dryAverageTmin[j],simulated[i].dryAverageTmin[j] - observed[i].dryAverageTmin[j]);
            monthlyMax->wetAverageTmax[j] = MAXVALUE(monthlyMax->wetAverageTmax[j],simulated[i].wetAverageTmax[j] - observed[i].wetAverageTmax[j]);
            monthlyMax->wetAverageTmin[j] = MAXVALUE(monthlyMax->wetAverageTmin[j],simulated[i].wetAverageTmin[j] - observed[i].wetAverageTmin[j]);
            monthlyMax->fractionWetDays[j] = MAXVALUE(monthlyMax->fractionWetDays[j],simulated[i].fractionWetDays[j] - observed[i].fractionWetDays[j]);
            monthlyMax->fractionWetWet[j] = MAXVALUE(monthlyMax->fractionWetWet[j],simulated[i].fractionWetWet[j] - observed[i].fractionWetWet[j]);

            monthlyMin->month[j] = bias[i].month[j];
            monthlyMin->averageTmax[j] = MINVALUE(monthlyMin->averageTmax[j],simulated[i].averageTmax[j] - observed[i].averageTmax[j]);
            monthlyMin->averageTmin[j] = MINVALUE(monthlyMin->averageTmin[j],simulated[i].averageTmin[j] - observed[i].averageTmin[j]);
            monthlyMin->sumPrec[j] = MINVALUE(monthlyMin->sumPrec[j],simulated[i].sumPrec[j] - observed[i].sumPrec[j]);
            monthlyMin->stdDevTmax[j] = MINVALUE(monthlyMin->stdDevTmax[j],simulated[i].stdDevTmax[j] - observed[i].stdDevTmax[j]);
            monthlyMin->stdDevTmin[j] = MINVALUE(monthlyMin->stdDevTmin[j],simulated[i].stdDevTmin[j] - observed[i].stdDevTmin[j]);
            monthlyMin->dewPointTmax[j] = MINVALUE(monthlyMin->dewPointTmax[j],simulated[i].dewPointTmax[j] - observed[i].dewPointTmax[j]);
            monthlyMin->dryAverageTmax[j] = MINVALUE(monthlyMin->dryAverageTmax[j],simulated[i].dryAverageTmax[j] - observed[i].dryAverageTmax[j]);
            monthlyMin->dryAverageTmin[j] = MINVALUE(monthlyMin->dryAverageTmin[j],simulated[i].dryAverageTmin[j] - observed[i].dryAverageTmin[j]);
            monthlyMin->wetAverageTmax[j] = MINVALUE(monthlyMin->wetAverageTmax[j],simulated[i].wetAverageTmax[j] - observed[i].wetAverageTmax[j]);
            monthlyMin->wetAverageTmin[j] = MINVALUE(monthlyMin->wetAverageTmin[j],simulated[i].wetAverageTmin[j] - observed[i].wetAverageTmin[j]);
            monthlyMin->fractionWetDays[j] = MINVALUE(monthlyMin->fractionWetDays[j],simulated[i].fractionWetDays[j] - observed[i].fractionWetDays[j]);
            monthlyMin->fractionWetWet[j] = MINVALUE(monthlyMin->fractionWetWet[j],simulated[i].fractionWetWet[j] - observed[i].fractionWetWet[j]);
        }
    }

}

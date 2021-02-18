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

};

void readFileContents(FILE *fp,int site,TmonthlyData* monthlyData);
void computeBias(TmonthlyData *observed, TmonthlyData *simulated, TmonthlyData* bias, TmonthlyData *monthlyMax, TmonthlyData *monthlyMin, TmonthlyData *monthlyAverage, int nrSites);
int readERG5CellListNumber(FILE *fp);
void readTheCellNumber(FILE *fp, char* numCell);
void logInfo(QString myStr);
bool loadMeteoGridDB(QString* errorString, QString xmlName);


static Crit3DMeteoGridDbHandler* meteoGridDbHandler;
static Crit3DMeteoGridDbHandler* meteoGridDbHandlerWG2D;
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


bool loadMeteoGridDB(QString* errorString, QString xmlName)
{
    //QString xmlName = QFileDialog::getOpenFileName(nullptr, "Open XML grid", "", "XML files (*.xml)");

    QString path;
    if (! searchDataPath(&path)) return -1;
    //QString xmlName = path + "METEOGRID/DBGridXML_Eraclito4.xml";
    //QString xmlName = path + "METEOGRID/DBGridXML_ERG5_v2.1.xml";
    xmlName = path + xmlName;
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


int main(int argc, char *argv[])
{
    QCoreApplication myApp(argc, argv);
    QString appPath = myApp.applicationDirPath() + "/";

    QString myError;
    //Crit3DMeteoPoint* meteoPointTemp = new Crit3DMeteoPoint;
    meteoVariable variable;
    QDate firstDay(2001,1,1);
    QDate lastDay(2020,12,31);
    QDate currentDay;
    QDate firstDateDB(1,1,1);
    TObsDataD** obsDataD = nullptr;
    QString xmlName;
    xmlName = "METEOGRID/DBGridXML_ERG5_v2.1.xml";
    //xmlName = "METEOGRID/DBGridXML_Output_WG2D.xml";
    QString errorString;
    if (! loadMeteoGridDB(&errorString,xmlName))
    {
        std::cout << errorString.toStdString() << std::endl;
        return -1;
    }
    std::string id;
    int nrActivePoints = 0;
    int lengthSeries = 0;
    std::vector<float> dailyVariable;
    FILE* fp;
    fp = fopen("../test_WG2D_Eraclito/inputData/list_enza_secchia_panaro_30_sites.txt","r"); // !! take out
    //fp = fopen("./inputData/list_C7_shortlisted_few_sites.txt","r"); // !! take out
    int numberOfCells; // !! take out
    numberOfCells = readERG5CellListNumber(fp); // !! take out
    fclose(fp); // !! take out

    fp = fopen("../test_WG2D_Eraclito/inputData/list_enza_secchia_panaro_30_sites.txt","r"); // !! take out
    //fp = fopen("./inputData/list_C7_shortlisted_few_sites.txt","r"); // !! take out

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

// read second DB
    printf("second DB\n");
    QDate firstDayWG2D(2001,1,1);
    QDate lastDayWG2D(2020,12,31);

    TObsDataD** outputDataD = nullptr;
    xmlName = "METEOGRID/DBGridXML_Output_WG2D.xml";

    if (! loadMeteoGridDB(&errorString,xmlName))
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
    fp = fopen("../test_WG2D_Eraclito/inputData/list_enza_secchia_panaro_30_sites.txt","r"); // !! take out
    //fp = fopen("./inputData/list_C7_shortlisted_few_sites.txt","r"); // !! take out
    //int numberOfCells; // !! take out
    numberOfCells = readERG5CellListNumber(fp); // !! take out
    fclose(fp); // !! take out

    fp = fopen("../test_WG2D_Eraclito/inputData/list_enza_secchia_panaro_30_sites.txt","r"); // !! take out
    //fp = fopen("./inputData/list_C7_shortlisted_few_sites.txt","r"); // !! take out

    //int* cellCode = nullptr; // !! take out
    //char* numCell = (char *)calloc(6, sizeof(char)); // !! take out
    cellCode = (int *) calloc(numberOfCells, sizeof(int)); // !! take out
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
               if (nrActivePoints == 1)
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
        }
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

    dailyVariableWG2D.clear();
    meteoGridDbHandlerWG2D->closeDatabase();


    // end of read second DB




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
    return 0;
}

void readFileContents(FILE *fp,int site,TmonthlyData *monthlyData)
{
    for (int iMonth=0;iMonth<12;iMonth++)
    {
        for (int iVariable=0;iVariable<13;iVariable++)
        {
            char dummy;
            char variable[100];
            double variableNumberFormat;
            for (int i=0;i<100;i++)
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

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
    double probabilityWetWet[12];
    double dewPointTmax[12];
    double dryAverageTmin[12];
    double dryAverageTmax[12];
    double wetAverageTmin[12];
    double wetAverageTmax[12];

};

void readFileContents(FILE *fp,int site,TmonthlyData* monthlyData);

int main(int argc, char *argv[])
{

    int nrSites;
    printf("insert the number of sites\n");
    scanf("%d",&nrSites);
    TmonthlyData *monthlyDataClimate = (TmonthlyData*)calloc(nrSites, sizeof(TmonthlyData));
    TmonthlyData *monthlyDataSimulation = (TmonthlyData*)calloc(nrSites, sizeof(TmonthlyData));
    QString outputFileNameSiteClimate;
    QString outputFileNameSiteSimulation;
    FILE *fpClimate;
    FILE *fpSimulation;

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

        /*

        outputFileNameSiteSimulation = "../test_WG2D_Eraclito/outputData/wgSimulation_station_" + QString::number(i) + ".txt";
        //std::cout << "...read WG2D simulation file -->  " << outputFileNameSiteSimulation.toStdString() << "\n";

        temp = outputFileNameSiteSimulation.toLocal8Bit();
        fileName = temp.data();
        fpSimulation = fopen(fileName,"r");
        readFileContents(fpSimulation,156);
        fclose(fpSimulation);
        */
    }


    printf("Hello World!\n");
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
            printf("%f\n",variableNumberFormat);
        }
    }
}

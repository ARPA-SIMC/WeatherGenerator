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
void computeBias(TmonthlyData *observed, TmonthlyData *simulated, TmonthlyData* bias, TmonthlyData monthlyMax, TmonthlyData monthlyMin, int nrSites);



int main(int argc, char *argv[])
{

    int nrSites;
    printf("insert the number of sites\n");
    scanf("%d",&nrSites);
    TmonthlyData *monthlyDataClimate = (TmonthlyData*)calloc(nrSites, sizeof(TmonthlyData));
    TmonthlyData *monthlyDataSimulation = (TmonthlyData*)calloc(nrSites, sizeof(TmonthlyData));
    TmonthlyData *monthlyBias = (TmonthlyData*)calloc(nrSites, sizeof(TmonthlyData));
    TmonthlyData monthlyMaxBias,monthlyMinBias;
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



        outputFileNameSiteSimulation = "../test_WG2D_Eraclito/outputData/wgSimulation_station_" + QString::number(iSite) + ".txt";
        //std::cout << "...read WG2D simulation file -->  " << outputFileNameSiteSimulation.toStdString() << "\n";

        temp = outputFileNameSiteSimulation.toLocal8Bit();
        fileName = temp.data();
        fpSimulation = fopen(fileName,"r");
        readFileContents(fpSimulation,iSite,monthlyDataSimulation);
        fclose(fpSimulation);

    }


    // determine the bias

    computeBias(monthlyDataClimate,monthlyDataSimulation,monthlyBias,monthlyMaxBias,monthlyMinBias,nrSites);
    for (int j=0;j<12;j++)
    {
        printf("max %d  %f%f%f%f%f%f%f%f%f%f%f%f  \n min %d  %f%f%f%f%f%f%f%f%f%f%f%f \n",
               monthlyMaxBias.month[j],
               monthlyMaxBias.averageTmax[j],
               monthlyMaxBias.averageTmin[j],
               monthlyMaxBias.sumPrec[j],
               monthlyMaxBias.stdDevTmax[j],
               monthlyMaxBias.stdDevTmin[j],
               monthlyMaxBias.dewPointTmax[j],
               monthlyMaxBias.dryAverageTmax[j],
               monthlyMaxBias.dryAverageTmin[j],
               monthlyMaxBias.wetAverageTmax[j],
               monthlyMaxBias.wetAverageTmin[j],
               monthlyMaxBias.fractionWetDays[j],
               monthlyMaxBias.probabilityWetWet[j],
               monthlyMinBias.month[j],
               monthlyMinBias.averageTmax[j],
               monthlyMinBias.averageTmin[j],
               monthlyMinBias.sumPrec[j],
               monthlyMinBias.stdDevTmax[j],
               monthlyMinBias.stdDevTmin[j],
               monthlyMinBias.dewPointTmax[j],
               monthlyMinBias.dryAverageTmax[j],
               monthlyMinBias.dryAverageTmin[j],
               monthlyMinBias.wetAverageTmax[j],
               monthlyMinBias.wetAverageTmin[j],
               monthlyMinBias.fractionWetDays[j],
               monthlyMinBias.probabilityWetWet[j]);
    }

    //printf("Hello World!\n");
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
            if (iVariable == 7) monthlyData[site].probabilityWetWet[iMonth] = variableNumberFormat;
            if (iVariable == 8) monthlyData[site].dewPointTmax[iMonth] = variableNumberFormat;
            if (iVariable == 9) monthlyData[site].dryAverageTmin[iMonth] = variableNumberFormat;
            if (iVariable == 10) monthlyData[site].dryAverageTmax[iMonth] = variableNumberFormat;
            if (iVariable == 11) monthlyData[site].wetAverageTmin[iMonth] = variableNumberFormat;
            if (iVariable == 12) monthlyData[site].wetAverageTmax[iMonth] = variableNumberFormat;
        }
    }
}

void computeBias(TmonthlyData *observed, TmonthlyData *simulated, TmonthlyData* bias, TmonthlyData monthlyMax, TmonthlyData monthlyMin, int nrSites)
{
    for (int j=0;j<12;j++)
    {
        monthlyMax.month[j] = NODATA;
        monthlyMax.averageTmax[j] = NODATA;
        monthlyMax.averageTmin[j] = NODATA;
        monthlyMax.sumPrec[j] = NODATA;
        monthlyMax.stdDevTmax[j] = NODATA;
        monthlyMax.stdDevTmin[j] = NODATA;
        monthlyMax.dewPointTmax[j] = NODATA;
        monthlyMax.dryAverageTmax[j] = NODATA;
        monthlyMax.dryAverageTmin[j] = NODATA;
        monthlyMax.wetAverageTmax[j] = NODATA;
        monthlyMax.wetAverageTmin[j] = NODATA;
        monthlyMax.fractionWetDays[j] = NODATA;
        monthlyMax.probabilityWetWet[j] = NODATA;
        monthlyMin.month[j] = -NODATA;
        monthlyMin.averageTmax[j] = -NODATA;
        monthlyMin.averageTmin[j] = -NODATA;
        monthlyMin.sumPrec[j] = -NODATA;
        monthlyMin.stdDevTmax[j] = -NODATA;
        monthlyMin.stdDevTmin[j] = -NODATA;
        monthlyMin.dewPointTmax[j] = -NODATA;
        monthlyMin.dryAverageTmax[j] = -NODATA;
        monthlyMin.dryAverageTmin[j] = -NODATA;
        monthlyMin.wetAverageTmax[j] = -NODATA;
        monthlyMin.wetAverageTmin[j] = -NODATA;
        monthlyMin.fractionWetDays[j] = -NODATA;
        monthlyMin.probabilityWetWet[j] = -NODATA;
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
            bias[i].probabilityWetWet[j] = simulated[i].probabilityWetWet[j] - observed[i].probabilityWetWet[j];

            monthlyMax.month[j] = bias[i].month[j];
            monthlyMax.averageTmax[j] = MAXVALUE(monthlyMax.averageTmax[j],simulated[i].averageTmax[j] - observed[i].averageTmax[j]);
            monthlyMax.averageTmin[j] = MAXVALUE(monthlyMax.averageTmin[j],simulated[i].averageTmin[j] - observed[i].averageTmin[j]);
            monthlyMax.sumPrec[j] = MAXVALUE(monthlyMax.sumPrec[j],simulated[i].sumPrec[j] - observed[i].sumPrec[j]);
            monthlyMax.stdDevTmax[j] = MAXVALUE(monthlyMax.stdDevTmax[j],simulated[i].stdDevTmax[j] - observed[i].stdDevTmax[j]);
            monthlyMax.stdDevTmin[j] = MAXVALUE(monthlyMax.stdDevTmin[j],simulated[i].stdDevTmin[j] - observed[i].stdDevTmin[j]);
            monthlyMax.dewPointTmax[j] = MAXVALUE(monthlyMax.dewPointTmax[j],simulated[i].dewPointTmax[j] - observed[i].dewPointTmax[j]);
            monthlyMax.dryAverageTmax[j] = MAXVALUE(monthlyMax.dryAverageTmax[j],simulated[i].dryAverageTmax[j] - observed[i].dryAverageTmax[j]);
            monthlyMax.dryAverageTmin[j] = MAXVALUE(monthlyMax.dryAverageTmin[j],simulated[i].dryAverageTmin[j] - observed[i].dryAverageTmin[j]);
            monthlyMax.wetAverageTmax[j] = MAXVALUE(monthlyMax.wetAverageTmax[j],simulated[i].wetAverageTmax[j] - observed[i].wetAverageTmax[j]);
            monthlyMax.wetAverageTmin[j] = MAXVALUE(monthlyMax.wetAverageTmin[j],simulated[i].wetAverageTmin[j] - observed[i].wetAverageTmin[j]);
            monthlyMax.fractionWetDays[j] = MAXVALUE(monthlyMax.fractionWetDays[j],simulated[i].fractionWetDays[j] - observed[i].fractionWetDays[j]);
            monthlyMax.probabilityWetWet[j] = MAXVALUE(monthlyMax.probabilityWetWet[j],simulated[i].probabilityWetWet[j] - observed[i].probabilityWetWet[j]);

            monthlyMin.month[j] = bias[i].month[j];
            monthlyMin.averageTmax[j] = MINVALUE(monthlyMin.averageTmax[j],simulated[i].averageTmax[j] - observed[i].averageTmax[j]);
            monthlyMin.averageTmin[j] = MINVALUE(monthlyMin.averageTmin[j],simulated[i].averageTmin[j] - observed[i].averageTmin[j]);
            monthlyMin.sumPrec[j] = MINVALUE(monthlyMin.sumPrec[j],simulated[i].sumPrec[j] - observed[i].sumPrec[j]);
            monthlyMin.stdDevTmax[j] = MINVALUE(monthlyMin.stdDevTmax[j],simulated[i].stdDevTmax[j] - observed[i].stdDevTmax[j]);
            monthlyMin.stdDevTmin[j] = MINVALUE(monthlyMin.stdDevTmin[j],simulated[i].stdDevTmin[j] - observed[i].stdDevTmin[j]);
            monthlyMin.dewPointTmax[j] = MINVALUE(monthlyMin.dewPointTmax[j],simulated[i].dewPointTmax[j] - observed[i].dewPointTmax[j]);
            monthlyMin.dryAverageTmax[j] = MINVALUE(monthlyMin.dryAverageTmax[j],simulated[i].dryAverageTmax[j] - observed[i].dryAverageTmax[j]);
            monthlyMin.dryAverageTmin[j] = MINVALUE(monthlyMin.dryAverageTmin[j],simulated[i].dryAverageTmin[j] - observed[i].dryAverageTmin[j]);
            monthlyMin.wetAverageTmax[j] = MINVALUE(monthlyMin.wetAverageTmax[j],simulated[i].wetAverageTmax[j] - observed[i].wetAverageTmax[j]);
            monthlyMin.wetAverageTmin[j] = MINVALUE(monthlyMin.wetAverageTmin[j],simulated[i].wetAverageTmin[j] - observed[i].wetAverageTmin[j]);
            monthlyMin.fractionWetDays[j] = MINVALUE(monthlyMin.fractionWetDays[j],simulated[i].fractionWetDays[j] - observed[i].fractionWetDays[j]);
            monthlyMin.probabilityWetWet[j] = MINVALUE(monthlyMin.probabilityWetWet[j],simulated[i].probabilityWetWet[j] - observed[i].probabilityWetWet[j]);
        }
    }
}

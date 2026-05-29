#include <QCoreApplication>

int main2(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    return a.exec();
}

#include <iostream>
#include <vector>
#include <string>

#include "functionsIO.h"

#include <iostream>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

int main3() {
    std::string directory_path = "../DATA/TEST_weatherGeneratorScenario/input/climate/climate/";

    try {
        for (const auto& entry : fs::directory_iterator(directory_path)) {
            if (entry.is_regular_file() && entry.path().extension() == ".csv") {
                std::string old_name = entry.path().filename().string();

                // Controlla se il nome inizia già con "GRD_"
                if (old_name.substr(0, 4) != "GRD_") {
                    std::string new_name = "GRD_" + old_name;
                    fs::rename(entry.path(), entry.path().parent_path() / new_name);
                    std::cout << "Rinominato " << old_name << " in " << new_name << std::endl;
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Errore del file system: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Rinominamento completato." << std::endl;
    return 0;
}


int main_old()
{
    //std::string dataPath = "C:/Github/WeatherGenerator/WG1D/DATA/ARCADIA_WG/";   // TODO
    //std::string filenameAnagraphicExtraER = dataPath + "anagrafica_Erg5_Eraclito_pointsExtraER.csv";

    std::string dataPath = "C:/Github/WeatherGenerator/WG1D/DATA/ARCADIA_WG/";

    std::string filenameAnagraphic = dataPath + "input/originalCsvFiles/anagrafica_SDs_scenari_ER.csv";
    std::vector<DataProperties> dataAnagraphicCodeName;
    //readAnagraficaCSV(filenameAnagraphic,dataAnagraphicCodeName);

    std::vector<std::string> filenameTmax;
    std::vector<std::string> filenameTmin;
    std::vector<std::string> filenamePrec3M;
    std::vector<std::string> filenameWetDaysFrequency;
    filenameTmax.resize(4);
    filenameTmin.resize(4);
    filenamePrec3M.resize(4);

    filenameWetDaysFrequency.resize(4);
    // definisce i nomi dei file di scenario
    filenameTmax[0] = dataPath + "input/originalCsvFiles/SD_GCMs-SD_EM-1961-1990-RCP4.5-2021-2050-DJF-Tmax-anomaly.csv";
    filenameTmin[0] = dataPath + "input/originalCsvFiles/SD_GCMs-SD_EM-1961-1990-RCP4.5-2021-2050-DJF-Tmin-anomaly.csv";
    filenameTmax[1] = dataPath + "input/originalCsvFiles/SD_GCMs-SD_EM-1961-1990-RCP4.5-2021-2050-MAM-Tmax-anomaly.csv";
    filenameTmin[1] = dataPath + "input/originalCsvFiles/SD_GCMs-SD_EM-1961-1990-RCP4.5-2021-2050-MAM-Tmin-anomaly.csv";
    filenameTmax[2] = dataPath + "input/originalCsvFiles/SD_GCMs-SD_EM-1961-1990-RCP4.5-2021-2050-JJA-Tmax-anomaly.csv";
    filenameTmin[2] = dataPath + "input/originalCsvFiles/SD_GCMs-SD_EM-1961-1990-RCP4.5-2021-2050-JJA-Tmin-anomaly.csv";
    filenameTmax[3] = dataPath + "input/originalCsvFiles/SD_GCMs-SD_EM-1961-1990-RCP4.5-2021-2050-SON-Tmax-anomaly.csv";
    filenameTmin[3] = dataPath + "input/originalCsvFiles/SD_GCMs-SD_EM-1961-1990-RCP4.5-2021-2050-SON-Tmin-anomaly.csv";


    // TODO VAR
    // TODO PARTE FISSA filename
    filenamePrec3M[0] = dataPath + "input/originalCsvFiles/SD_GCMs-SD_EM-1961-1990-RCP4.5-2021-2050-DJF-Prec3M-anomaly.csv";
    filenameWetDaysFrequency[0] = dataPath +  "input/originalCsvFiles/SD_GCMs-SD_EM-1961-1990-RCP4.5-2021-2050-DJF-WetDaysFrequency-anomaly.csv";
    filenamePrec3M[1] = dataPath +  "input/originalCsvFiles/SD_GCMs-SD_EM-1961-1990-RCP4.5-2021-2050-MAM-Prec3M-anomaly.csv";
    filenameWetDaysFrequency[1] = dataPath +  "input/originalCsvFiles/SD_GCMs-SD_EM-1961-1990-RCP4.5-2021-2050-MAM-WetDaysFrequency-anomaly.csv";
    filenamePrec3M[2] = dataPath +  "input/originalCsvFiles/SD_GCMs-SD_EM-1961-1990-RCP4.5-2021-2050-JJA-Prec3M-anomaly.csv";
    filenameWetDaysFrequency[2] = dataPath +  "input/originalCsvFiles/SD_GCMs-SD_EM-1961-1990-RCP4.5-2021-2050-JJA-WetDaysFrequency-anomaly.csv";
    filenamePrec3M[3] = dataPath + "input/originalCsvFiles/SD_GCMs-SD_EM-1961-1990-RCP4.5-2021-2050-SON-Prec3M-anomaly.csv";
    filenameWetDaysFrequency[3] = dataPath +  "input/originalCsvFiles/SD_GCMs-SD_EM-1961-1990-RCP4.5-2021-2050-SON-WetDaysFrequency-anomaly.csv";

    // crea se necessario dei file con anagrafica e anomalia nulla in modo da confrontare il clima con il clima riprodotto dal weather generator
    for (int i=0;i<4;i++)
    {
        //writeCsvForTest(filenameTmax[i],dataAnagraphicCodeName,0.0);
        //writeCsvForTest(filenameTmin[i],dataAnagraphicCodeName,0.0);
        //writeCsvForTest(filenamePrec3M[i],dataAnagraphicCodeName,0.0);
        //writeCsvForTest(filenameWetDaysFrequency[i],dataAnagraphicCodeName,0.0);
    }

    std::vector<std::vector<DataRow>> dataTmax;
    std::vector<std::vector<DataRow>> dataTmin;
    std::vector<std::vector<DataRow>> dataPrec3M;
    std::vector<std::vector<DataRow>> dataWetDaysFrequency;

    dataTmax.resize(4);
    dataTmin.resize(4);
    dataPrec3M.resize(4);
    dataWetDaysFrequency.resize(4);

    // Estrai le variabili dal nome del file
    std::vector<std::string> variables;
    //extractVariables(filenameTmax[0], variables);
    // Stampa le variabili estratte
    std::cout << "Variabili estratte dal nome del file:" << std::endl;
    for (const auto &var : variables)
    {
        std::cout << var << std::endl;
    }

    // TODO pos code, lat, lon, valore
    // lettura dei file uno per variabile
    /*
    for(int i=0;i<4;i++)
    {
        readCSV(filenameTmax[i], dataTmax[i]);
        readCSV(filenameTmin[i], dataTmin[i]);
        readCSV(filenamePrec3M[i], dataPrec3M[i]);
        readCSV(filenameWetDaysFrequency[i], dataWetDaysFrequency[i]);
    }*/

    //std::vector<DataVariable> anomalies;
    //anomalies.resize(4);
    // scrive i file xml
    int cell = 0;
    for (cell = 0; cell < dataTmax[0].size();cell++)
    {
        std::string pathXML = dataPath + "input/scenarios/";   // TODO
        //std::string filenameXML = generateFilename(pathXML, dataTmax[0][cell].col1);
        //writeXML(filenameXML,dataTmin,dataTmax,dataPrec3M,dataWetDaysFrequency,cell,variables,dataAnagraphicCodeName);
    }
    return 0;
}

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>

#include "functionsIO.h"


void extractVariables(const std::string &filename, std::vector<std::string> &variables) {
    size_t start = filename.find_last_of("/\\") + 1; // Trova l'ultimo separatore di percorso
    size_t end = filename.find_last_of('.'); // Trova l'ultimo punto per rimuovere l'estensione
    std::string baseFilename = filename.substr(start, end - start);

    std::istringstream ss(baseFilename);
    std::string token;

    while (std::getline(ss, token, '-')) {
        variables.push_back(token);
    }
}


void readAnagraficaCSV(const std::string &filename, std::vector<DataAnagraphic> &data) {
    std::fstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Errore nell'apertura del file " << filename << std::endl;
        return;
    }

    std::string line;
    // Salta la prima riga (intestazione)
    std::getline(file, line);
    DataAnagraphic row;
    //int counter=0;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string token;
        //row.resize(counter+1);
        std::getline(ss, token, ',');
        row.code = std::stoi(token);
        std::getline(ss, token, ',');
        row.name = token;
        std::getline(ss, token, ',');
        std::getline(ss, token, ',');
        std::getline(ss, token, ',');
        std::getline(ss, token, ',');
        row.lat = std::stof(token);
        std::getline(ss, token, ',');
        row.lon = std::stof(token);
        std::getline(ss, token, ',');
        std::getline(ss, token, ',');
        std::getline(ss, token, ',');
        std::getline(ss, token, ',');
        std::getline(ss, token, ',');
        row.height = std::stof(token);
        data.push_back(row);
    }
    file.close();
}

void readCSV(const std::string &filename, std::vector<DataRow> &data) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Errore nell'apertura del file " << filename << std::endl;
        return;
    }

    std::string line;
    // Salta la prima riga (intestazione)
    std::getline(file, line);
    DataRow row;
    //int counter=0;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string token;
        //row.resize(counter+1);
        std::getline(ss, token, ',');
        row.col1 = std::stoi(token);

        std::getline(ss, token, ',');
        row.col2 = std::stof(token);

        std::getline(ss, token, ',');
        row.col3 = std::stof(token);

        std::getline(ss, token, ',');
        row.col4 = std::stof(token);

        std::getline(ss, token, ',');
        row.col5 = std::stof(token);

        data.push_back(row);
        //counter++;
    }
    file.close();
}

void writeCsvForTest(const std::string &filename, std::vector<DataAnagraphic> &data, float anomaly)
{
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Errore nell'apertura del file " << filename << std::endl;
        return;
    }
    file << "Code,Latitude,Longitude,Height,Changes2021-2050"<< std::endl;
    for (int i=0;i<data.size();i++)
    {
        file << std::setw(5) << std::setfill('0') << data[i].code << "," << data[i].lat << "," << data[i].lon << "," << data[i].height << "," << anomaly << std::endl;
    }
    file.close();
}


void writeXML(const std::string& filename, std::vector<std::vector<DataRow>>& dataTmin,
              std::vector<std::vector<DataRow>>& dataTmax, std::vector<std::vector<DataRow>>& dataPrec3M,
              std::vector<std::vector<DataRow>>& dataWetDaysFrequency,
              int cell,std::vector<std::string>& variables,std::vector<DataAnagraphic> &data)
{
    // Valori delle variabili
    std::string point_name = data[cell].name;
    int point_code = data[cell].code;
    std::string point_info = "";
    float point_lat = data[cell].lat;
    float point_lon = data[cell].lon;
    float point_height = data[cell].height;

    std::string models_type = variables[0];
    std::string models_value = variables[1];

    std::string climate_type = "";
    int climate_from = stoi(variables[2]);
    int climate_to = stoi(variables[3]);

    std::string scenario_type = variables[4];
    int scenario_from = stoi(variables[5]);
    int scenario_to = stoi(variables[6]);

    // Periodi e variabili
    struct PeriodVar {
        std::string type;
        std::string attribute;
        float value;
    };

    struct Period {
        std::string time_type;
        std::string time_from;
        std::string time_to;
        std::vector<PeriodVar> vars;
    };

    std::vector<Period> periods = {
        {"DJF", "", "", {{"Tmin", "anomaly", dataTmin[0][cell].col5}, {"Tmax", "anomaly", dataTmax[0][cell].col5}, {"Prec3M", "anomaly",dataPrec3M[0][cell].col5}, {"WetDaysFrequency", "anomaly", dataWetDaysFrequency[0][cell].col5}}},
        {"MAM", "", "", {{"Tmin", "anomaly", dataTmin[1][cell].col5}, {"Tmax", "anomaly", dataTmax[1][cell].col5}, {"Prec3M", "anomaly",dataPrec3M[1][cell].col5}, {"WetDaysFrequency", "anomaly", dataWetDaysFrequency[1][cell].col5}}},
        {"JJA", "", "", {{"Tmin", "anomaly", dataTmin[2][cell].col5}, {"Tmax", "anomaly", dataTmax[2][cell].col5}, {"Prec3M", "anomaly",dataPrec3M[2][cell].col5}, {"WetDaysFrequency", "anomaly", dataWetDaysFrequency[2][cell].col5}}},
        {"SON", "", "", {{"Tmin", "anomaly", dataTmin[3][cell].col5}, {"Tmax", "anomaly", dataTmax[3][cell].col5}, {"Prec3M", "anomaly",dataPrec3M[3][cell].col5}, {"WetDaysFrequency", "anomaly", dataWetDaysFrequency[3][cell].col5}}}
    };

    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Errore nell'apertura del file " << filename << std::endl;
        return;
    }

    file << R"(<xml version="1.0" encoding="ISO-8859-1">)" << std::endl;
    file << "<point>" << std::endl;
    file << "    <name>" << point_name << "</name>" << std::endl;
    file << "    <code>" << point_code << "</code>" << std::endl;
    file << "    <info>" << point_info << "</info>" << std::endl;
    file << "    <lat>" << point_lat << "</lat>" << std::endl;
    file << "    <lon>" << point_lon << "</lon>" << std::endl;
    file << "    <height>" << point_height << "</height>" << std::endl;
    file << "</point>" << std::endl;

    file << "<models>" << std::endl;
    file << "    <type>" << models_type << "</type>" << std::endl;
    file << "    <value>" << models_value << "</value>" << std::endl;
    file << "</models>" << std::endl;

    file << "<climate>" << std::endl;
    file << "    <type>" << climate_type << "</type>" << std::endl;
    file << "    <from>" << climate_from << "</from>" << std::endl;
    file << "    <to>" << climate_to << "</to>" << std::endl;
    file << "</climate>" << std::endl;

    file << "<scenario>" << std::endl;
    file << "    <type>" << scenario_type << "</type>" << std::endl;
    file << "    <from>" << scenario_from << "</from>" << std::endl;
    file << "    <to>" << scenario_to << "</to>" << std::endl;
    file << "</scenario>" << std::endl;

    for (const auto& period : periods) {
        file << "<period>" << std::endl;
        file << "    <time>" << std::endl;
        file << "        <type>" << period.time_type << "</type>" << std::endl;
        file << "        <from>" << period.time_from << "</from>" << std::endl;
        file << "        <to>" << period.time_to << "</to>" << std::endl;
        file << "    </time>" << std::endl;

        for (const auto& var : period.vars) {
            file << "    <var>" << std::endl;
            file << "        <type>" << var.type << "</type>" << std::endl;
            file << "        <attribute>" << var.attribute << "</attribute>" << std::endl;
            file << "        <value>" << var.value << "</value>" << std::endl;
            file << "    </var>" << std::endl;
        }
        file << "</period>" << std::endl;
    }
    file << "</xml>" <<std::endl;

    file.close();
}

std::string generateFilename(const std::string& path, int number) {
    std::ostringstream oss;
    oss << path << "GRD_" << std::setw(5) << std::setfill('0') << number << ".xml";
    return oss.str();
}

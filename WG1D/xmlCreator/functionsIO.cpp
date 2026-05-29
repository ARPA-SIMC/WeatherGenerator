#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>

#include "functionsIO.h"



bool readPropertiesCSV(const std::string &filename, const XmlScenarioSettings &settings,
                       std::vector<DataProperties> &data)
{
    std::fstream file(filename);
    if (! file.is_open())
    {
        std::cerr << "Error opening file: " << filename << std::endl;
        return false;
    }

    std::string line;
    // skip header
    std::getline(file, line);

    int maxPosition = std::max({
        settings.cellCodePosition,
        settings.latPosition,
        settings.lonPosition,
        settings.heightPosition
    });

    while (std::getline(file, line))
    {
        if (line.empty())
            continue;

        std::vector<std::string> properties;
        std::stringstream ss(line);
        std::string item;
        while (std::getline(ss, item, settings.csvSeparator))
        {
            properties.push_back(item);
        }

        if (properties.size() <= maxPosition)
        {
            std::cerr << "missing properties in file: " << filename << std::endl;
            return false;
        }

        DataProperties row;
        row.code = properties[settings.cellCodePosition];
        row.name = row.code;

        try
        {
            row.lat = std::stof(properties[settings.latPosition]);

            row.lon = std::stof(properties[settings.lonPosition]);

            row.height = std::stof(properties[settings.heightPosition]);
        }
        catch (const std::exception&)
        {
            std::cerr << "Invalid numeric value in file: " << filename << std::endl;
            return false;
        }

        data.push_back(row);
    }

    file.close();

    return true;
}


bool readCSV(const std::string &filename, const XmlScenarioSettings &settings, std::vector<float> &data)
{
    data.clear();

    std::ifstream file(filename);
    if (! file.is_open())
    {
        std::cerr << "Error opening file: " << filename << std::endl;
        return false;
    }

    // skip header
    std::string line;
    std::getline(file, line);

    int lineNr = 1;
    while (std::getline(file, line))
    {
        ++lineNr;
        if (line.empty())
            continue;

        std::vector<std::string> values;
        std::stringstream ss(line);
        std::string item;
        while (std::getline(ss, item, settings.csvSeparator))
        {
            values.push_back(item);
        }

        if (values.size() <= settings.anomalyPosition)
        {
            std::cerr << "missing properties in file: " << filename << std::endl;
            return false;
        }

        float value;
        try
        {
            value = std::stof(values[settings.anomalyPosition]);
        }
        catch (const std::exception&)
        {
            std::cerr << "Invalid numeric value at line " << lineNr
                      << " in file: " << filename << std::endl;
            return false;
        }

        data.push_back(value);
    }

    file.close();
    return true;
}


void writeXML(const std::string& filename, const XmlScenarioSettings &settings,
              std::vector<std::vector<float>>& dataTmin,
              std::vector<std::vector<float>>& dataTmax,
              std::vector<std::vector<float>>& dataPrec3M,
              std::vector<std::vector<float>>& dataWetDaysFrequency,
              int cell, std::vector<DataProperties> &properties)
{
    // properties
    std::string point_name = properties[cell].name;
    std::string point_code = properties[cell].code;
    std::string point_info = "";
    float point_lat = properties[cell].lat;
    float point_lon = properties[cell].lon;
    float point_height = properties[cell].height;

    std::string models_type = "";
    std::string models_value = settings.model.toStdString();

    std::string climate_type = "";
    int climate_from = settings.climateYear1;
    int climate_to = settings.climateYear2;

    std::string scenario_type = settings.scenario.toStdString();
    int scenario_from = settings.scenarioYear1;
    int scenario_to = settings.scenarioYear2;

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
        {"DJF", "", "", {{"Tmin", "anomaly", dataTmin[0][cell]}, {"Tmax", "anomaly", dataTmax[0][cell]}, {"Prec3M", "anomaly",dataPrec3M[0][cell]}, {"WetDaysFrequency", "anomaly", dataWetDaysFrequency[0][cell]}}},
        {"MAM", "", "", {{"Tmin", "anomaly", dataTmin[1][cell]}, {"Tmax", "anomaly", dataTmax[1][cell]}, {"Prec3M", "anomaly",dataPrec3M[1][cell]}, {"WetDaysFrequency", "anomaly", dataWetDaysFrequency[1][cell]}}},
        {"JJA", "", "", {{"Tmin", "anomaly", dataTmin[2][cell]}, {"Tmax", "anomaly", dataTmax[2][cell]}, {"Prec3M", "anomaly",dataPrec3M[2][cell]}, {"WetDaysFrequency", "anomaly", dataWetDaysFrequency[2][cell]}}},
        {"SON", "", "", {{"Tmin", "anomaly", dataTmin[3][cell]}, {"Tmax", "anomaly", dataTmax[3][cell]}, {"Prec3M", "anomaly",dataPrec3M[3][cell]}, {"WetDaysFrequency", "anomaly", dataWetDaysFrequency[3][cell]}}}
    };

    std::ofstream file(filename);
    if (! file.is_open()) {
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


std::string generateFilename(const std::string& path, const std::string& code)
{
    std::ostringstream oss;
    oss << path << "GRD_" << code << ".xml";
    return oss.str();
}

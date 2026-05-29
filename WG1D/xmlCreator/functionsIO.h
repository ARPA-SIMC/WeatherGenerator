#ifndef FUNCTIONSIO_H
#define FUNCTIONSIO_H


#include <vector>
#include <string>
#include "xmlProject.h"

struct DataRow {
    int col1;
    float col2;
    float col3;
    float col4;
    float col5;
};

struct DataProperties {
    std::string code;
    std::string name;
    float lat, lon, height;
};


struct DataVariable {
    float Tmax;
    float Tmin;
    float Prec3M;
    float WetDaysFrequency;
};


bool readPropertiesCSV(const std::string &filename, const XmlScenarioSettings &settings,
                       std::vector<DataProperties> &data);

bool readCSV(const std::string &filename, const XmlScenarioSettings &settings,
             std::vector<float> &data);

void writeXML(const std::string& filename, const XmlScenarioSettings &settings,
              std::vector<std::vector<float>>& dataTmin,
              std::vector<std::vector<float>>& dataTmax,
              std::vector<std::vector<float>>& dataPrec3M,
              std::vector<std::vector<float>>& dataWetDaysFrequency,
              int cell, std::vector<DataProperties> &properties);

std::string generateFilename(const std::string& path, const std::string &code);


#endif // FUNCTIONSIO_H

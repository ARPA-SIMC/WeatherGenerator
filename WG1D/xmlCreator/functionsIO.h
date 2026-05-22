#ifndef FUNCTIONSIO_H
#define FUNCTIONSIO_H


#include <vector>
#include <string>

struct DataRow {
    int col1;
    float col2;
    float col3;
    float col4;
    float col5;
};

struct DataAnagraphic {
    int code;
    std::string name;
    float lat,lon,height;
};


struct DataVariable {
    float Tmax;
    float Tmin;
    float Prec3M;
    float WetDaysFrequency;
};

std::string generateFilename(const std::string& path, int number);
void writeXML(const std::string& filename, std::vector<std::vector<DataRow>>& dataTmin,
              std::vector<std::vector<DataRow>>& dataTmax, std::vector<std::vector<DataRow>>& dataPrec3M,
              std::vector<std::vector<DataRow>>& dataWetDaysFrequency,
              int cell, std::vector<std::string>& variables, std::vector<DataAnagraphic> &data);
void extractVariables(const std::string &filename, std::vector<std::string> &variables);
void readCSV(const std::string &filename, std::vector<DataRow> &data);
void readAnagraficaCSV(const std::string &filename, std::vector<DataAnagraphic> &data);
void writeCsvForTest(const std::string &filename, std::vector<DataAnagraphic> &data, float anomaly);

#endif // FUNCTIONSIO_H

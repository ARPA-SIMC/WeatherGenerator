#include <QCoreApplication>
#include <QDir>
#include <QString>

#include <iostream>
#include <vector>
#include <iomanip>
#include <sstream>
#include <string>
#include <filesystem>
#include <algorithm>

#include "functionsIO.h"
#include "xmlProject.h"
#include "utilities.h"


// uncomment to execute test
#define TEST


void usage()
{
    std::cout << std::endl << "Usage:" << std::endl
              << "xmlCreator <projectName.ini>" << std::endl;
    std::cout << std::flush;
}


int main(int argc, char *argv[])
{
    QCoreApplication myApp(argc, argv);

    std::cout << "XML scenarios creator  V1.0\n";

    XmlProject myProject;

    QString dataPath, settingsFileName;
    if (! searchDataPath(&dataPath))
        return -1;

    #ifdef TEST
        settingsFileName = dataPath + "TEST_xmlCreator/testXmlCreator.ini";
    #else
        if (argc > 1)
            settingsFileName = argv[1];
        else
        {
            usage();
            return 0;
        }
    #endif

    // read settings
    if (! myProject.readSettings(settingsFileName))
        return -1;


    QDir inputDir(myProject.xmlSettings.inputPath);

    if (! inputDir.exists())
    {
        myProject.logger.writeError("inputDir directory does not exist: "
                          + myProject.xmlSettings.inputPath);
        return false;
    }

    // take only: *.csv
    QStringList fileFilters;
    fileFilters << "*" + myProject.xmlSettings.suffix;

    QFileInfoList fileList = inputDir.entryInfoList(fileFilters,
                                                    QDir::Files | QDir::NoSymLinks,
                                                    QDir::Name);

    QStringList filenameTmax(4);
    QStringList filenameTmin(4);
    QStringList filenamePrec3M(4);
    QStringList filenameWetDaysFrequency(4);
    int nrKeyFiles = 0;

    int maxPosition = std::max({myProject.xmlSettings.seasonPosition,
                                myProject.xmlSettings.varPosition,
                                myProject.xmlSettings.modelPosition,
                                myProject.xmlSettings.scenarioPosition});

    // scan fileNames
    for (const QFileInfo& fileInfo : fileList)
    {
        QString fileName = fileInfo.fileName();

        // remove suffix
        fileName.chop(myProject.xmlSettings.suffix.length());

        // split using '_'
        QStringList elements = fileName.split("_", Qt::SkipEmptyParts);

        myProject.logger.writeInfo("File = " + fileInfo.fileName());

        // check filename
        if (elements.size() <= maxPosition)
        {
            myProject.logger.writeError("Wrong filename format: " + fileInfo.fileName());
            return false;
        }

        // MODEL
        const QString model = elements[myProject.xmlSettings.modelPosition].toUpper();
        const QString scenario = elements[myProject.xmlSettings.scenarioPosition].toUpper();
        if (myProject.xmlSettings.model.isEmpty())
        {
            myProject.xmlSettings.model = model;
            myProject.xmlSettings.scenario = scenario;
        }
        else if (myProject.xmlSettings.model != model)
        {
            myProject.logger.writeError("Model is different from previous: " + model);
            return false;
        }
        else if (myProject.xmlSettings.scenario != scenario)
        {
            myProject.logger.writeError("Scenario is different from previous: " + scenario);
            return false;
        }

        // SEASON
        const QString season = elements[myProject.xmlSettings.seasonPosition].toUpper();

        int seasonIndex = -1;
        if (season == "DJF")
            seasonIndex = 0;
        else if (season == "MAM")
            seasonIndex = 1;
        else if (season == "JJA")
            seasonIndex = 2;
        else if (season == "SON")
            seasonIndex = 3;
        else
        {
            myProject.logger.writeError("Unknown season: " + season);
            return false;
        }

        // VARIABLE
        const QString variable = elements[myProject.xmlSettings.varPosition].toUpper();

        if (variable == "TMIN")
        {
            filenameTmin[seasonIndex] = fileInfo.filePath();
            ++nrKeyFiles;
        }
        else if (variable == "TMAX")
        {
            filenameTmax[seasonIndex] = fileInfo.filePath();
            ++nrKeyFiles;
        }
        else if (variable == "PREC")
        {
            filenamePrec3M[seasonIndex] = fileInfo.filePath();
            ++nrKeyFiles;
        }
        else if (variable == "FWET")
        {
            filenameWetDaysFrequency[seasonIndex] = fileInfo.filePath();
        }
        else
        {
            myProject.logger.writeError("Unknown Variable: " + variable);
            return false;
        }
    }

    // check on key variables
    if (nrKeyFiles < 12)
    {
        QString missingNr = QString::number(12 - nrKeyFiles);
        myProject.logger.writeError("Missing " + missingNr + " key variables (TMIN, TMAX or PREC)");
        return false;
    }

    // read first file as anagrafica
    std::vector<DataProperties> dataProperties;
    if (! readPropertiesCSV(filenameTmin[0].toStdString(), myProject.xmlSettings, dataProperties))
    {
        myProject.logger.writeError("Wrong file format in readPropertiesCSV: " + filenameTmin[0]);
        return false;
    }

    const int nrCells = dataProperties.size();
    myProject.logger.writeInfo("Nr of Cells = " + QString::number(nrCells));

    // read anomaly
    std::vector<std::vector<float>> dataTmax(4);
    std::vector<std::vector<float>> dataTmin(4);
    std::vector<std::vector<float>> dataPrec3M(4);
    std::vector<std::vector<float>> dataWetDaysFrequency(4);

    for(int i=0; i < 4; i++)
    {
        readCSV(filenameTmin[i].toStdString(), myProject.xmlSettings, dataTmin[i]);
        if (dataTmin[i].size() != nrCells)
        {
            myProject.logger.writeError("Missing data in file: " + filenameTmin[i]);
            return false;
        }

        readCSV(filenameTmax[i].toStdString(), myProject.xmlSettings, dataTmax[i]);
        if (dataTmax[i].size() != nrCells)
        {
            myProject.logger.writeError("Missing data in file: " + filenameTmax[i]);
            return false;
        }

        readCSV(filenamePrec3M[i].toStdString(), myProject.xmlSettings, dataPrec3M[i]);
        if (dataPrec3M[i].size() != nrCells)
        {
            myProject.logger.writeError("Missing data in file: " + filenamePrec3M[i]);
            myProject.logger.writeInfo ("Nr of data: " + QString::number(dataPrec3M[i].size()));
            return false;
        }

        if (! filenameWetDaysFrequency[i].isEmpty())
        {
            readCSV(filenameWetDaysFrequency[i].toStdString(), myProject.xmlSettings, dataWetDaysFrequency[i]);
        }
        else
        {
            dataWetDaysFrequency[i].resize(nrCells, 0.0);
        }
    }

    // todo generateFilename
    //writeXML()
    for (int cell = 0; cell < nrCells;cell++)
    {
        std::string filenameXML;
        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(4) << cell;
        const std::string stringCode = oss.str();
        std::string path = inputDir.absolutePath().toStdString();
        filenameXML = generateFilename(path,stringCode);
        writeXML(filenameXML,myProject.xmlSettings,dataTmin,dataTmax,dataPrec3M,dataWetDaysFrequency,cell,dataProperties);
        std::cout << filenameXML << '\n';
    }


    return true;
}


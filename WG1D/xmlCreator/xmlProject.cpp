#include "xmlProject.h"

#include <QSettings>
#include <QDir>


XmlScenarioSettings::XmlScenarioSettings()
{
    settingsFileName = "";
    inputPath = "";
    outputPath = "";

    suffix = ".csv";
    csvSeparator = ',';

    meteoVarList.clear();

    climateYear1 = 2000;
    climateYear2 = 2000;
    scenarioYear1 = 2000;
    scenarioYear2 = 2000;

    // info position in the filename (start from zero)
    varPosition = -1;
    seasonPosition = -1;
    modelPosition = -1;

    // data position in the csv file (start from zero)
    cellCodePosition = 0;
    latPosition = -1;
    lonPosition = -1;
    heightPosition = -1;
    changePosition = -1;
}


XmlProject::XmlProject()
{}


bool XmlProject::readSettings(const QString &settingsFileName)
{
    xmlSettings.settingsFileName = settingsFileName;

    QFile myFile(settingsFileName);
    if (! myFile.exists())
    {
        logger.writeError("settings file doesn't exist: " + settingsFileName);
        return false;
    }

    QFileInfo fileInfo(settingsFileName);
    QString currentPath = fileInfo.path() + "/";

    QSettings* mySettings = new QSettings(settingsFileName, QSettings::IniFormat);
    mySettings->beginGroup("settings");

    // INPUT path
    xmlSettings.inputPath = mySettings->value("input").toString();
    if (xmlSettings.inputPath.left(1) == ".")
    {
        xmlSettings.inputPath = QDir::cleanPath(currentPath + xmlSettings.inputPath);
    }

    if (xmlSettings.inputPath.isEmpty())
    {
        logger.writeError("Missing input path in file .ini");
        return false;
    }
    QDir inputDirectory(xmlSettings.inputPath);
    if (! inputDirectory.exists())
    {
        if (! inputDirectory.mkdir(xmlSettings.inputPath))
        {
            logger.writeError("Error creating directory: " + xmlSettings.inputPath);
            return false;
        }
    }

    logger.writeInfo("Input path = " + xmlSettings.inputPath);

    // OUTPUT path
    xmlSettings.outputPath = mySettings->value("output").toString();
    if (xmlSettings.outputPath.left(1) == ".")
    {
        xmlSettings.outputPath = QDir::cleanPath(currentPath + xmlSettings.outputPath);
    }

    if (xmlSettings.outputPath == "")
    {
        logger.writeError("Missing output path in file .ini");
        return false;
    }
    // check  directory
    QDir outputDirectory(xmlSettings.outputPath);
    if (! outputDirectory.exists())
    {
        if (! outputDirectory.mkdir(xmlSettings.outputPath))
        {
            logger.writeError("Error creating directory: " + xmlSettings.outputPath);
            return false;
        }
    }

    logger.writeInfo("Output path = " + xmlSettings.outputPath);


    QString mySeparator = mySettings->value("separator").toString();
    xmlSettings.csvSeparator = mySeparator.toStdString().c_str()[0];


    return true;
}


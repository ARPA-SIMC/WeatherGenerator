#include "xmlProject.h"

#include <QSettings>
#include <QDir>
#include <QSet>


QSet<QString> validVariablesSet = {
    "TMIN",
    "TMAX",
    "PREC",
    "FWET"
};

XmlScenarioSettings::XmlScenarioSettings()
{
    settingsFileName = "";
    inputPath = "";
    outputPath = "";
    model = "";

    // default: comma separated values
    suffix = ".csv";
    csvSeparator = ',';

    climateYear1 = 2000;
    climateYear2 = 2000;
    scenarioYear1 = 2000;
    scenarioYear2 = 2000;

    // info position in the filename (start from zero)
    varPosition = -1;
    seasonPosition = -1;
    modelPosition = -1;
    scenarioPosition = -1;

    // data position in the csv file (start from zero)
    cellCodePosition = 0;
    latPosition = -1;
    lonPosition = -1;
    heightPosition = -1;
    anomalyPosition = -1;
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
    QString currentPath = fileInfo.path();

    QSettings mySettings(settingsFileName, QSettings::IniFormat);
    mySettings.beginGroup("settings");

    // INPUT path
    xmlSettings.inputPath = mySettings.value("input").toString();
    if (QDir::isRelativePath(xmlSettings.inputPath))
    {
        xmlSettings.inputPath = QDir::cleanPath(currentPath + "/" + xmlSettings.inputPath);
    }

    if (xmlSettings.inputPath.isEmpty())
    {
        logger.writeError("Missing input path in file .ini");
        return false;
    }
    QDir inputDirectory(xmlSettings.inputPath);
    if (! inputDirectory.exists())
    {
        if (! QDir().mkpath(xmlSettings.inputPath))
        {
            logger.writeError("Error creating directory: " + xmlSettings.inputPath);
            return false;
        }
    }

    logger.writeInfo("Input path = " + xmlSettings.inputPath);

    // OUTPUT path
    xmlSettings.outputPath = mySettings.value("output").toString();
    if (QDir::isRelativePath(xmlSettings.outputPath))
    {
        xmlSettings.outputPath = QDir::cleanPath(currentPath + "/" + xmlSettings.outputPath);
    }

    if (xmlSettings.outputPath.isEmpty())
    {
        logger.writeError("Missing output path in file .ini");
        return false;
    }
    // check  directory
    QDir outputDirectory(xmlSettings.outputPath);
    if (! outputDirectory.exists())
    {
        if (!QDir().mkpath(xmlSettings.outputPath))
        {
            logger.writeError("Error creating directory: " + xmlSettings.outputPath);
            return false;
        }
    }

    logger.writeInfo("Output path = " + xmlSettings.outputPath);

    QString mySeparator = mySettings.value("separator").toString();
    if (! mySeparator.isEmpty())
        xmlSettings.csvSeparator = mySeparator.at(0).toLatin1();

    QString mySuffix = mySettings.value("suffix").toString();
    if (! mySuffix.isEmpty())
        xmlSettings.suffix = mySuffix;

    // climate period
    bool isNumberOk;
    QString yearStr = mySettings.value("climateYear1").toString();
    if (yearStr.isEmpty())
    {
        logger.writeError("Missing climateYear1 in file .ini");
        return false;
    }
    xmlSettings.climateYear1 = yearStr.toInt(&isNumberOk);
    if (! isNumberOk)
    {
        logger.writeError("Wrong climateYear1 in file .ini");
        return false;
    }

    yearStr = mySettings.value("climateYear2").toString();
    if (yearStr.isEmpty())
    {
        logger.writeError("Missing climateYear2 in file .ini");
        return false;
    }
    xmlSettings.climateYear2 = yearStr.toInt(&isNumberOk);
    if (! isNumberOk)
    {
        logger.writeError("Wrong climateYear2 in file .ini");
        return false;
    }

    // check
    if (xmlSettings.climateYear1 > xmlSettings.climateYear2)
    {
        logger.writeError("climateYear1 must be <= climateYear2");
        return false;
    }

    logger.writeInfo("CLIMATE period = " + QString::number(xmlSettings.climateYear1) + "-"
                     + QString::number(xmlSettings.climateYear2));

    // scenarios period
    yearStr = mySettings.value("scenarioYear1").toString();
    if (yearStr.isEmpty())
    {
        logger.writeError("Missing scenarioYear1 in file .ini");
        return false;
    }
    xmlSettings.scenarioYear1 = yearStr.toInt(&isNumberOk);
    if (! isNumberOk)
    {
        logger.writeError("Wrong scenarioYear1 in file .ini");
        return false;
    }

    yearStr = mySettings.value("scenarioYear2").toString();
    if (yearStr.isEmpty())
    {
        logger.writeError("Missing scenarioYear2 in file .ini");
        return false;
    }
    xmlSettings.scenarioYear2 = yearStr.toInt(&isNumberOk);
    if (! isNumberOk)
    {
        logger.writeError("Wrong scenarioYear2 in file .ini");
        return false;
    }

    // check
    if (xmlSettings.scenarioYear1 > xmlSettings.scenarioYear2)
    {
        logger.writeError("scenarioYear1 must be <= scenarioYear2");
        return false;
    }

    logger.writeInfo("SCENARIOS period = " + QString::number(xmlSettings.scenarioYear1) + "-"
                     + QString::number(xmlSettings.scenarioYear2));

    // variables
    xmlSettings.variableList = mySettings.value("variables").toStringList();
    if (xmlSettings.variableList.isEmpty())
    {
        logger.writeError("Missing variables in file .ini");
        return false;
    }

    for (const QString& newVar : xmlSettings.variableList)
    {
        if (! validVariablesSet.contains(newVar))
        {
            logger.writeError("Wrong variable: " + newVar);
            return false;
        }
    }

    logger.writeInfo("Variables = " + xmlSettings.variableList.join(","));

    // info position in the filename (start from zero)
    xmlSettings.varPosition = mySettings.value("varPosition").toInt(&isNumberOk);
    if (! isNumberOk)
    {
        logger.writeError("Wrong varPosition in file .ini");
        return false;
    }
    xmlSettings.seasonPosition = mySettings.value("seasonPosition").toInt(&isNumberOk);
    if (! isNumberOk)
    {
        logger.writeError("Wrong seasonPosition in file .ini");
        return false;
    }
    xmlSettings.modelPosition = mySettings.value("modelPosition").toInt(&isNumberOk);
    if (! isNumberOk)
    {
        logger.writeError("Wrong modelPosition in file .ini");
        return false;
    }
    xmlSettings.scenarioPosition = mySettings.value("scenarioPosition").toInt(&isNumberOk);
    if (! isNumberOk)
    {
        logger.writeError("Wrong scenarioPosition in file .ini");
        return false;
    }

    // check
    if (xmlSettings.varPosition < 0 || xmlSettings.seasonPosition < 0 ||
        xmlSettings.modelPosition < 0 || xmlSettings.scenarioPosition < 0)
    {
        logger.writeError("missing information: varPosition or seasonPosition "
                          "or modelPosition or scenarioPosition.");
        return false;
    }

    // data position in the csv file (start from zero)
    xmlSettings.cellCodePosition = mySettings.value("cellCodePosition").toInt(&isNumberOk);
    if (! isNumberOk)
    {
        logger.writeError("Wrong cellCodePosition in file .ini");
        return false;
    }

    if (xmlSettings.cellCodePosition < 0)
    {
        logger.writeError("cellCodePosition must be >= 0");
        return false;
    }

    xmlSettings.latPosition = mySettings.value("latPosition").toInt(&isNumberOk);
    if (! isNumberOk)
    {
        logger.writeError("Wrong latPosition in file .ini");
        return false;
    }
    xmlSettings.lonPosition = mySettings.value("lonPosition").toInt(&isNumberOk);
    if (! isNumberOk)
    {
        logger.writeError("Wrong lonPosition in file .ini");
        return false;
    }
    xmlSettings.heightPosition = mySettings.value("heightPosition").toInt(&isNumberOk);
    if (! isNumberOk)
    {
        logger.writeError("Wrong heightPosition in file .ini");
        return false;
    }
    xmlSettings.anomalyPosition = mySettings.value("anomalyPosition").toInt(&isNumberOk);
    if (! isNumberOk)
    {
        logger.writeError("Wrong anomalyPosition in file .ini");
        return false;
    }

    return true;
}


#ifndef XMLPROJECT_H
#define XMLPROJECT_H

#include <QString>
#include "logger.h"



class XmlScenarioSettings
{
public:
    XmlScenarioSettings();

    QString settingsFileName;
    QString inputPath;
    QString outputPath;

    QString suffix;
    char csvSeparator;

    QList<QString> variableList;

    // periods
    int climateYear1, climateYear2;
    int scenarioYear1, scenarioYear2;

    // info position in the filename (start from zero)
    int varPosition;
    int seasonPosition;
    int modelPosition;

    // data position in the csv file (start from zero)
    int cellCodePosition = 0;
    int latPosition, lonPosition;
    int heightPosition;
    int anomalyPosition;
};


class XmlProject
{
public:
    XmlProject();

    XmlScenarioSettings xmlSettings;
    Logger logger;

    bool readSettings(const QString &settingsFileName);
};





#endif // XMLPROJECT_H

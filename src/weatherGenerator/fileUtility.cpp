#include <QDebug>
#include <QFile>
#include <QTextStream>

#include "commonConstants.h"
#include "crit3dDate.h"
#include "weatherGenerator.h"
#include "fileUtility.h"


bool readMeteoDataCsv (const QString &fileName, char mySeparator, double noData, TinputObsData &inputData)
{
    clearInputData(inputData);

    QFile file(fileName);
    if (! file.open(QIODevice::ReadOnly))
    {
        qDebug() << "\nERROR!\n" << fileName << file.errorString();
        return false;
    }

    QList<QString> listDate;
    QList<QString> listTMin;
    QList<QString> listTMax;
    QList<QString> listPrecip;

    int indexLine = 0;
    int indexDate = 0;
    Crit3DDate tmpDate;

    QString noDataString = QString::number(noData);
    QString currentDateStr;

    // header
    file.readLine();

    while (! file.atEnd())
    {
        QByteArray line = file.readLine();

        // split
        QList<QByteArray> valueList = line.split(mySeparator);

        // check format
        if (valueList.count() < 4)
        {
            qDebug() << "ERROR!" << "\nfile =" << fileName << "\nline =" << indexLine+2;;
            qDebug() << "missing data or invalid format or invalid separator";
            qDebug() << "required separator = " << mySeparator <<"\n";
            return false;
        }

        int precPosition = 3;
        // presence of tavg
        if (valueList.count() > 4)
            precPosition = 4;

        // date
        currentDateStr = valueList[0];

        //check presence of quotation
        if (currentDateStr.left(1) == "\"")
        {
            currentDateStr = currentDateStr.mid(1, currentDateStr.length()-2);
        }
        listDate.append(currentDateStr);

        // save the first date into the struct and check it is a valid date
        if (indexLine == 0)
        {
            inputData.inputFirstDate.year = listDate[indexLine].mid(0,4).toInt();
            if (inputData.inputFirstDate.year == 0)
            {
                qDebug() << "Invalid date format ";
                return false;
            }
            inputData.inputFirstDate.month = listDate[indexLine].mid(5,2).toInt();
            if (inputData.inputFirstDate.month == 0 || inputData.inputFirstDate.month > 12 )
            {
                qDebug() << "Invalid date format ";
                return false;
            }
            inputData.inputFirstDate.day = listDate[indexLine].mid(8,2).toInt();
            if (inputData.inputFirstDate.day == 0 || inputData.inputFirstDate.day > 31)
            {
                qDebug() << "Invalid date format ";
                return false;
            }
        }
        else
        {
            tmpDate.year = listDate[indexLine].mid(0,4).toInt();
            tmpDate.month = listDate[indexLine].mid(5,2).toInt();
            tmpDate.day = listDate[indexLine].mid(8,2).toInt();

            indexDate = difference(inputData.inputFirstDate, tmpDate);

            // check LACK of data
            if (indexDate != indexLine)
            {
                // insert nodata row
                listDate.removeLast();
                for (int i = indexLine; i < indexDate ; i++)
                {
                    listDate.append(noDataString);
                    listTMin.append(noDataString);
                    listTMax.append(noDataString);
                    listPrecip.append(noDataString);
                    indexLine++;
                }
                listDate.append(currentDateStr);
            }
        }

        // tmin
        if (valueList[1] == "" || valueList[1] == " " || valueList[1] == noDataString )
            listTMin.append(QString::number(NODATA));
        else
            listTMin.append(line.split(mySeparator)[1]);

        // tmax
        if (valueList[2] == "" || valueList[2] == " " || valueList[2] == noDataString)
            listTMax.append(QString::number(NODATA));
        else
            listTMax.append(valueList[2]);

        // prec
        QString precString = valueList[precPosition];
        if (precString == "" || precString == " " || precString == noDataString)
            listPrecip.append(QString::number(NODATA));
        else
            listPrecip.append(precString);

        indexLine++;
    }

    file.close();

    // save and check the last date
    inputData.inputLastDate = tmpDate;
    if (inputData.inputLastDate.year == 0)
    {
        qDebug() << "Invalid date format ";
        return false;
    }
    if (inputData.inputLastDate.month == 0 || inputData.inputLastDate.month > 12 )
    {
        qDebug() << "Invalid date format ";
        return false;
    }
    if (inputData.inputLastDate.day == 0 || inputData.inputLastDate.day > 31)
    {
        qDebug() << "Invalid date format ";
        return false;
    }

    if (listDate.length() != listTMin.length() || (listDate.length()!= listTMax.length() ) || (listDate.length() != listPrecip.length()) )
    {
        qDebug() << "list data - different size";
        return false;
    }

    inputData.dataLength = listDate.length();
    inputData.inputTMin.resize(inputData.dataLength);
    inputData.inputTMax.resize(inputData.dataLength);
    inputData.inputPrecip.resize(inputData.dataLength);

    for (int i = 0; i < inputData.dataLength; i++)
    {
        inputData.inputTMin[i] = listTMin[i].toFloat();
        inputData.inputTMax[i] = listTMax[i].toFloat();
        inputData.inputPrecip[i] = listPrecip[i].toFloat();

        // check tmin <= tmax
        if ((inputData.inputTMin[i] != NODATA) && (inputData.inputTMax[i] != NODATA)
             && (inputData.inputTMin[i] > inputData.inputTMax[i]))
        {
            // switch
            inputData.inputTMin[i] = listTMax[i].toFloat();
            inputData.inputTMax[i] = listTMin[i].toFloat();
        }
    }

    listTMax.clear();
    listTMin.clear();
    listPrecip.clear();

    return true;
}


// Write the output of weather generator: a daily series of tmin, tmax, prec data
bool writeMeteoDataCsv(const QString &fileName, char separator, std::vector<ToutputDailyMeteo> &dailyData, bool isWaterTable)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qDebug() << file.errorString();
        return false;
    }

    QTextStream stream( &file );

    if (isWaterTable)
    {
        stream << "date" << separator << "tmin" << separator << "tmax" << separator << "prec" << separator << "watertable \n";

        for (unsigned int i=0; i < dailyData.size(); i++)
        {
            if (dailyData[i].date == NO_DATE)
                break;

            QString yearStr = QString::number(dailyData[i].date.year);
            QString monthStr = QString::number(dailyData[i].date.month).rightJustified(2, '0');
            QString dayStr = QString::number(dailyData[i].date.day).rightJustified(2, '0');
            QString dateStr = yearStr + "-" + monthStr + "-" + dayStr;

            QString tMin = QString::number(double(dailyData[i].minTemp), 'f', 1);
            QString tMax = QString::number(double(dailyData[i].maxTemp), 'f', 1);
            QString prec = QString::number(double(dailyData[i].prec), 'f', 1);

            QString waterTableDepth = QString::number(double(dailyData[i].waterTableDepth), 'f', 2);
            if (dailyData[i].waterTableDepth == NODATA)
            {
                waterTableDepth = QString::number(NODATA);
            }

            stream << dateStr << separator << tMin << separator << tMax << separator << prec << separator << waterTableDepth << "\n";
        }
    }
    else
    {
        stream << "date" << separator << "tmin" << separator << "tmax" << separator << "prec\n";

        for (unsigned int i=0; i < dailyData.size(); i++)
        {
            if (dailyData[i].date == NO_DATE)
                break;

            QString year = QString::number(dailyData[i].date.year);
            QString month = QString::number(dailyData[i].date.month).rightJustified(2, '0');
            QString day = QString::number(dailyData[i].date.day).rightJustified(2, '0');
            QString myDate = year + "-" + month + "-" + day;

            QString tMin = QString::number(double(dailyData[i].minTemp), 'f', 1);
            QString tMax = QString::number(double(dailyData[i].maxTemp), 'f', 1);
            QString prec = QString::number(double(dailyData[i].prec), 'f', 1);

            stream << myDate << separator << tMin << separator << tMax << separator << prec << "\n";
        }
    }

    return true;
}


#ifndef DBMETEOCRITERIA1D_H
#define DBMETEOCRITERIA1D_H

#ifndef MAX_MISSING_TOT_DAYS
    #define MAX_MISSING_TOT_DAYS 30
#endif
#ifndef MAX_MISSING_CONSECUTIVE_DAYS_T
    #define MAX_MISSING_CONSECUTIVE_DAYS_T 1
#endif
#ifndef MAX_MISSING_CONSECUTIVE_DAYS_PREC
    #define MAX_MISSING_CONSECUTIVE_DAYS_PREC 7
#endif

    #include <QStringList>
    class QSqlDatabase;
    class QSqlQuery;
    class QDate;
    class Crit3DMeteoPoint;

    bool openDbMeteo(QString dbName, QSqlDatabase* dbMeteo, QString* error);
    bool getMeteoPointList(QSqlDatabase* dbMeteo, QStringList* idMeteoList, QString* error);
    bool getYearList(QSqlDatabase* dbMeteo, QString table, QStringList* yearList, QString *error);
    bool getLatLonFromIdMeteo(QSqlDatabase* dbMeteo, QString idMeteo, QString* lat, QString* lon, QString *error);
    bool updateLatLonFromIdMeteo(QSqlDatabase* dbMeteo, QString idMeteo, QString lat, QString lon, QString *error);
    bool updateLatFromIdMeteo(QSqlDatabase* dbMeteo, QString idMeteo, QString lat, QString *error);
    QString getTableNameFromIdMeteo(QSqlDatabase* dbMeteo, QString idMeteo, QString *error);

    bool checkYear(QSqlDatabase* dbMeteo, QString table, QString year, QString *error);
    bool checkYearMeteoGridFixedFields(QSqlDatabase dbMeteo, QString tableD, QString fieldTime, QString fieldTmin, QString fieldTmax, QString fieldPrec, QString year, QString *error);
    bool checkYearMeteoGrid(QSqlDatabase dbMeteo, QString tableD, QString fieldTime, int varCodeTmin, int varCodeTmax, int varCodePrec, QString year, QString *error);

    bool getLastDate(QSqlDatabase* dbMeteo, QString table, QString year, QDate* date, QString *error);
    bool getLastDateGrid(QSqlDatabase dbMeteo, QString table, QString fieldTime, QString year, QDate* date, QString *error);

    bool fillDailyTempPrecCriteria1D(QSqlDatabase* dbMeteo, QString table, Crit3DMeteoPoint *meteoPoint, QString validYear, QString *error);
    bool readDailyDataCriteria1D(QSqlQuery *query, Crit3DMeteoPoint *meteoPoint, QString *myError);


#endif // DBMETEOCRITERIA1D_H

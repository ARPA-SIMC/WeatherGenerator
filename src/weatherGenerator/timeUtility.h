#ifndef TIMEUTILITY
#define TIMEUTILITY

    #define NRDAYSTOLERANCE  31
    #define USEDATA false

    class Crit3DDate;
    class QString;
    class QDate;
    struct TinputObsData;

    int getMonthsInPeriod(int month1, int month2);

    bool getDoyFromSeason(const QString &season, int predictionYear, int &wgDoy1, int &wgDoy2);

    bool checkLastYearDate(TinputObsData *dailyObsData, int predictionYear, int &wgDoy1, int &nrDaysBeforeWGDay1);

    void setCorrectWgDoy(int wgDoy1, int wgDoy2, int predictionYear, int myYear, int &fixedWgDoy1, int &fixedWgDoy2);

    void setAnomalyMonthScenario(QString startingSeason, int *anomalyMonth1, int *anomalyMonth2);


#endif // TIMEUTILITY

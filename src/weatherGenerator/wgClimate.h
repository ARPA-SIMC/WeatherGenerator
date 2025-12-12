#ifndef WGCLIMATE_H
#define WGCLIMATE_H

    class Crit3DDate;
    struct TinputObsData;
    struct TweatherGenClimate;

    #include <QString>
    #include <vector>

    bool climateGenerator(int nrDays, TinputObsData climateDailyObsData, Crit3DDate climateDateIni,
                          Crit3DDate climateDateFin, float precThreshold, float minDataPercentage,
                          TweatherGenClimate* wGen, bool isWriteOutput = false, const QString &outputFileName = QString(""));

    float sampleStdDeviation(float values[], int nElement);

    bool computeWGClimate(int nrDays, Crit3DDate inputFirstDate, const std::vector<float>& inputTMin,
                          const std::vector<float>& inputTMax, const std::vector<float>& inputPrec,
                          float precThreshold, float minDataPercentage, TweatherGenClimate* wGen,
                          bool isWriteOutput,  const QString &outputFileName);

    bool computeWG2DClimate(int nrDays, Crit3DDate inputFirstDate, float *inputTMin, float *inputTMax,
                          float *inputPrec, float precThreshold, float minPrecData,
                          TweatherGenClimate* wGen, bool writeOutput, bool outputForStats,
                          QString outputFileName, float* monthlyPrecipitation,
                          float** consecutiveDry, float** consecutiveWet, int nrConsecutiveDryDaysBins);

    bool computeWG2DClimate(int nrDays, Crit3DDate inputFirstDate, float *inputTMin, float *inputTMax,
                          float *inputPrec, float precThreshold, float minPrecData,
                          TweatherGenClimate* wGen, bool writeOutput, bool outputForStats,
                          QString outputFileName, float* monthlyPrecipitation);

    float computeNormalizedDryDayLength(std::vector<float> &probabiltyConsecutiveDays);
    void averagedVectorProbability(std::vector<float>& probabiltyConsecutiveDays);
    float avgProbabilityVectorTail(std::vector<float>& probabiltyConsecutiveDays);

#endif // WGCLIMATE_H


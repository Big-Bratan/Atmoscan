#ifndef AIR_QUALITY_H
#define AIR_QUALITY_H

#include <Arduino.h>

class AirQuality {
public:
    enum Quality {
        EXCELLENT,
        GOOD,
        MODERATE,
        POOR,
        VERY_POOR
    };

    static Quality getCO2Quality(int ppm);
    static Quality getPM25Quality(float pm25);
    static Quality getHumidityQuality(float humidity);
    static int getQualityScore(int co2, float pm25, float humidity);
    static const char* getQualityText(Quality quality);
    static void generateProgressBar(char* buffer, int score);

private:
    static int mapCO2ToScore(int ppm);
    static int mapPM25ToScore(float pm25);
    static int mapHumidityToScore(float humidity);
};

#endif 
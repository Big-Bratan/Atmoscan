#include "AirQuality.h"

AirQuality::Quality AirQuality::getCO2Quality(int ppm) {
    if (ppm <= 800) return EXCELLENT;
    if (ppm <= 1000) return GOOD;
    if (ppm <= 1500) return MODERATE;
    if (ppm <= 2000) return POOR;
    return VERY_POOR;
}

AirQuality::Quality AirQuality::getPM25Quality(float pm25) {
    if (pm25 <= 12.0) return EXCELLENT;
    if (pm25 <= 35.4) return GOOD;
    if (pm25 <= 55.4) return MODERATE;
    if (pm25 <= 150.4) return POOR;
    return VERY_POOR;
}

AirQuality::Quality AirQuality::getHumidityQuality(float humidity) {
    if (humidity >= 40 && humidity <= 60) return EXCELLENT;
    if (humidity >= 30 && humidity <= 70) return GOOD;
    return POOR;
}

int AirQuality::mapCO2ToScore(int ppm) {
    if (ppm <= 800) return map(ppm, 0, 800, 100, 80);
    if (ppm <= 1000) return map(ppm, 801, 1000, 79, 60);
    if (ppm <= 1500) return map(ppm, 1001, 1500, 59, 40);
    if (ppm <= 2000) return map(ppm, 1501, 2000, 39, 20);
    return map(min(ppm, 3000), 2001, 3000, 19, 0);
}

int AirQuality::mapPM25ToScore(float pm25) {
    if (pm25 <= 12.0) return map((int)(pm25 * 10), 0, 120, 100, 80);
    if (pm25 <= 35.4) return map((int)(pm25 * 10), 121, 354, 79, 60);
    if (pm25 <= 55.4) return map((int)(pm25 * 10), 355, 554, 59, 40);
    if (pm25 <= 150.4) return map((int)(pm25 * 10), 555, 1504, 39, 20);
    return map(min((int)(pm25 * 10), 2000), 1505, 2000, 19, 0);
}

int AirQuality::mapHumidityToScore(float humidity) {
    if (humidity >= 40 && humidity <= 60) return 100;
    if (humidity >= 30 && humidity <= 70) return 75;
    return 50;
}

int AirQuality::getQualityScore(int co2, float pm25, float humidity) {
    int co2Score = mapCO2ToScore(co2);
    int pm25Score = mapPM25ToScore(pm25);
    int humScore = mapHumidityToScore(humidity);
    
    // Moyenne pondérée : CO2 (40%), PM2.5 (40%), Humidité (20%)
    return (co2Score * 40 + pm25Score * 40 + humScore * 20) / 100;
}

const char* AirQuality::getQualityText(Quality quality) {
    switch (quality) {
        case EXCELLENT: return "Excellent";
        case GOOD: return "Bon";
        case MODERATE: return "Modere";
        case POOR: return "Mauvais";
        case VERY_POOR: return "Tres mauvais";
        default: return "Inconnu";
    }
}

void AirQuality::generateProgressBar(char* buffer, int score) {
    strcpy(buffer, "[");
    int blocks = map(score, 0, 100, 0, 10);
    
    for (int i = 0; i < 10; i++) {
        if (i < blocks) {
            strcat(buffer, "\xFF");  // Caractère plein pour l'écran OLED
        } else {
            strcat(buffer, " ");
        }
    }
    
    strcat(buffer, "] ");
    char scoreStr[5];
    sprintf(scoreStr, "%d%%", score);
    strcat(buffer, scoreStr);
} 
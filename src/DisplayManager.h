#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include "Font.h"
#include "AirQuality.h"

class DisplayManager {
public:
    DisplayManager(uint8_t address);
    void begin();
    void clearDisplay();
    void writeString(const char* str, uint8_t x, uint8_t page);
    void displayDot(bool state);
    void displayData(int co2, float temp, float hum, bool heating);
    void displayImage(const uint8_t* image);
    void drawProgressBar(uint8_t x, uint8_t page, int percentage);

private:
    void writeChar(char c, uint8_t x, uint8_t page);
    void initOLED();
    
    uint8_t _address;
    uint8_t _displayBuffer[128]; // DISPLAY_WIDTH
    static const uint8_t DISPLAY_WIDTH = 128;
    static const uint8_t DISPLAY_HEIGHT = 64;
    static const uint8_t DISPLAY_PAGES = DISPLAY_HEIGHT / 8;
};

#endif 
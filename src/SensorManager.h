#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "MHZ19.h"
#include <dht.h>

class SensorManager {
public:
    SensorManager(uint8_t co2Rx, uint8_t co2Tx, uint8_t dhtPin);
    void begin();
    int getCO2();
    float getTemperature();
    float getHumidity();
    bool isHeating();

private:
    SoftwareSerial _co2Serial;
    MHZ19 _mhz19;
    dht _dht;
    uint8_t _dhtPin;
};

#endif 
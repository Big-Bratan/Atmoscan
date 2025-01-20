#include "SensorManager.h"

SensorManager::SensorManager(uint8_t co2Rx, uint8_t co2Tx, uint8_t dhtPin) 
    : _co2Serial(co2Rx, co2Tx), _dhtPin(dhtPin) {}

void SensorManager::begin() {
    _co2Serial.begin(9600);
    _mhz19.begin(_co2Serial);
}

int SensorManager::getCO2() {
    return _mhz19.getCO2();
}

float SensorManager::getTemperature() {
    _dht.read11(_dhtPin);
    return _dht.temperature;
}

float SensorManager::getHumidity() {
    _dht.read11(_dhtPin);
    return _dht.humidity;
}

bool SensorManager::isHeating() {
    return (getCO2() == 0);
} 
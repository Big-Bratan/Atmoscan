#include <Wire.h>
#include <SoftwareSerial.h>
#include "MHZ19.h"
#include <dht.h>
#include "DisplayManager.h"
#include "Logo.h"

// Configuration I2C
#define SCREEN_ADDRESS 0x3C

// DHT11 sensor
dht DHT;

#define DHT11_PIN 7

// Configuration MHZ19
SoftwareSerial co2Serial(12, 13);
MHZ19 myMHZ19;

// Create display manager
DisplayManager display(SCREEN_ADDRESS);

void setup() {
    Serial.begin(9600);
    delay(100);
    Serial.println(F("Starting..."));
    
    // Initialisation écran
    display.begin();
    
    // Afficher le logo au démarrage
    display.displayImage(LOGO);
    delay(3000);  // Afficher pendant 3 secondes
    display.clearDisplay();
    
    // Afficher un message simple
    display.writeString("ATMOSCAN", 25, 2);
    display.writeString("Loading...", 25, 4);
    delay(2000);
    display.clearDisplay();
    
    Serial.println(F("Display initialized"));
    
    // Initialisation MHZ19
    co2Serial.begin(9600);
    myMHZ19.begin(co2Serial);
    myMHZ19.autoCalibration(false);  // Désactive l'auto-calibration
    Serial.println(F("MHZ19 ready"));
    
    // Affichage initial
    display.displayData(0, 0, 0, false);
}

void loop() {
    static unsigned long lastRead = 0;
    
    if (millis() - lastRead >= 2000) {
        int co2 = myMHZ19.getCO2();
        
        // Lecture DHT11
        int chk = DHT.read11(DHT11_PIN);
        float temp = DHT.temperature;
        float hum = DHT.humidity;
        
        // Vérifier si le capteur est en chauffe (CO2 = 0 pendant la chauffe)
        // bool isHeating = (co2 == 0);
        bool isHeating = myMHZ19.getPWMStatus();

        Serial.print(F("CO2: "));
        Serial.print(co2);
        Serial.print(F(" ppm, Temp: "));
        Serial.print(temp);
        Serial.print(F("C, Hum: "));
        Serial.print(hum);
        Serial.println(F("%"));
        if (isHeating) {
            Serial.println(F("MHZ19 is heating..."));
        }
        
        display.displayData(co2, temp, hum, isHeating);
        
        lastRead = millis();
    }
    
    delay(50);
}

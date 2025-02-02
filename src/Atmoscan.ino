#include <Wire.h>
#include <SoftwareSerial.h>
#include "MHZ19.h"
#include <dht.h>
#include <SPI.h>
#include <SD.h>
#include "Logo.h"
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

// Group related constants together
#define SD_CS_PIN 10
#define DHT11_PIN 7
#define I2C_ADDRESS 0x3C
#define SDS011_RX 8
#define SDS011_TX 9
#define REAL_DISPLAY_WIDTH 132
#define DISPLAY_WIDTH 128
#define BAR_WIDTH 96
#define PROGRESS_BAR_PAGE 7

// Timing constants
const unsigned long SAVE_INTERVAL = 900000;  // 15 minutes
const unsigned long SENSOR_SWITCH_INTERVAL = 5000;  // 5 seconds
const unsigned long DISPLAY_UPDATE_INTERVAL = 1000;  // 1 second

// Sensor objects
SoftwareSerial sensorSerial(4, 5);  // CO2 sensor serial
MHZ19 myMHZ19;
dht DHT;
SSD1306AsciiWire oled;

// SDS011 protocol constants
const uint8_t SDS011_QUERY_DATA[] = {
    0xAA, 0xB4, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
    0xFF, 0x02, 0xAB
};

// Sensor state variables
struct PMData {
    float pm25;
    float pm10;
};

static bool readingCO2 = true;
static float pm25 = 0;
static float pm10 = 0;
static float lastValidPM25 = 0;
static float lastValidPM10 = 0;

// Display cache variables
static int lastCO2 = -1;
static int lastTemp = -999;
static int lastHum = -999;
static float lastDisplayedPM25 = -999;
static int lastAirQuality = -1;

// Calculates air quality index (0-100)
int calculateAirQuality(int co2, int temp, int humidity, int pm25, int pm10) {
    int quality = 100;

    // CO2 (ppm): <800 excellent, 800-1000 good, 1000-1500 average, >1500 bad
    if(co2 > 1500) quality -= 30;
    else if(co2 > 1000) quality -= 15;
    else if(co2 > 800) quality -= 7;

    // PM2.5 (µg/m³): <12 excellent, 12-35 good, 35-55 average, >55 bad
    if(pm25 > 55) quality -= 30;
    else if(pm25 > 35) quality -= 15;
    else if(pm25 > 12) quality -= 7;

    // PM10 (µg/m³): <55 excellent, 56-155 good, 156-255 average, >255 bad
    if(pm10 > 255) quality -= 30;
    else if(pm10 > 155) quality -= 15;
    else if(pm10 > 55) quality -= 7;

    // Temperature (°C): 20-25 excellent, 18-20 ou 25-28 good, <18 ou >28 bad
    if(temp < 15 || temp > 26) quality -= 20;
    else if(temp < 20 || temp > 25) quality -= 10;

    // Humidity (%): 40-60 excellent, 30-40 ou 60-70 good, <30 ou >70 bad
    if(humidity < 30 || humidity > 70) quality -= 20;
    else if(humidity < 40 || humidity > 60) quality -= 10;

    return max(0, quality);
}

// Draws a progress bar with percentage on display
void drawProgressBar(SSD1306AsciiWire& display, int percentage) {
  uint8_t displayBuffer[REAL_DISPLAY_WIDTH] = {0};
  uint8_t pos = 5;  // Start position

  // Draw '['
  displayBuffer[pos] = 0xFF;
  displayBuffer[pos + 1] = 0x80;
  pos += 2;

  // Draw bar blocks
  int blocks = map(percentage, 0, 100, 0, BAR_WIDTH - 10);
  for (int i = 0; i < (BAR_WIDTH - 10); i++) {
    displayBuffer[pos + i] = (i < blocks) ? 0xFF : 0x00;
  }
  pos += BAR_WIDTH - 10;

  // Draw ']'
  displayBuffer[pos] = 0x80;
  displayBuffer[pos + 1] = 0xFF;
  pos += 2;

  display.setCol(0);
  display.setRow(PROGRESS_BAR_PAGE);

  Wire.beginTransmission(I2C_ADDRESS);
  Wire.write(0x00);
  Wire.write(0xB0 | PROGRESS_BAR_PAGE);
  Wire.write(0x00);
  Wire.write(0x10);
  Wire.endTransmission();

  for (uint8_t i = 0; i < REAL_DISPLAY_WIDTH; i += 16) {
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(0x40);
    for (uint8_t j = 0; j < 16 && (i + j) < REAL_DISPLAY_WIDTH; j++) {
      Wire.write(displayBuffer[i + j]);
    }
    Wire.endTransmission();
  }

  // Display percentage
  display.setCol(BAR_WIDTH + 5);
  display.setRow(PROGRESS_BAR_PAGE);
  display.print(percentage);
  display.print(F("%"));
}

// Clears a specific row on display
void clearLine(SSD1306AsciiWire& display, uint8_t row) {
  display.setRow(row);
  display.setCol(0);

  Wire.beginTransmission(I2C_ADDRESS);
  Wire.write(0x00);
  Wire.write(0xB0 | row);
  Wire.write(0x00);
  Wire.write(0x10);
  Wire.endTransmission();

  for (uint8_t i = 0; i < REAL_DISPLAY_WIDTH; i += 16) {
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(0x40);
    for (uint8_t j = 0; j < 16 && (i + j) < REAL_DISPLAY_WIDTH; j++) {
      Wire.write(0x00);
    }
    Wire.endTransmission();
  }
}

// Reads PM2.5 and PM10 values from SDS011 sensor
PMData readPM25() {
  PMData result = {lastValidPM25, lastValidPM10};  // Default Values

  sensorSerial.end();  // End CO2 serial
  sensorSerial = SoftwareSerial(SDS011_RX, SDS011_TX);  // Reassign pins
  sensorSerial.begin(9600);

  // Send query command
  for (int i = 0; i < sizeof(SDS011_QUERY_DATA); i++) {
    sensorSerial.write(SDS011_QUERY_DATA[i]);
  }

  delay(100);

  uint8_t buffer[10];
  int index = 0;
  while (sensorSerial.available() > 0 && index < 10) {
    buffer[index++] = sensorSerial.read();
  }

  if (index == 10) {
    if (buffer[0] == 0xAA && buffer[1] == 0xC0 && buffer[9] == 0xAB) {
      float newPM25 = ((buffer[3] * 256 + buffer[2]) / 10.0) * 0.2;
      float newPM10 = ((buffer[5] * 256 + buffer[4]) / 10.0) * 0.2;

      lastValidPM25 = newPM25;
      lastValidPM10 = newPM10;

      result = {newPM25, newPM10};
    }
  }

  return result;
}

void clearScreenRegion(uint8_t col, uint8_t row, uint8_t width) {
    oled.setCol(col);
    oled.setRow(row);
    for (uint8_t i = oled.col(); i < min(REAL_DISPLAY_WIDTH, col + width); i++) {
        oled.ssd1306WriteRam(0);
    }
}

void switchToCO2Mode() {
    sensorSerial.end();
    sensorSerial = SoftwareSerial(4, 5);
    sensorSerial.begin(9600);
    myMHZ19.begin(sensorSerial);
}

void writeToSD(unsigned long currentMillis, int co2, float temp, float hum, float pm25, float pm10) {
    File dataFile = SD.open("data.csv", FILE_WRITE);
    if (dataFile) {
        dataFile.print(currentMillis);
        dataFile.print(F(","));
        dataFile.print(co2);
        dataFile.print(F(","));
        dataFile.print(temp);
        dataFile.print(F(","));
        dataFile.print(hum);
        dataFile.print(F(","));
        dataFile.print(pm25);
        dataFile.print(F(","));
        dataFile.println(pm10);
        dataFile.close();
        Serial.println(F("Data saved"));
    } else {
        Serial.println(F("Error opening data.csv for writing"));
    }
}

void clearScreen() {
  oled.clear();
  oled.setFont(System5x7);
  oled.setCursor(0,0);
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Wire.setClock(400000L);

  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(System5x7);
  oled.clear();

  // Display Logo
  for (uint8_t i = 0; i < 8; i++) {
    oled.setRow(i);
    oled.setCol(0);
    for (uint8_t j = 0; j < REAL_DISPLAY_WIDTH; j++) {
      if (j < 128) {
        oled.ssd1306WriteRam(pgm_read_byte(&LOGO[i * 128 + j]));
      } else {
        oled.ssd1306WriteRam(0);
      }
    }
  }
  delay(2000);

  clearScreen();

  // Display name
  oled.setFont(Callibri15);
  oled.setCol((DISPLAY_WIDTH - (8 * 8)) / 2);
  oled.setRow(2);
  oled.println(F("ATMOSCAN"));

  oled.setFont(System5x7);
  oled.setCol(25);
  oled.setRow(5);
  oled.println(F("Loading..."));
  delay(2000);

  clearScreen();

  oled.setFont(System5x7);
  oled.setCol(5);
  oled.setRow(1);
  oled.println(F("CO2:     ppm"));
  oled.setCol(5);
  oled.setRow(3);
  oled.println(F("Temp:    C  Hum:    %"));
  oled.setCol(5);
  oled.setRow(5);
  oled.println(F("PM2.5:    ug/m3"));

  // Init MHZ19
  sensorSerial.begin(9600);
  myMHZ19.begin(sensorSerial);
  myMHZ19.autoCalibration(false);

  // Init SD
  Serial.println(F("Initializing SD card..."));
  pinMode(SD_CS_PIN, OUTPUT);  // Make sure SS pin is output

  if (!SD.begin(SD_CS_PIN)) {
    Serial.println(F("SD card initialization failed!"));
    Serial.println(F("* is a card inserted?"));
    Serial.println(F("* is your wiring correct?"));
    Serial.println(F("* is the chipSelect pin correct?"));
    while (1);  // Don't proceed
  }
  Serial.println(F("SD card initialization done."));

  // Try to create/open file
  Serial.println(F("Trying to open data.csv..."));
  File dataFile = SD.open("data.csv", FILE_WRITE);
  if (dataFile) {
    Serial.println(F("File opened successfully"));
    if (dataFile.size() == 0) {
      Serial.println(F("New file - writing header"));
      dataFile.println(F("Time,CO2,Temperature,Humidity,PM2.5,PM10"));
    } else {
      Serial.println(F("Existing file - writing session separator"));
      dataFile.println(F(""));
      dataFile.println(F("----new session----"));
      dataFile.println(F("Time,CO2,Temperature,Humidity,PM2.5,PM10"));
    }
    dataFile.close();
    Serial.println(F("File closed successfully"));
  } else {
    Serial.println(F("Error opening data.csv"));
    Serial.println(F("Trying to create a new file..."));

    // Try to create file
    dataFile = SD.open("data.csv", FILE_WRITE);
    if (dataFile) {
      Serial.println(F("New file created successfully"));
      dataFile.println(F("Time,CO2,Temperature,Humidity,PM2.5,PM10"));
      dataFile.close();
      Serial.println(F("Header written and file closed"));
    } else {
        Serial.println(F("Error creating new file"));
        Serial.println(F("SD card might be write-protected or damaged"));
        while(1);  // Don't proceed if we can't write to SD
    }
  }
}

void loop() {
  static unsigned long lastSave = 0;
  static unsigned long lastDisplay = 0;
  static unsigned long lastSensorSwitch = 0;
  unsigned long currentMillis = millis();

  // Switch between sensors every 5 seconds
  if (currentMillis - lastSensorSwitch >= SENSOR_SWITCH_INTERVAL) {
    if (!readingCO2) {
      PMData pmData = readPM25();
      pm25 = pmData.pm25;
      pm10 = pmData.pm10;
      switchToCO2Mode();
    }
    readingCO2 = !readingCO2;
    lastSensorSwitch = currentMillis;
  }

  // Read PM2.5 more frequently when it's its turn
  if (!readingCO2 && (currentMillis - lastDisplay >= DISPLAY_UPDATE_INTERVAL)) {
    PMData pmData = readPM25();
    pm25 = pmData.pm25;
    pm10 = pmData.pm10;
    switchToCO2Mode();
  }

  if (currentMillis - lastDisplay >= DISPLAY_UPDATE_INTERVAL) {
    int co2 = myMHZ19.getCO2();
    DHT.read11(DHT11_PIN);
    int temp = (int)DHT.temperature;
    int hum = (int)DHT.humidity;

    // Update CO2 display if changed
    if (co2 != lastCO2) {
      clearScreenRegion(5 + 5*6, 1, REAL_DISPLAY_WIDTH - (5 + 5*6));
      oled.setCol(5 + 5*6);
      oled.print(co2);
      oled.print(F(" ppm"));
      lastCO2 = co2;
    }

    // Update temperature & humidity if changed
    if (temp != lastTemp || hum != lastHum) {
      clearLine(oled, 3);

      oled.setCol(5);
      oled.setRow(3);
      oled.print(F("Temp: "));
      oled.print(temp);
      oled.print(F("C  Hum: "));
      oled.print(hum);
      oled.print(F("%"));

      lastTemp = temp;
      lastHum = hum;
    }

    // Update PM2.5 display if changed
    if (abs(pm25 - lastDisplayedPM25) > 0.1) {
      clearScreenRegion(5 + 7*6, 5, REAL_DISPLAY_WIDTH - (5 + 7*6));
      oled.setCol(5 + 7*6);
      oled.print(pm25, 1);
      oled.print(F(" ug/m3"));
      lastDisplayedPM25 = pm25;
    }

    // Update air quality if changed
    int airQuality = calculateAirQuality(co2, temp, hum, (int)pm25, (int)pm10);
    if (airQuality != lastAirQuality) {
      drawProgressBar(oled, airQuality);
      lastAirQuality = airQuality;
    }

    lastDisplay = currentMillis;
  }

  if (currentMillis - lastSave >= SAVE_INTERVAL) {
    switchToCO2Mode();
    int co2 = myMHZ19.getCO2();
    DHT.read11(DHT11_PIN);

    if (co2 > 0 && co2 < 5000) {
      writeToSD(currentMillis, co2, DHT.temperature, DHT.humidity, pm25, pm10);
    }
    lastSave = currentMillis;
  }
}

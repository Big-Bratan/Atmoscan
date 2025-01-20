#include "DisplayManager.h"
#include "Font.h"
#include "AirQuality.h"

DisplayManager::DisplayManager(uint8_t address) : _address(address) {}

void DisplayManager::begin() {
    initOLED();
    clearDisplay();
}

void DisplayManager::writeChar(char c, uint8_t x, uint8_t page) {
    if (c >= 'a' && c <= 'z') {
        c = c - 'a' + 'A';
    }
    
    if (c < ' ' || c > 'Z') return;
    if (x < 2) x = 2;
    
    for (uint8_t i = 0; i < 5; i++) {
        _displayBuffer[x + i] = pgm_read_byte(&font5x7[(c - ' ') * 5 + i]);
    }
    _displayBuffer[x + 5] = 0x00;
}

void DisplayManager::writeString(const char* str, uint8_t x, uint8_t page) {
    memset(_displayBuffer, 0, DISPLAY_WIDTH);
    
    uint8_t pos = x + 2;
    while (*str && pos < (DISPLAY_WIDTH - 8)) {
        writeChar(*str++, pos, page);
        pos += 7;
    }
    
    Wire.beginTransmission(_address);
    Wire.write(0x00);
    Wire.write(0xB0 | page);
    Wire.write(0x00);
    Wire.write(0x10);
    Wire.endTransmission();
    
    for (uint8_t i = 0; i < DISPLAY_WIDTH; i += 16) {
        Wire.beginTransmission(_address);
        Wire.write(0x40);
        for (uint8_t j = 0; j < 16; j++) {
            Wire.write(_displayBuffer[i + j]);
        }
        Wire.endTransmission();
    }
}

void DisplayManager::clearDisplay() {
    memset(_displayBuffer, 0, DISPLAY_WIDTH);
    
    for (uint8_t page = 0; page < DISPLAY_PAGES; page++) {
        Wire.beginTransmission(_address);
        Wire.write(0x00);
        Wire.write(0xB0 | page);
        Wire.write(0x00);
        Wire.write(0x10);
        Wire.endTransmission();
        
        for (uint8_t i = 0; i < DISPLAY_WIDTH; i += 16) {
            Wire.beginTransmission(_address);
            Wire.write(0x40);
            for (uint8_t j = 0; j < 16; j++) {
                Wire.write(0x00);
            }
            Wire.endTransmission();
        }
    }
}

void DisplayManager::displayDot(bool state) {
    Wire.beginTransmission(_address);
    Wire.write(0x00);
    Wire.write(0xB0);  // Page 0
    Wire.write(0x7C);  // Position x=124 (ajusté pour le cercle 5x5)
    Wire.write(0x17);
    Wire.endTransmission();
    
    if (state) {
        Wire.beginTransmission(_address);
        Wire.write(0x40);
        Wire.write(0b00111000);  // ..***..
        Wire.write(0b01111100);  // .*****
        Wire.write(0b01111100);  // .*****
        Wire.write(0b01111100);  // .*****
        Wire.write(0b00111000);  // ..***..
        Wire.endTransmission();
    } else {
        Wire.beginTransmission(_address);
        Wire.write(0x40);
        Wire.write(0x00);
        Wire.write(0x00);
        Wire.write(0x00);
        Wire.write(0x00);
        Wire.write(0x00);
        Wire.endTransmission();
    }
}

void DisplayManager::displayData(int co2, float temp, float hum, bool heating) {
    char buffer[32];
    
    // Ligne 1: CO2
    sprintf(buffer, "CO2: %d PPM", co2);
    writeString(buffer, 0, 0);
    
    // Ligne 2: Température et Humidité sur la même ligne
    sprintf(buffer, "T:%dC  H:%d%%", (int)temp, (int)hum);
    writeString(buffer, 0, 2);
    
    // Ligne 3: PM2.5 (valeur en dur pour l'instant)
    sprintf(buffer, "PM2.5: 10 ug/m3");
    writeString(buffer, 0, 4);
    
    // Ligne 4: Barre de progression
    int score = AirQuality::getQualityScore(co2, 10.0, hum);
    drawProgressBar(0, 6, score);
    
    // Afficher le cercle si en chauffe
    displayDot(heating);
}

void DisplayManager::initOLED() {
    Wire.begin();
    Wire.setClock(400000);  // 400kHz
    delay(100);
    
    Wire.beginTransmission(_address);
    Wire.write(0x00);    // Command mode
    Wire.write(0xAE);    // Display off
    Wire.write(0xD5);    // Set display clock
    Wire.write(0x80);    // Clock ratio
    Wire.write(0xA8);    // Multiplex ratio
    Wire.write(0x3F);    // 1/64 Duty
    Wire.write(0xD3);    // Display offset
    Wire.write(0x00);    // No offset
    Wire.write(0x40);    // Start line = 0
    Wire.write(0x8D);    // Charge pump
    Wire.write(0x14);    // Enable charge pump
    Wire.write(0x20);    // Memory mode
    Wire.write(0x00);    // Horizontal addressing
    Wire.write(0xA1);    // Segment remap (inversé)
    Wire.write(0xC8);    // COM scan direction (inversé)
    Wire.write(0xDA);    // COM pins
    Wire.write(0x12);    // COM pins config
    Wire.write(0x81);    // Contrast
    Wire.write(0xCF);    // Contrast value
    Wire.write(0xD9);    // Pre-charge period
    Wire.write(0xF1);    // Pre-charge
    Wire.write(0xDB);    // VCOMH deselect
    Wire.write(0x20);    // VCOMH
    Wire.write(0xA4);    // Display resume
    Wire.write(0xA6);    // Normal display
    Wire.write(0x2E);    // Deactivate scroll
    Wire.write(0xAF);    // Display on
    Wire.endTransmission();
    
    delay(100);
}

void DisplayManager::displayImage(const uint8_t* image) {
    for (uint8_t page = 0; page < DISPLAY_PAGES; page++) {
        Wire.beginTransmission(_address);
        Wire.write(0x00);
        Wire.write(0xB0 | page);
        Wire.write(0x00);
        Wire.write(0x10);
        Wire.endTransmission();
        
        for (uint8_t i = 0; i < DISPLAY_WIDTH; i += 16) {
            Wire.beginTransmission(_address);
            Wire.write(0x40);
            for (uint8_t j = 0; j < 16; j++) {
                Wire.write(pgm_read_byte(&image[page * DISPLAY_WIDTH + i + j]));
            }
            Wire.endTransmission();
        }
    }
}

void DisplayManager::drawProgressBar(uint8_t x, uint8_t page, int percentage) {
    memset(_displayBuffer, 0, DISPLAY_WIDTH);
    
    // Position initiale pour '['
    uint8_t pos = x + 2;
    
    // Dessiner '[' en pixels pleins
    _displayBuffer[pos] = 0xFF;     // Barre verticale
    _displayBuffer[pos + 1] = 0x80; // Coin supérieur
    pos += 2;
    
    // Dessiner les blocs de la barre
    int blocks = map(percentage, 0, 100, 0, 48);
    for (int i = 0; i < 48; i++) {
        _displayBuffer[pos + i] = (i < blocks) ? 0xFF : 0x00;
    }
    pos += 48;
    
    // Dessiner ']' en pixels pleins
    _displayBuffer[pos] = 0x80;     // Coin supérieur
    _displayBuffer[pos + 1] = 0xFF; // Barre verticale
    pos += 3;
    
    // Ajouter le pourcentage directement dans le buffer
    char buffer[5];
    sprintf(buffer, " %d%%", percentage);
    const char* ptr = buffer;
    while (*ptr) {
        for (uint8_t i = 0; i < 5; i++) {
            _displayBuffer[pos + i] = pgm_read_byte(&font5x7[(*ptr - ' ') * 5 + i]);
        }
        _displayBuffer[pos + 5] = 0x00;
        ptr++;
        pos += 7;
    }
    
    // Envoyer tout à l'écran en une fois
    Wire.beginTransmission(_address);
    Wire.write(0x00);
    Wire.write(0xB0 | page);
    Wire.write(0x00);
    Wire.write(0x10);
    Wire.endTransmission();
    
    for (uint8_t i = 0; i < DISPLAY_WIDTH; i += 16) {
        Wire.beginTransmission(_address);
        Wire.write(0x40);
        for (uint8_t j = 0; j < 16; j++) {
            Wire.write(_displayBuffer[i + j]);
        }
        Wire.endTransmission();
    }
}
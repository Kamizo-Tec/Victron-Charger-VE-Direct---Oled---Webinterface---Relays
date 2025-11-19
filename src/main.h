#pragma once

#define ARDUINOJSON_USE_DOUBLE 0
#define ARDUINOJSON_USE_LONG_LONG 0

#define VICTRON_BAUD 19200   // Baudrate für VE.Direct
#define MYPORT_TX 15 // D8   read pin
#define MYPORT_RX 13 // D7  write pin , not used 

#define LED_PIN 02           // D4 LED auf Wemos D1 Mini

#define DEBUG_BAUD 115200
#define JSON_BUFFER 1024  

#define FlashSize ESP.getFreeSketchSpace()

#define SOFTWARE_VERSION SWVERSION

// Debug-Ausgabe über Serial
//#define DEBUG_PRINT(...) Serial.print(__VA_ARGS__)
//#define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
//#define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)

// Funktionsdeklarationen
bool getJsonData();
void notifyClients();
void prozessData();


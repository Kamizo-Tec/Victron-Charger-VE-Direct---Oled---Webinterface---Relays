
// KAMIZO TEC VICTRON VE_DIRECT V_1.0_ 07/2025

/* ESP 8266 Pins to use 

1. Sensoren
DS18B20 (Temperatur, OneWire) → D4 / GPIO4

2. Taster / Schalter
GPIO12 / D5, GPIO14 / D6 → für manuelle Eingaben oder Resetfunktionen

GPIO0 / D3 Flash-Pin beim ESP8266. boot, Flash-Modus / Normalbetrieb

3. Relais / Aktoren
PCF EXPANDER / NO LIBRARY / Direct codet 

4. LEDs / Statusanzeigen
für Status-LEDs (z. B. Betrieb, Fehler, WLAN)

5. I2C-Erweiterungen
OLED nutzt meist GPI O4 (SDA) und GPIO5 (SCL) → ( RTC, ADC, IO-Expander)

6. PWM-Ausgabe
Für z. B. Lüftersteuerung, LED-Dimmung → GPIO12 / D6, GPIO13 / D7 , GPIO14 / D5 /RX

GPIO10
Nicht empfohlen für normale Nutzung //boot flash
*/

// LOAD OUTPUT Lastausgang schalten falls vorhanden: --------------------------

 // veDirect.print(":LOADOFF\r"); // \r ist CR (Carriage Return)
//  Serial.println("Schalte LOAD OFF...");
 
 // veDirect.print(":LOADON\r");
  // Serial.println("Schalte LOAD ON...");

  // to use GO TO --- VICTRON SETTINGS :   LOAD OUTPUT : Manuell or User


#include <Arduino.h>
#include "main.h"

#include <ArduinoJson.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266WiFiMulti.h>
#include <SoftwareSerial.h>

#include "VeDirectFrameHandler.h"
#include "VeDirectDataList.h"
#include "VeDirectDeviceList.h"
#include "html.h"

#include <LittleFS.h>

//temperature
#include <OneWire.h>
#include <DallasTemperature.h>

// DISPLAY ----------------------------------------------------------
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define OLED_ADDR 0x3C  // typische SH1106-Adresse

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
unsigned long lastDisplayUpdate = 0; // Loop display 

// FREE PINS 
// D0  // NO interrupt 

// D1 // SCL OLED DISPLAY // PCF 8574 / EXPANDER 4 CH 
// D2 // SDA OLED DISPLAY // PCF8574 /  EXPANDER 4 CH

//  D7 / 
//------------------------------------------------------------------------

bool displayOn = false;
bool lastButtonState = HIGH;
unsigned long displayStartTime = 0;
const unsigned long DISPLAY_TIMEOUT = 240 * 60 * 1000; // 60 Minuten
// SDA D2 GPIO 4 // SCL D1 // gpio 5
#define DISPLAY_BUTTON_PIN D6 // // button display on / off input pullup
//---------------------------------------------------------------------------

#define ONE_WIRE_BUS D5  
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

float tempSensor_victron = 25.0;
float tempSensor_battery = 25.0;

DeviceAddress sensorVictron = {0x28, 0x17, 0x90, 0x43, 0xD4, 0xE1, 0x3C, 0x34}; // sensor 1 
DeviceAddress sensorBattery = {0x28, 0xF9, 0x3E, 0x43, 0xD4, 0xE1, 0x3C, 0x9F}; // sensor 0

// Globale Variablen
int mppt = 0; // MPPT CHARGER STATE / bulk float
float Victron_BatteryVoltage = 00.00;
float Victron_BatteryCurrent = 00.00;
float Victron_PanelVoltage = 00.00;
float Victron_PanelPower = 00.00;

float test_PanelVoltage = 12.0;
// Steuerungsflags
//bool shouldSaveConfig = false;
bool restartNow = false;
bool dataProzessing = false;
unsigned long RestartTimer = 0;
uint32_t bootcount = 0;

//  autoMode   ---------------------
bool wsConnected = false;    // WebSocket Client verbunden?
bool autoMode = false;

static bool lastAutoModeSent = !autoMode;
static bool firstRun = true;

bool autoModeTriggeredByPanel = false;

unsigned long panelOverThresholdStart = 0;
unsigned long panelUnderThresholdStart = 0;

// Globale Variablen für WebSocket-Timer-Minuten-Tracking
float lastOnMinutes = -2;
float lastOffMinutes = -2;

int start_inverter = 35;
int stop_inverter = 25;
int start_delay = 5;
int stop_delay = 15;   
int fan_t = 40;  //  fan start temp

//--------------------------------------------------------------

// PIN D7 read / not used D8 write
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
VeDirectFrameHandler myve;
SoftwareSerial veSerial;

// JSON-Datenstruktur
DynamicJsonDocument Json(JSON_BUFFER); // prozessData() // VICTRON 
// ESP separat:
DynamicJsonDocument JsonESP(JSON_BUFFER); // prozessData_ESP() 

// Deklarationen
void prozessData_ESP();
//void sendWS(const char* message);   // Funktionsdeklaration
//---------------------------------------------------------------

//========================================================================
//**********  PCF GLOBAL SETUP FUNKTIONEN UND DEKLARATIONEN  ***************
//============================================================================
#include <Wire.h>
#define PCF_ADDR 0x20
byte pcfState = 0xFF;

// PCF EXPANDER steuerung legende
/* 
Aktiv LOW: Relais switch  LOW Level , also bitClear() = EIN
Wire.write(0b11111101);  // Bit 1 = 0 → P1 = LOW

set(fan, true);       // FAN EIN
 set(fan, false);       // FAN AUS
*/

// PCF Relaisstruktur
struct Relay {
  const char* name;
  uint8_t pin;
  bool state;
};
// Relaisdefinitionen
Relay fan    = {"fan", 1, false}; // FAN Victron
Relay wr     = {"wr", 2, false}; // Wechselrichter INVERTER 
Relay pc     = {"pc", 3, false}; // MiniPc NP93
Relay ssr    = {"ssr", 4, false}; // Solid State Relay DC

//**************************************************************
// VOID PCF RELAYS  //***********************

void set(Relay &r, bool newState) {
  r.state = newState;
  if (newState) {
    bitClear(pcfState, r.pin);  // Relais EIN (aktiv LOW)
  } else {
    bitSet(pcfState, r.pin);    // Relais AUS
  }

  Wire.beginTransmission(PCF_ADDR);
  Wire.write(pcfState);
  Wire.endTransmission();

  Serial.print(r.name);
  Serial.print(" -> PCF EXPANDER -> ");
  Serial.println(newState ? "ON" : "OFF");

  // WebSocket-Nachricht an alle Clients
  ws.textAll(String(r.name) + ":" + (newState ? "on" : "off"));

}
//-------------------------------------------------------
void toggle(Relay &r) {
  set(r, !r.state);
}
//   toggle(fan);     // Schaltet FAN von ON → OFF oder OFF → ON

//------------------------------------------------------------
// Relaiszustand abfragen
bool getState(Relay &r) {
  return r.state;
}
/* 
//  zustand abfragen
if (getState(fan)) {
  Serial.println("FAN ist aktiv");
} else {
  Serial.println("FAN ist aus");
}

//bedingte steuerung 
if (!getState(wr)) {
  set(wr, true);  // WR einschalten, wenn er aus ist
}

// Anzeige im Webinterface oder Display
String status = getState(ssr) ? "ON" : "OFF";
display.print("SSR: " + status);

Du musst nicht direkt auf fan.state zugreifen
Du kannst die Logik zentral über getState() steuern
*/

//*******************************************************************
//**********  PCF FUNKTIONEN UND DEKLARATIONEN  **********************


// Globale Hilfsfunktion: gibt den Textnamen des Betriebszustands (CS) 
const char* getOperationStateName(int mppt) {
  switch (mppt) {
    case 0: return "Off";
    case 2: return "Fault";
    case 3: return "Bulk";
    case 4: return "Absorption";
    case 5: return "Float";
    case 6: return "Storage";
    case 7: return "Equalize";
    case 8: return "Passthrough";
    case 9: return "Inverting";
    default: return "Unknown";
  }
}
// display shortcuts 
const char* getOperationStateShort(int mppt) {
  switch (mppt) {
    case 0: return "OFF";
    case 2: return "FLT";  // Fault
    case 3: return "BLK";  // Bulk
    case 4: return "ABS";  // Absorption
    case 5: return "FLT";  // Float
    case 6: return "STO";  // Storage
    case 7: return "EQL";  // Equalize
    case 8: return "PAS";  // Passthrough
    case 9: return "INV";  // Inverting
    default: return "UNK"; // Unknown
  }
}
//-------MPPT tracker modus-------------------------------

const char* getTrackerModeName(int mode) {
  switch (mode) {
    case 0: return "Off";
    case 1: return "Charging";
    case 2: return "MPPT";
    default: return "Unknown";
  }
}
//-----------------------------------------------------------

const char* getErrorCodeText(int code) {
  switch (code) {
    case 0:  return "No problems";
    case 2:  return "Device overheated";
    case 17: return "Solar charger overheated";
    case 18: return "Input voltage too high";
    case 19: return "PV voltage too high";
    case 20: return "Battery voltage too high";
    case 21: return "Battery voltage too low";
    case 33: return "Communication error";
    case 34: return "Internal error";
    case 38: return "Hardware error";
    default: return "Unknown error";
  }
}

//---------------------------------------
ESP8266WiFiMulti wifiMulti;

void adjustWiFiPower(); 
  
int rssi = WiFi.RSSI(); 
int mW  ; 

const char* ssid1 = "antenna1mp";
const char* password1 = "1234567890";

const char* ssid2 = "antenna2mp";
const char* password2 = "1234567890";

const char* ssid3 = "antenna3mp";
const char* password3 = "1234567890";

const int httpPort = 80;  // HTTP-Port für Webserver

String myssid;  // Aktive SSID
String myip;    // Lokale IP-Adresse

const uint32_t connectTimeoutMs = 20000;  // Timeout für WLAN-Verbindung

// acc
ADC_MODE(ADC_VCC);  // VCC-Messung

// RSSI → Prozent dynamic wifi -----------------------------------------------

int txPower_mW;
int signalPercent; 
int txPower_dBm_int;

int rssiToPercent(int rssi) {
    if (rssi >= -50) return 99;
    if (rssi <= -80) return 0;
    return (int)((rssi + 80) * 100 / 30.0);  // linear zwischen -80..-50 dBm
}

// Hauptfunktion: Status anzeigen & TX anpassen --------------------------

void dynamicWifi() {

   rssi = WiFi.RSSI();

    // Dynamische TX-Leistung 3–100 mW, stufenlos
    const int minRSSI = -80;
    const int maxRSSI = -50;
    const int minPower_mW = 3;
    const int maxPower_mW = 100;

// calculate MW TX------------------------
if (rssi >= maxRSSI) {
  txPower_mW = minPower_mW;
}
else if (rssi <= minRSSI) {
  txPower_mW = maxPower_mW;
}
// else txPower_mW = minPower_mW + (rssi - maxRSSI) * (maxPower_mW - minPower_mW) / (minRSSI - maxRSSI);
else {
  txPower_mW = map(rssi, maxRSSI, minRSSI, minPower_mW, maxPower_mW);
  txPower_mW = constrain(txPower_mW, minPower_mW, maxPower_mW);
}
//------------------------------------------------------------------------
    float txPower_dBm = 10.0 * log10(txPower_mW);  // jetzt korrekt
    txPower_dBm_int = (int)(txPower_dBm + 0.5);    // gerundet

    signalPercent = rssiToPercent(rssi);       
    WiFi.setOutputPower(txPower_dBm); 
     
  }

//-----------------------------------------------
// display sleep and wakeup

void checkDisplayButton() {

  bool buttonState = digitalRead(DISPLAY_BUTTON_PIN);
  delay(50);
  bool buttonStateAfterDelay = digitalRead(DISPLAY_BUTTON_PIN);
  if (buttonState != buttonStateAfterDelay) return;

  if (lastButtonState == HIGH && buttonState == LOW) {
    displayOn = !displayOn;
    if (displayOn) {
      displayStartTime = millis();

  Wire.beginTransmission(OLED_ADDR);
  Wire.write(0x00);
  Wire.write(0xAF); // on 
  Wire.endTransmission();
  ws.textAll("display:on");
    } else {
  Wire.beginTransmission(OLED_ADDR);
  Wire.write(0x00);
  Wire.write(0xAE); // off
  Wire.endTransmission();
  ws.textAll("display:off");
    }
  }
  lastButtonState = buttonState;
}


void checkDisplayTimeout() {
  if (displayOn && (millis() - displayStartTime > DISPLAY_TIMEOUT)) {
    displayOn = false;
  Wire.beginTransmission(OLED_ADDR);
  Wire.write(0x00);
  Wire.write(0xAE);
  Wire.endTransmission();
  ws.textAll("display:off");
    Serial.println("DISPLAY SLEEP MODE TIMER");
  }
}

//---------------------------------------------------------------------
// WebSocket-Message-Handler-Funktion
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  // Beispiel: empfangene Nachricht ausgeben
 
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    // Eingehende Nachricht in einen String umwandeln
    String msg = "";
    for (size_t i = 0; i < len; i++) {
      msg += (char) data[i];
    }

// ALL MESSAGES --------------------------------

if (msg == "wr:toggle") {
  toggle(wr);
}
else if (msg == "wr:on") {
  set(wr, true);
}
else if (msg == "wr:off") {
  set(wr, false);
}
  //------------------------

// PC RELAY schalten
if (msg == "pc:on") {
  set(pc, true);
}
else if (msg == "pc:off") {
  set(pc, false);
}
 //-----------------------------

  // FAN _RELAY schalten mit VOID_SET
if (msg == "fan:toggle") {
  toggle(fan);
}
else if (msg == "fan:on") {
  set(fan, true);
}
else if (msg == "fan:off") {
  set(fan, false);
}
//**********************************************************
// DISPLAY ON OFF
if (msg == "display:toggle" || msg == "display:on" || msg == "display:off") {
  if (msg == "display:toggle") displayOn = !displayOn;
  else displayOn = (msg == "display:on");

  if (displayOn) {
    displayStartTime = millis();
    Wire.beginTransmission(OLED_ADDR);
    Wire.write(0x00);
    Wire.write(0xAF); // Display ON
    Wire.endTransmission();
    ws.textAll("display:on");
  } else {
    Wire.beginTransmission(OLED_ADDR);
    Wire.write(0x00);
    Wire.write(0xAE); // Display OFF
    Wire.endTransmission();
    ws.textAll("display:off");
  }
}

// FAN _RELAY schalten mit VOID_SET
if (msg == "fan:toggle") {
  toggle(fan);
}

else if (msg == "fan:on") {
  set(fan, true);
}

else if (msg == "fan:off") {
  set(fan, false);
}


//------AUTOMODE ------------------------------
if (msg == "autoMode:on") {
  autoMode = true;
}

else if (msg == "autoMode:off") {
  autoMode = false;
}

// ESP RESTART 
 if (msg == "esp:reset") {
  restartNow = true;
  RestartTimer = millis();
}


}

 Serial.printf("WebSocket-Nachricht: %.*s\n", len, data);
}

//-------------------------------------------------------------------
void notifyClients() {
  size_t len = measureJson(Json);
  AsyncWebSocketMessageBuffer *buffer = ws.makeBuffer(len);
  if (buffer) {
    serializeJson(Json, (char *)buffer->get(), len + 1);
    ws.textAll(buffer); // ✅ an alle Clients senden
  }
}
//-----------------------------------------------------------------------
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      if (!dataProzessing) // wenn kein data prozessing gemach wird

    notifyClients();


  // >>> Hier  den DISPLAY Status an den neuen Client senden
  if (displayOn) {
    client->text("display:on");
  } else {
    client->text("display:off");
  }


// >>> Hier den load Relay-Status an den neuen Client senden
  client->text(getState(wr) ? "wr:on" : "wr:off");
  // >>> Hier  den pulse Relay-Status an den neuen Client senden
  client->text(getState(pc) ? "pc:on" : "pc:off");
    // >>> Hier  den FAN Status an den neuen Client senden
  client->text(getState(fan) ? "fan:on" : "fan:off");

// client automode steuerung 
// --- Automodus-Status senden ---
if (autoMode == true) {
  client->text("autoMode:on");
  Serial.println("autoMode:on an neuen Client gesendet");

} else {
  client->text("autoMode:off");

}

// --- Timerstatus beim Verbindungsaufbau ---
// Nur Grundstatus senden (0.0 oder off), 
// die Live-Minuten kommen später durch sendTimerStatus()

if (panelOverThresholdStart > 0) {
  client->text("on_timer_min:0.0");
} else {
  client->text("on_timer_min:off");
}

if (panelUnderThresholdStart > 0) {
  client->text("off_timer_min:0.0");
} else {
  client->text("off_timer_min:off");
}
//---------
      // Flag für Verbindung setzen 
      wsConnected = true;

      break;
  //--------------------------------------------------------------

    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());

      wsConnected = false; // flag verbindung 

      break;

    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;

    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}
//--------------------------------------------------

// PID Variables
char veDeviceId[8] = {0};           // z. B. "0X0300"
char veDeviceName[32] = {0};        // Gerätename aus Liste
unsigned long pidStartTime = 0;  // timer pid extraqct
bool pidTimerStarted = false;
bool pidChecked = false;
bool pidfound = false;

// TIMER esp_data
unsigned long lastProcessDataESP = 0; 
const unsigned long processDataEspInterval = 995;

unsigned long lastVictronData =0;


//*********************************************************************
// littlefs --------------------------------------------------------
//********************************************************
void saveAllToLittleFS() {
  File file = LittleFS.open("/variables.txt", "w");
  if (!file) {
    Serial.println(" Fehler beim Öffnen der Datei zum Schreiben.");
    return;
  }

file.printf(
  "start_inverter=%d\n"
  "stop_inverter=%d\n"
  "start_delay=%d\n"
  "stop_delay=%d\n"
  "fan_t=%d\n"
  "autoMode=%s\n",
  (int)start_inverter,
  (int)stop_inverter,
  (int)start_delay,
  (int)stop_delay,
  (int)fan_t,
  autoMode ? "true" : "false"
);



  file.close();
  Serial.println(" Werte erfolgreich gespeichert.");
}

//-----------------------------------------------
void loadValuesFromLittleFS() {
  Serial.println(" loadValuesFromLittleFS() gestartet...");

  if (LittleFS.exists("/variables.txt")) {
    Serial.println(" variables.txt existiert.");

    File file = LittleFS.open("/variables.txt", "r");
    if (!file) {
      Serial.println(" Fehler beim Öffnen der Datei zum Lesen.");
      return;
    }

    while (file.available()) {
      String line = file.readStringUntil('\n');
      line.trim(); // entfernt \r oder Leerzeichen

if (line.startsWith("start_inverter=")) {
  start_inverter = line.substring(15).toInt(); // korrekt: ab Index 15
  Serial.print("start_inverter = "); Serial.println(start_inverter);
}
else if (line.startsWith("stop_inverter=")) {
  stop_inverter = line.substring(14).toInt(); //  Index 14
  Serial.print("stop_inverter = "); Serial.println(stop_inverter);
}
else if (line.startsWith("start_delay=")) {
  start_delay = line.substring(12).toInt(); // korrekt: ab Index 15
  Serial.print("start_delay = "); Serial.println(start_delay);
}
else if (line.startsWith("stop_delay=")) {
  stop_delay = line.substring(11).toInt(); //  Index 14
  Serial.print("stop_delay = "); Serial.println(stop_delay);
}
else if (line.startsWith("fan_t=")) {
  fan_t = line.substring(6).toInt();
  Serial.print("fan_t = "); Serial.println(fan_t);
}
else if (line.startsWith("autoMode=")) {
  String modeText = line.substring(9);
  modeText.trim();
  autoMode = (modeText == "true");
  Serial.print("autoMode = "); Serial.println(autoMode ? "true" : "false");
}

 }

    file.close();
    Serial.println(" Werte erfolgreich geladen.");
  } 
  else {
    Serial.println(" variables.txt existiert NICHT!");
  }
}//-------------------------------------------------


//********/ READ  VE.Direct-Data VICTRON // ********************************************
  void ReadVEData() {
  bool hasData = false;

  while (veSerial.available()) {
    byte b = veSerial.read();
    myve.rxData(b);
    hasData = true;
    esp_yield();
  } //-----------------------------------------------------------


 // transform PID to Devicename

  // IF Victron send data again and PID was not found → PID-search restart
  //if (hasData && !pidfound && !pidTimerStarted) {
  if (hasData && !pidfound && pidChecked && !pidTimerStarted) {  
  //  Serial.println("Victron sends data PID-search started");
    pidChecked = false;
   // pidTimerStarted = false;
    pidStartTime = millis();
    pidTimerStarted = true;
  }

  //  PID-search activ
  if (!pidChecked && millis() - pidStartTime < 5000) {

   // Serial.print("PID-search active");

    if (myve.frameIndex > 0) {
      for (int i = 0; i < myve.frameIndex; i++) {
        if (myve.tempName[i] && strcasecmp(myve.tempName[i], "PID") == 0) {
          strncpy(veDeviceId, myve.tempValue[i], sizeof(veDeviceId) - 1);
          veDeviceId[sizeof(veDeviceId) - 1] = '\0';

          // Gerätename zuordnen
          for (size_t j = 0; j < sizeof(VeDirectDeviceList) / sizeof(VeDirectDeviceList[0]); j++) {
            char idBuffer[8];
            strcpy_P(idBuffer, (PGM_P)VeDirectDeviceList[j][0]);

            if (strcasecmp(veDeviceId, idBuffer) == 0) {
              strcpy_P(veDeviceName, (PGM_P)VeDirectDeviceList[j][1]);
              break;
            }
          }

          if (strlen(veDeviceName) == 0) {
            strcpy(veDeviceName, "NO PID DATA");

            Serial.print("NO PID DATA");
            
          }

       //   Serial.print("=== VE.Direct Device info ===");
       //   Serial.print("Device_model: ");
       //   Serial.println(veDeviceId);
          Serial.print("Device_name: ");
          Serial.println(veDeviceName);
        //  Serial.println("==========END of Scan =============");

          pidChecked = true;
          pidfound = true;
          pidTimerStarted = false;
          myve.frameIndex = 0;

         Json["Device_name"]  = veDeviceName;

          break;
        }
      }
    }
  }
  // Timeout – keine PID gefunden
  else if (!pidChecked && millis() - pidStartTime >= 5000) {

 // else if (!pidChecked) {
    pidChecked = true;
    pidfound = false;
    pidTimerStarted = false;
    pidStartTime = 0;
  //  Serial.println(" NO PID / Victron not found.");
    Json["Device_name"] = "Victron offline"; // OK
   // Json["victronStatus"] = "no PID";  // OK
  }

//---------------------------------------------------------------
  // prozessData_ESP

  if (!hasData) {
    if (millis() - lastProcessDataESP < processDataEspInterval) return;
    lastProcessDataESP = millis();

    prozessData_ESP();
    notifyClients();
  //   if (ws.count() > 0) {
  //  notifyClients();
 // }

    // Serial.println("ESP Prozess Data");
  }

  if (hasData) {
     lastVictronData = millis();  // ✅ Zeitpunkt merken
    }

    
    if (millis() - lastVictronData > 5000) {
    Json["victron"].clear(); // wieder aktivieren 

    // Json["victronStatus"] = "timeout";
      Json["Device_name"]  = "Victron offline";

    //  Serial.println("Victron offline");
      
        pidfound = false;           //  PID als "nicht gefunden" markieren
        pidChecked = true;          // Suche abgeschlossen (wird später neu gestartet)
        pidTimerStarted = false;    // Timer freigeben
  }

}



// ---WRAPPER  Hilfsfunktion für WebSocket + Serial macht aus ws.textAll("n"); sendWS--
void sendWS(const String &msg) {
  ws.textAll(msg);
  Serial.println(msg);
}
//-----------------------------------------------------------

void sendTimerStatus() {
  if (panelOverThresholdStart > 0 && !autoModeTriggeredByPanel) {
    float elapsedMinutes = (millis() - panelOverThresholdStart) / 60000.0;
    sendWS("on_timer_min:" + String(elapsedMinutes, 1));
  }

  if (panelUnderThresholdStart > 0 && autoModeTriggeredByPanel) {
    float elapsedMinutes = (millis() - panelUnderThresholdStart) / 60000.0;
    sendWS("off_timer_min:" + String(elapsedMinutes, 1));
  }
}
// -------------------------------------------------

void logAutoModeStatus() {
  static unsigned long lastLog = 0;
  if (millis() - lastLog < 5000) return;  // nur alle 2 Sekunden loggen
  lastLog = millis();

  Serial.print("[AutoMode] Panel: ");
  Serial.print(Victron_PanelVoltage, 1);
  Serial.print("V | State: ");
  Serial.print(autoModeTriggeredByPanel ? "ON" : "OFF");

  if (panelOverThresholdStart > 0) {
    float minRun = (millis() - panelOverThresholdStart) / 60000.0;
    Serial.print(" | onTimer: ");
    Serial.print(minRun, 1);
    Serial.print(" min");
  }

  if (panelUnderThresholdStart > 0) {
    float minRun = (millis() - panelUnderThresholdStart) / 60000.0;
    Serial.print(" | offTimer: ");
    Serial.print(minRun, 1);
    Serial.print(" min");
  }

  if (panelOverThresholdStart == 0 && panelUnderThresholdStart == 0) {
    Serial.print(" | Timer: none");
  }

  Serial.println(); 
}

// --- Automatiksteuerung ---
void AutoMode() {
 
    if (!autoMode) {

      if (panelOverThresholdStart > 0) {
      panelOverThresholdStart = 0;
      sendWS("on_timer_min:off");
      Serial.println("Einschalt-Timer gestoppt kein Automatikmodus");
    }

    if (panelUnderThresholdStart > 0) {
      panelUnderThresholdStart = 0;
      sendWS("off_timer_min:off");
      Serial.println("Ausschalt-Timer gestoppt kein Automatikmodus");
    }

      autoModeTriggeredByPanel = false;

    //  set(wr, false); // if autoMode is off do not switch the inverter off

    return;  
  } //-----------------------------------------

  // === Einschaltlogik ===
  if (Victron_PanelVoltage > start_inverter) {
   //  if (test_PanelVoltage > start_inverter) {

    // Ausschalt-Timer stoppen, falls aktiv
    panelUnderThresholdStart = 0;

    // Einschalt-Timer starten, falls noch nicht aktiv
    if (panelOverThresholdStart == 0 && !autoModeTriggeredByPanel) {
      panelOverThresholdStart = millis();
      sendWS("on_timer_min:0.0");
      Serial.println(" Einschalt-Timer gestartet");

    }

    // Einschalt-Timer läuft → prüfen, ob genug Zeit vergangen ist
    if (!autoModeTriggeredByPanel &&
      millis() - panelOverThresholdStart >= (unsigned long)start_delay * 60UL * 1000UL) {
      autoModeTriggeredByPanel = true;

      set(wr, true);

      sendWS("on_timer_min:off");
      panelOverThresholdStart = 0; // Timer zurücksetzen

    }
  }

  // === Ausschaltlogik ===
  else if (Victron_PanelVoltage < stop_inverter) {
   //   else if (test_PanelVoltage < stop_inverter) {
    // Einschalt-Timer stoppen
    panelOverThresholdStart = 0;

    // Ausschalt-Timer starten, falls noch nicht aktiv
    if (panelUnderThresholdStart == 0 && autoModeTriggeredByPanel) {
      panelUnderThresholdStart = millis();
      sendWS("off_timer_min:0.0");
      Serial.println(" Ausschalt-Timer gestartet");
    }

    // Ausschalt-Timer läuft → prüfen, ob genug Zeit vergangen ist
    if (autoModeTriggeredByPanel &&
        millis() - panelUnderThresholdStart >= (unsigned long)stop_delay * 60UL * 1000UL) {
      autoModeTriggeredByPanel = false;

     set(wr, false);
 
      sendWS("off_timer_min:off");
      panelUnderThresholdStart = 0;

    }
  }

  // === Zwischenbereich → beide Timer aus ===
  else {
    if (panelOverThresholdStart > 0) {
      panelOverThresholdStart = 0;
      sendWS("on_timer_min:off");
      Serial.println(" Einschalt-Timer abgebrochen");
    }

    if (panelUnderThresholdStart > 0) {
      panelUnderThresholdStart = 0;
      sendWS("off_timer_min:off");
      Serial.println(" Ausschalt-Timer abgebrochen");
    }
  }

  // === Timerstatus senden ===
 // logAutoModeStatus(); // logging serial print 
  sendTimerStatus(); // logging timer

}



//-----global end -----------------------------------------------


//***********************************************************
//  start SETUP **********************************
//***********************************************************
void setup() {

  // VE.Direct starten
  veSerial.begin(VICTRON_BAUD, SWSERIAL_8N1, MYPORT_RX);
  veSerial.flush();
  veSerial.enableRxGPIOPullUp(false);
  delay(500);  // kleine Pause, damit UART stabil ist
  myve.callback(prozessData);

  Wire.begin();

// pcf begin
  Wire.beginTransmission(PCF_ADDR);   // Alle Relais AUS
  Wire.write(pcfState);
  Wire.endTransmission();

  // Serielle Ausgabe
  Serial.begin(DEBUG_BAUD);

  // Pins & Resetlogik
  pinMode(LED_PIN, OUTPUT);
  analogWrite(LED_PIN, 0); // inverted led on

  digitalWrite(MYPORT_TX, 0); // set to pulldown

  pinMode(DISPLAY_BUTTON_PIN, INPUT_PULLUP);   // button display sleep/ wakeup Taster gegen GND
   
sensors.begin(); 
  sensors.requestTemperatures();

   tempSensor_victron = sensors.getTempC(sensorVictron);
   tempSensor_battery = sensors.getTempC(sensorBattery);

//------------------------------------------------

    // Display initialisieren
  display.begin(0x3C, false);
  delay(500);
  display.setTextColor(SH110X_WHITE);
  display.display();
  delay(1000);

  display.setContrast(100);  //200 = 80& //  255 is max // 130 = 50&

// ** set display timeout *****
  displayStartTime = millis();
  displayOn = true;

  // Gerät benennen
WiFi.hostname("VICTRON");

wifiMulti.addAP(ssid1, password1);
wifiMulti.addAP(ssid2, password2);
wifiMulti.addAP(ssid3, password3);

Serial.println("scan done");
int n = WiFi.scanNetworks();
if (n == 0) {
  Serial.println("no networks found");
} else {
  Serial.printf("%d networks found\n", n);
  for (int i = 0; i < n; ++i) {
    Serial.printf("%d: %s (%d)%s\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i),
      (WiFi.encryptionType(i) == AUTH_OPEN) ? " " : "*");
  }
}

//Serial.println("Connecting Wifi...");
int attempts = 0;
while (wifiMulti.run() != WL_CONNECTED && attempts < 10) {
  Serial.print(".");
  delay(500);
  attempts++;
}

if (WiFi.status() == WL_CONNECTED) {
  Serial.println("\nWiFi connected to:");
  Serial.printf("SSID: %s\n", WiFi.SSID().c_str());
  myssid = WiFi.SSID();
  myip = WiFi.localIP().toString();

  Serial.printf("IP address: %s\n", myip.c_str());
  Serial.printf("Signal strength: %d dBm\n", WiFi.RSSI());
  Serial.printf("HTTP Server running on port: %d\n", httpPort);

  WiFi.setSleep(false);
} else {
  Serial.println("Failed to connect to WiFi");
}

// 🔽 Sendeleistung reduzieren FIX (Wert in dBm)
 // WiFi.setOutputPower(10);  // z. B. 10 dBm = ca. 10 mW
/*
20,5	100 mW	Standard, hohe Reichweite
15	32 mW	gute Reichweite, weniger Hitze
10	10 mW	spart Strom, ideal bei kurzer Distanz
5	3 mW	sehr geringer Verbrauch, kurze Reichweite
*/

// ipadresse anzeigen display
String ip = WiFi.localIP().toString();  // splitt IP adresse in 2 teile
  int secondDot = ip.indexOf('.', ip.indexOf('.') + 1);
  String firstPart = ip.substring(0, secondDot);
  String secondPart = ip.substring(secondDot + 1);

//------------------------------------------------
if (!LittleFS.begin()) {
    Serial.println("LittleFS initialisieren fehlgeschlagen.");
} else {
    Serial.println("LittleFS initialisiert.");
}

 // saveAllToLittleFS(); // initial  write the variables.txt file 
  delay(500);
  loadValuesFromLittleFS();

  // Webserver-Routen *******************************

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", HTML_index);
    request->send(response);
  });

  //NO Zeigerobject (*response) NO RAM-Overhead for Stream-Handling
 server.on("/allvalues", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", HTML_allvalues);
    request->send(response);
  });

/*
  server.on("/livejson", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    serializeJson(Json, *response);
    request->send(response);
  });
*/
 
server.on("/livejson", HTTP_GET, [](AsyncWebServerRequest *request) {
  String json;
  serializeJson(Json, json);
  request->send(200, "application/json", json);
});

  
  server.on("/variables.txt", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(LittleFS, "/variables.txt", "text/plain");
});

server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request) {
  request->send(LittleFS, "/favicon.ico", "image/x-icon");
});


//------SEND  VALUES TO FRONTEND ------------------------------------------
server.on("/sendValues", HTTP_GET, [](AsyncWebServerRequest *request) {
  StaticJsonDocument<200> doc;
  doc["start_inverter"] = start_inverter;
  doc["stop_inverter"] = stop_inverter;
  doc["start_delay"] = start_delay;
  doc["stop_delay"] = stop_delay;
  doc["fan_t"] = fan_t;

  String json;
  serializeJson(doc, json);
  request->send(200, "application/json", json);
});

server.on("/saveAll", HTTP_GET, [](AsyncWebServerRequest *request) {
  saveAllToLittleFS(); // 👉 deine Funktion zum Speichern
  request->send(200, "text/plain", "-Werte gespeichert");
});

server.on("/sendback", HTTP_GET, [](AsyncWebServerRequest *request) {
  if (request->hasParam("var") && request->hasParam("val")) {
    String varName = request->getParam("var")->value();
    String valStr = request->getParam("val")->value();
    int val = valStr.toInt(); // oder valStr.toFloat(), falls du mit Kommazahlen arbeitest

    if (varName == "start_inverter") {
      start_inverter = val;
     // Serial.println("start_inverter aktualisiert: " + String(start_inverter));

    } else if (varName == "stop_inverter") {
      stop_inverter = val;
     // Serial.println("stop_inverter aktualisiert: " + String(stop_inverter));

     } else if (varName == "start_delay") {
      start_delay = val;
     // Serial.println("start_delay aktualisiert: " + String(start_inverter));

    } else if (varName == "stop_delay") {
      stop_delay = val;
     // Serial.println("stop_delay aktualisiert: " + String(stop_inverter));

    } else if (varName == "fan_t") {
      fan_t = val;
     // Serial.println("fan_t aktualisiert: " + String(fan_t));      
    }

    request->send(200, "text/plain", "Werte aktualisiert");
  } else {
    request->send(400, "text/plain", "Fehlende Parameter");
  }

});

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
  });

 // delay(1000);

  // WebSocket aktivieren
  ws.onEvent(onEvent);         // Event-Handler registrieren
  server.addHandler(&ws);      // WebSocket dem Server hinzufügen

  server.begin();

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(10, 5); //65/32 bei schrift watt gr 1
  display.print("Victron");
  display.setCursor(5, 25); //65/32 bei schrift watt gr 1
  display.print("KamizoTec");
  display.setCursor(35, 50); //65/32 bei schrift watt gr 1
  display.print("2025");
  display.display(); 
  delay(1000);

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(5, 5);
  display.print("WIFI- IP");
  display.setCursor(0, 30);
  display.print(firstPart);
  display.setCursor(0, 50);
  display.print(secondPart);
  display.display();
  delay(2000);

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(5, 10);
  display.print("Net-SSID");
  display.setCursor(0, 32);
  display.print(WiFi.SSID());
  display.display();
  delay(2000);

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(15, 0);
  display.print("Port-");
  display.print(httpPort);
  display.setTextSize(2);
  display.setCursor(15, 32);
  display.print("WIFI:");
  display.print(WiFi.RSSI());
  // display.print(signalStrength);
  display.display();
  delay(2000);

//-----------------------------------------------------------

  analogWrite(LED_PIN, 1); // inverted led off
//  resetCounter(false);

//************************************************************
} //----------- SETUP END ----------------------------------------
//************************************************************

// WIFI sendeleistung begrenzen , kann auch im setup fur nicht dynamisch 
    // adjustWiFiPower();

void loop() { // ------ START LOOP ---------------------------------

  // Reboot  -------------------------------
  if (restartNow && millis() >= (RestartTimer + 1000)) {
    ESP.restart();
  }

  // VE.Direct -Daten einlesen ----------------
   ReadVEData();
//---------------------------------------------
  // WebSocket pflegen -----------------------------
  if (WiFi.status() == WL_CONNECTED) {
    ws.cleanupClients();
  } 
//--------------------------------------
  //  checkDisplayButton();

//***************************************************************
  // Display aktualisieren alle 5 Sekunden -----------------------------
  if (millis() - lastDisplayUpdate > 5000) {
    lastDisplayUpdate = millis();

 // analogWrite(LED_PIN, 0); // inverted led on

  sensors.requestTemperatures();
   tempSensor_victron = sensors.getTempC(sensorVictron);
   tempSensor_battery = sensors.getTempC(sensorBattery);
   
//------------------------------------------------------------
    // Display-Werte aktualisieren
    mppt = Json["victron"]["Operation_state"];  // Integer-Zustand
    Victron_PanelVoltage = Json["victron"]["Panel_voltage"].as<float>();
    Victron_PanelPower = Json["victron"]["Panel_power"].as<float>();
    Victron_BatteryVoltage = Json["victron"]["Voltage"].as<float>();
    Victron_BatteryCurrent = Json["victron"]["Battery_current"].as<float>();

// auto button send 
  if(wsConnected){
   // send  autoMode to clients
    if(firstRun || autoMode != lastAutoModeSent){
      sendWS(autoMode ? "autoMode:on" : "autoMode:off");
      lastAutoModeSent = autoMode;
      firstRun = false;
    }
 
    // Automatik steuern
    AutoMode(); // VOID

//if (autoMode && Victron_PanelVoltage > 0.0) {
//  handleAutoMode();
//}

//if (Victron_PanelVoltage <= 0.0) {
 // Serial.println("[AutoMode] Victron_PanelVoltage wrong Automatik overturn");
  //return;
//}

}
  //*************************************
  display.clearDisplay();
// 🔹 Panel Voltage oben links – TextSize 1 ------------------
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("PV: ");
  display.print(String(Victron_PanelVoltage, 2));  // 2 Nachkommastellen
  display.print("V  ");

 // Serial.print("Battery Voltage: ");
 // Serial.println(Victron_BatteryVoltage, 2);  // zwei Nachkommastellen
//---------------------------------------------------
  // TX-Leistung 
 // display.setCursor(75, 0);
  display.print(String(txPower_mW));
  display.print("mW  ");

  // RSSI  in %
 // display.setCursor(105, 0);
  display.print(String(rssiToPercent(rssi)));
  display.print("%");

//serial
//  Serial.println("TX Power (mW): " + String(txPower_mW));
  //Serial.println("RSSI: " + String(rssi));
 // Serial.println("Signal: " + String(rssiToPercent(rssi)));

// 🔹 Große Leistungsanzeige MITTE  ------------------------------------
  display.setTextSize(3);           // Große Zahl
  display.setCursor(5, 16);         //20 Linksbündig, leicht nach unten versetzt
  int pwr = (int)Victron_PanelPower;  // kein Komma, ganze Zahl
// ---- Null-Padding auf 3 Stellen ----
  if      (pwr < 10)   display.print("00");
  else if (pwr < 100)  display.print("0");
  display.print(pwr);

  // 🔹 Einheit und Status rechts daneben
  display.setTextSize(2);           // Mittelgroß
  display.setCursor(65, 21);        // Rechts daneben, leicht höher für optische Mitte
  display.print("W.");              // Einheit
  display.setCursor(90, 21);       // Status daneben
  display.print(getOperationStateShort(mppt));  // z. B. "OK"

//------------------------------------------------------------------

// 🔹 TEMP SENSORs ------------------
  display.setTextSize(1);
  display.setCursor(0, 45);
  display.print("Vic:" );
  display.print(String(tempSensor_victron, 1));
  display.print("  ");
 // display.setCursor(64, 45); //70
  display.print("Batt:");
 // display.setCursor(110, 45);
  display.print(String(tempSensor_battery, 1));
  display.printf(" C");
   // display.print(String(tempSensor_battery, 1) +"°C");

  // 🔹 Battery Voltage & Current unten – TextSize 1 -----------------
display.setCursor(0, 56);//56
display.print("Batt: ");
display.print(String(Victron_BatteryVoltage, 2));
display.println(" V");

display.setCursor(85, 56);
display.print(" A");
display.print(String(Victron_BatteryCurrent, 2));
display.display(); // Anzeige aktualisieren

  //Serial.print("Temp Victron : ");
 // Serial.println(tempSensor_victron);
 // Serial.print("Temp Battery : ");
//  Serial.println(tempSensor_battery);

  // victron cooler
// Victron-Kühllogik mit einstellbarer Schwelle (fan_t) und Hysterese 5°C
// Lüfter EIN bei Überschreitung
if (!getState(fan) && tempSensor_victron >= fan_t) {
  set(fan, true);  // Relais EIN
  Serial.println(" Victron-Fan EIN  Temp over set");
}

// Lüfter AUS bei Unterschreitung mit Hysterese
if (getState(fan) && tempSensor_victron <= (fan_t - 5)) {
  set(fan, false); // Relais AUS
  Serial.println(" Victron-Fan AUS  Temperatur unter set minus Hysterese");
}

//----------------------------------------------
//if (WiFi.status() != WL_CONNECTED) {
 // WiFi.disconnect();
 // Serial.println("WLAN getrennt versuche Reconnect...");
 // wifiMulti.run(); // Versucht Verbindung zu einem bekannten Netzwerk  
//}

if (wifiMulti.run() != WL_CONNECTED) {
  Serial.println("WLAN getrennt – versuche Reconnect...");
}

  dynamicWifi(); // dynamic wifi calc TX

// switch off display auto after xx min
  checkDisplayTimeout();
  
 // analogWrite(LED_PIN, 1); // inverted led off

  } // DISPLAY MILLIS ENDE ----------------------------



}   //--------- LOOP END -------------------------------------------------
//****************************************************************** 
//---------VOID FUNCTIONS DATA -------------------------------------------------

// rozess data with delay millis

unsigned long lastreadProcess = 0;
const unsigned long readprocessInterval = 500;  // zeit prozessing

  void prozessData() {

  if (millis() - lastreadProcess < readprocessInterval) return;  

  lastreadProcess = millis();  // Zeit merken
  
  dataProzessing = true;
  getJsonData();           // VICTRON-Data read
  notifyClients();         // WebSocket senden
//  if (ws.count() > 0) {
 //   notifyClients();           // WebSocket senden
//  }

  dataProzessing = false;

} //----------------------------------------

// build VICTRON DATA JSON  -------------------------------------
bool getJsonData() {

  // VE.Direct-Daten auf Hauptebene JSON BAUEN
for (int i = 0; i < myve.veEnd; i++) {
    if (!myve.veName[i] || !myve.veValue[i] || strlen(myve.veName[i]) == 0 || strlen(myve.veValue[i]) == 0) {
        break;
    }

    for (size_t j = 0; j < sizeof(VePrettyData) / sizeof(VePrettyData[0]); j++) {
        if (strcmp(VePrettyData[j][0], myve.veName[i]) == 0) {
            const char* key = VePrettyData[j][1];
            const char* scaleStr = VePrettyData[j][2];

            // Wert umrechnen und in victron-Objekt ["victron"]  schreiben
            if (strlen(scaleStr) > 0 && strcmp(scaleStr, "0") != 0) {
                Json["victron"][key] = roundf(atof(myve.veValue[i]) / atof(scaleStr) * 100) / 100.0;

            } else if (strcmp(scaleStr, "0") == 0) {
                Json["victron"][key] = atoi(myve.veValue[i]);

            } else {
                Json["victron"][key] = myve.veValue[i];
            }

            // Textfelder direkt ergänzen
            if (strcmp(key, "Operation_state") == 0) {
                Json["victron"]["Operation_state_text"] = getOperationStateName(atoi(myve.veValue[i]));
            } 

            else if (strcmp(key, "Tracker_operation_mode") == 0) {
                Json["victron"]["Tracker_operation_mode_text"] = getTrackerModeName(atoi(myve.veValue[i]));
            }


    // 🔹 Fehlertext ergänzen – direkt über VE.Direct-Feldnamen
    if (strcmp(myve.veName[i], "ERR") == 0) {
      Json["victron"]["Current_error_text"] = getErrorCodeText(atoi(myve.veValue[i]));
    }

            break;
        }
    }
}


 //  Geräteinfos aus RAM einfügen
  // Beispiel {"0XA060", "SmartSolar MPPT 100/20 48V"}

// Json["Device_model"] = veDeviceId;
//Json["Device_model"] = pidfound ? veDeviceId : "unknown";
//Json["victronStatus"] = pidfound ? "online" : "no PID";

return true;
}
//-----------------------------------------------------------------

// BUILD ESP JSON 
void prozessData_ESP() {
 // Json.clear();  // wichtig: leeren, damit keine alten Victron-Daten drin sind
float vcc = (ESP.getVcc() / 1000.0) + 0.3;

  Json["IP"] = WiFi.localIP().toString();
  Json["Wifi_RSSI"] = WiFi.RSSI();
  Json["ESP_VCC"] = String(vcc, 2);  // ergibt z. B. "3.30" als String
  Json["Free_Sketch_Space"] = ESP.getFreeSketchSpace();
  Json["Free_Heap"] = ESP.getFreeHeap();
  Json["HEAP_Fragmentation"] = ESP.getHeapFragmentation();
  Json["json_space"] = measureJson(Json);
  Json["Runtime"] = millis() / 1000;
  Json["Temp_Victron"] = String(tempSensor_victron, 1);
  Json["Temp_Battery"] = String(tempSensor_battery, 1);

  String jsonString;
  serializeJson(Json, jsonString);
  ws.textAll(jsonString);  // oder webSocket.sendTXT(jsonString);
}
//-------------------------------------------------------------

/*
Jede Zeile bei TextSize(1) ist 8 Pixel hoch:

Y = 0
Y = 8
Y = 16
Y = 24
Y = 32
Y = 40
Y = 48 ← vorletzte Zeile
Y = 56 ← letzte volle Zeile


//------------------------------------------------------------

// 🔹 TextSize(2) – Zeilenhöhe 16 px
// Jede Zeile beginnt bei:
Y = 0;   // Zeile 1
Y = 16;  // Zeile 2
Y = 32;  // Zeile 3
Y = 48;  // Zeile 4 ← letzte volle Zeile

//------------------------------------------------------------

// 🔹 TextSize(3) – Zeilenhöhe 24 px // 6–7 Zeichen pro Zeile
// Jede Zeile beginnt bei:

Y = 0;   // Zeile 1
Y = 24;  // Zeile 2
Y = 48;  // Zeile 3 ← letzte volle Zeile

*/


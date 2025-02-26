/*
author Leszek Klich
Licence: see LICENSE file
Automatic Router Reseter with web configuration - ESP32 Lolin Lite - AsyncWebServer
Automatyczny Reseter Routera z konfiguracją przez interfejs Web przy wykorzystaniu - AsyncWebServer
*/

#include <Arduino.h>
#include <logger.h>
#include <pinout.h>
#include <config.h>
#include <tools.h>
#include <relay.h>
#include <network.h>
#include "Thread.h"
#include <www.h>
#ifdef ESP32
#include <ESP32Ping.h>
#elif defined(ESP8266)
#include <ESP8266Ping.h>
#endif
#include <time.h>
#include <mqttcli.h>


bool isRebootRequired = false;
bool isClearLog = false;
bool connected = false;
bool started = false;

int8_t rssi = 0;           // signal strench
long countDisconnect = 0;  // count error ping
long countConnect = 0;     // count ping ok
struct tm timeInfo;        // Datetime struct
time_t now;
String actTime = "00:00:00";
String actDate = "0000-00-00";

String act_rssi_percent = "0"; // Variable for www template

ConfigManager configManager;
Relay relay(RELAY_PIN);
WiFiManager wifi; 

Thread wifiTestThread = Thread();     // WiFi testing
Thread testingPingThread = Thread();  // Ping test
Thread ntpTimeThread = Thread();      // Get datetime from NTP server
Thread actDateTimeThread = Thread();  // Refresh datetime
Thread startTestThread = Thread();    // Delay first time start test after router reset

MQTTCli mqttHandler; 

// ---------------------------------------
void setup();
void checkWiFi();
void resetBtnClick();
void runTestPing();
void timeFromNTP();
void actualizeTime();
void writeLog(const char* message);
void clearLog();
void mqttSend();
void startTest();
// --------------------------------------

void setup() {
  Logger::getInstance().begin(115200);
  Logger::getInstance().log(LOG_INFO, "START SYSTEM");
  pinMode(LED_PIN, OUTPUT);          // LED WiFi info
  digitalWrite(LED_BUILTIN, OUTPUT); // LED on module
  pinMode(BTN_PIN, INPUT);           // Button 
  pinMode(RELAY_PIN, OUTPUT);        // Relay
  digitalWrite(RELAY_PIN, LOW);      // Disable relay
  digitalWrite(BUILTIN_LED, LOW);    // Disable builtin LED
  delay(500);
  Tools tools;
  tools.displayWelcome();
  tools.getChipType();
  tools.checkFlash();
  configManager.begin();
  if(digitalRead(BTN_PIN) == LOW) {
    resetBtnClick();
  }
  configManager.showConfig();
  relay.begin();
  wifi.begin();
  fsStart();
  Logger::getInstance().log(LOG_INFO, "WIFI SCAN");
  if (config.confMode == 0){
    WiFi.scanNetworks(true);
    String per_rssi = "WIFI RSSI: " + act_rssi_percent + "%";
    Logger::getInstance().log(LOG_INFO, per_rssi.c_str());
    wifiTestThread.onRun(checkWiFi);
    wifiTestThread.setInterval(5000);     // Test WiFi connection every 5 second
  
    testingPingThread.onRun(runTestPing);
    testingPingThread.setInterval(5000);  // Test ping connection every 5 second
  
    ntpTimeThread.onRun(timeFromNTP);
    ntpTimeThread.setInterval(1800000);   // Get time from NTP every half time

    actDateTimeThread.onRun(actualizeTime);
    actDateTimeThread.setInterval(1000);  // REfrest datetime every 1 second

    startTestThread.onRun(startTest);
    startTestThread.setInterval(config.starttest * 1000); // Delay ping test every router restart and run device

    timeFromNTP();
    
    if(config.mqtt){
      mqttHandler.enabled = true;
      mqttHandler.begin(config.mqttBroker, config.mqttPort, config.mqttUser, config.mqttPass, config.hostname);
    } else {
      mqttHandler.enabled = false;
    }
  }
  routing(); 
  Logger::getInstance().log(LOG_INFO, "SYSTEM STARTED");
}

void startTest(){
  if(!started){
    String ii = "DELAY " + String(config.starttest) + " SEC. START";
    Logger::getInstance().log(LOG_INFO, ii.c_str());
    writeLog("Enable testing");
    startTestThread.enabled = false;
    started = true;
  }
}

void mqttSend(){
 if((wifi.isWiFiOK()) && (config.mqtt)){
    const char *stateMsg = relay.isOn() ? "OFF" : "ON"; // inverted logic: relay off = power on
    if (!mqttHandler.publishState(stateMsg)) {
      Serial.println("Błąd publikacji MQTT - nie wysłano stanu!");
  }
 }
}

void actualizeTime(){
    time(&now);
    localtime_r(&now, &timeInfo);
    char fTime[64];
    char fDate[64];
    strftime(fTime, sizeof(fTime), "%H:%M:%S", &timeInfo);
    strftime(fDate, sizeof(fDate), "%Y-%m-%d", &timeInfo);
    actTime = String(fTime);
    actDate = String(fDate);
}

void timeFromNTP(){
  if(wifi.isWiFiOK()){
    Logger::getInstance().log(LOG_INFO, "START NTP");
    //configTime(0, config.timeoffset, "pool.ntp.org", "time.nist.gov");
    // Ustawienie strefy czasowej dla Polski
    // "CET-1CEST,M3.5.0/2,M10.5.0/3" oznacza:
    // - CET (czas zimowy): UTC+1,
    // - CEST (czas letni): UTC+2,
    // - Przejście na czas letni: ostatnia niedziela marca o 2:00,
    // - Powrót do czasu zimowego: ostatnia niedziela października o 3:00.
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    setenv("TZ", "CET-1CEST,M3.5.0/2,M10.5.0/3", 1);
    tzset();
    Logger::getInstance().log(LOG_INFO, "NTP GET DATETIME");
    uint8_t licz = 1;
    uint8_t max_licz = 10;
    String inf = "";
    while (!time(nullptr)) {
      inf = "NTP TRY: " + String(licz) + " OF " + String(max_licz);
      Logger::getInstance().log(LOG_INFO, inf.c_str());
      delay(1000);
      licz++;
      if(licz>max_licz) {
        Logger::getInstance().log(LOG_ERROR, "NTP TIME ERROR");
        return;
      } 
    }
  Logger::getInstance().log(LOG_INFO, "NTP COMPLETED");
  return;
} else {
  if(config.confMode == 0) Logger::getInstance().log(LOG_INFO, "NTP NO CONNECTION");
}
}

// Function that saves the log to the logs.txt file
void writeLog(const char* message) {
  // File size check and cleanup if it exceeds 50KB
  if (LittleFS.exists("/nettest.txt")) {
    File checkFile = LittleFS.open("/nettest.txt", FILE_READ);
    if (checkFile.size() > 50 * 1024) {  // 50 * 1024 bajtów = 50KB
      Logger::getInstance().log(LOG_INFO, "NETTEST EXCEED 50KB");
      clearLog();
    } else {
      checkFile.close();
    }
  }
  String mess = "";
  if((actDate == "0000-00-00") || (actTime == "00:00:00"))
    mess = message;
  else
    mess = "[" + actDate + "] [" + actTime + "] " + message;
  File logFile = LittleFS.open("/nettest.txt", FILE_APPEND);
  if (!logFile) {
    Logger::getInstance().log(LOG_ERROR, "ERROR READ NETTEST.TXT");
    return;
  }
  logFile.println(mess);
  logFile.close();
}

// Clear log file
void clearLog() {
  if (LittleFS.exists("/nettest.txt")) {
    File myFile = LittleFS.open("/nettest.txt", "w");
    if (!myFile) {
      Logger::getInstance().log(LOG_INFO, "ERROR CLEAR NETTEST LOG");
    } else {
      Logger::getInstance().log(LOG_INFO, "LOG NETTEST CLEARED");
      myFile.close(); 
  }
  } else {
    Logger::getInstance().log(LOG_INFO, "LOG NETTEST NOT EXISTS");
  }
}

void checkWiFi(){
  if (!wifi.isWiFiOK()) {
    connected = false; // Lack of WiFi is a error!
    ntpTimeThread.enabled = false;
    act_rssi_percent = "0";
    rssi = 0;
  } else {
    connected = true;
    ntpTimeThread.enabled = true;
    rssi = wifi.getRSSI();
    act_rssi_percent = String(abs(wifi.rssiToPercent(rssi)));
  }
}

void resetBtnClick(){
    uint8_t counter = 10; // how many seconds to hold the factory reset button
    Serial.print("RSTBTN ");
    while(digitalRead(BTN_PIN) == LOW){
      Serial.print(counter); Serial.print(",");
      wifi.ledBlink();
      counter--;
      delay(1000);
      if (counter <= 1) { 
        digitalWrite(LED_PIN, HIGH);
        Logger::getInstance().log(LOG_INFO, "RESET FACTORY");
        while(digitalRead(BTN_PIN) == LOW){;}
        digitalWrite(LED_PIN, LOW);
        configManager.resetToDefaults();
        isRebootRequired = true;
      } else {
        isRebootRequired = true;
      }
    }
    Serial.println("");
}  

void runTestPing(){
 if(!started) return;
 if (wifi.isWiFiOK() && (config.confMode == 0)) {
  act_rssi_percent = String(wifi.getRSSI());
  IPAddress remote_ip(8,8,8,8);
  wifi.convertCharToIPAddress(config.iptest, remote_ip);
  bool pingResult = Ping.ping(remote_ip);
    if (pingResult) {
      countDisconnect = 0;
      countConnect++;
      connected = true;
    } else {
      countDisconnect++;
      countConnect=0;
      connected = false;
      Logger::getInstance().log(LOG_ERROR, "PING ERROR");
    }
  } else {
      if(config.confMode == 0){
        Logger::getInstance().log(LOG_ERROR, "WIFI ERROR");
        countDisconnect++; // if no WiFi increase
        connected = false;
      }
    }
    mqttSend();
}

void loop() {
  mqttHandler.loop();
  if(digitalRead(BTN_PIN) == LOW) {
    wifi.ledBlink();
    while(digitalRead(BTN_PIN) == LOW) {}
    Logger::getInstance().log(LOG_INFO, "HAND RESET");
    relay.turnOn();
    mqttSend();
  }

   if(ntpTimeThread.shouldRun())
    ntpTimeThread.run();

  if(wifiTestThread.shouldRun())
		wifiTestThread.run();

  if (testingPingThread.shouldRun())
    testingPingThread.run();

  if (actDateTimeThread.shouldRun()) // Refresh datetime every 1 second
    actDateTimeThread.run();

  if (startTestThread.shouldRun())  // config.starttest (delay run test after router rebot)
    startTestThread.run();

  if (isRebootRequired) {
    Logger::getInstance().log(LOG_INFO, "RESTART");
		delay(1000); // Delay for reboot.html page can load
		ESP.restart();
  }

  // switching on the relay when connected = false i countDisconnect >= config.timeout
  if (countDisconnect >= config.timeout) {
    Logger::getInstance().log(LOG_ERROR, "ROUTER RESTART");
    if (config.confMode == 0){
      writeLog("ROUTER RESTART");
      Logger::getInstance().log(LOG_INFO, "MQTT RESET");
      relay.turnOn(); // Enable relay (async)
      mqttSend();
      countDisconnect = 0;
      countConnect = 0;
      started = false;
      startTestThread.enabled = true;
    } else {
      started = false;
      startTestThread.enabled = false;
      countDisconnect = 0;
      countConnect = 0;
    }
  }

  if (isClearLog){
    clearLog();
    isClearLog = false;
  }

  relay.update();
  delay(150);
}

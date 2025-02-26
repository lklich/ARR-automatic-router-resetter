#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>
#ifdef ESP32
  #include <WiFi.h>
  #include <WebServer.h>
  #include <ESPmDNS.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266mDNS.h>
#endif
#include <pinout.h>
#include <config.h>
#include <logger.h>

class WiFiManager {
  public:
    WiFiManager();
    void begin();
    void connectToWiFi(); // void connectToWiFi(const char* ssid, const char* password);
    void setupAccessPoint(const char* newSSID, const char* newPassword);
    void checkWiFiConnection();
    void updateLED();
    void setupMDNS();
    void ReadConnection();
    bool isConnected(); // Zwraca stan połączenia
    bool convertCharToIPAddress(const char *str, IPAddress& ip);
    void ledBlink();
    bool isWiFiOK(); // True, gdy WiFi połączone i false, gdy brak połączenia
    int rssiToPercent(int rssi); // rssi to percent
    
    int8_t getRSSI();
    int8_t rssi = 0;
    bool useDHCP = true;
    IPAddress local_IP;
    IPAddress gateway;
    IPAddress subnet;
    IPAddress dns; 
    String ssidAP = "RESETER_2931";
    String passwordAP = "12345678";
    
    private:
      bool isAccessPoint = false;
};

#endif // WIFIMANAGER_H

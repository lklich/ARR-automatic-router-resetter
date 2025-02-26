#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <EEPROM.h>
#include <logger.h>

#define EEPROM_SIZE 512

extern bool isRebootRequired;
extern bool isClearLog;
extern bool connected;
extern long countConnect;
extern long countDisconnect;
extern String act_rssi_percent;
extern int8_t rssi;
extern String actDate;
extern String actTime;

struct Config {
  char ip[16];
  char subnet[16];
  char gateway[16];
  char dns[16];
  char iptest[16];     // adres pinga
  char ssid[32];
  char password[32];
  char hostname[32];
  char place[100];       // miejsce instalacji
  bool enable = true;    // czy włączać resetowanie routera?
  bool dhcp;             // czy włączyć DHCP?
  bool confMode;         // czy to tryb AP - konfiguracja?
  char user[10];         // użytkownik konfiguracji
  char pass[20];         // hasło użytkownika konfiguracji
  uint8_t relayTime;     // na jaki czas włączać przekaźnik do resetownia
  uint8_t timeout;       // czas jaki musi upłynąć po braku pinga do resetu routera.
  uint8_t starttest;     // po jakim czasie po resecie rozpocząć pingowanie.
  uint8_t timeoffset;    // przesunięcie czasu - letni/zimowy
  bool mqtt;             // włącz przesyłanie przez MQTT
  char mqttBroker[50];   // adres brokera MQTT
  uint16_t mqttPort; // port MQTT
  char mqttUser[30];  // login MQTT
  char mqttPass[50];  // hasło MQTT
};

// Global config declaration
extern Config config;

class ConfigManager {
  public:
    ConfigManager();  
    void begin();            // EEPROM initialization
    void readConfig();       // Odczyt konfiguracji z EEPROM
    void saveConfig();       // Zapis konfiguracji do EEPROM
    void resetToDefaults();  // Reset do ustawień domyślnych
    void showConfig();

    private:
      bool isEEPROMEmpty(); 
};

#endif

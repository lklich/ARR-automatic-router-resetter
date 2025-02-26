#include <config.h>

Config config;

ConfigManager::ConfigManager() {}

// EEPROM initialize
void ConfigManager::begin() {
  Logger::getInstance().log(LOG_INFO, "BEGIN Config");
  EEPROM.begin(EEPROM_SIZE);
  if (isEEPROMEmpty()) {
    Logger::getInstance().log(LOG_INFO, "EEPROM Empty");
    resetToDefaults();
    saveConfig();
    readConfig(); 
  } else {
    Logger::getInstance().log(LOG_INFO, "READ Config");
    readConfig();
  }
}

// Check is EEPROM is empty
bool ConfigManager::isEEPROMEmpty() {
  if (EEPROM.read(EEPROM_SIZE-1) != 252){ 
    Logger::getInstance().log(LOG_INFO, "INIT NEW EEPROM!");
    return true;
  } else {
  return false;
  }
}

// Read configurstion from EEPROM
void ConfigManager::readConfig() {
  EEPROM.get(0, config);
}

// Save config to EEPROM
void ConfigManager::saveConfig() {
  Logger::getInstance().log(LOG_INFO, "SAVE CONFIG");
  EEPROM.put(0, config);
  EEPROM.write(EEPROM_SIZE-1, 252);
  if (EEPROM.commit()) {
    Logger::getInstance().log(LOG_INFO, "SAVE EEPROM OK");
  } else {
    Logger::getInstance().log(LOG_ERROR, "SAVE EEPROM ERROR");
  }
  EEPROM.end();
}

// Factory reset EEPROM
void ConfigManager::resetToDefaults() {
    Logger::getInstance().log(LOG_INFO, "EEPROM RESET FACTORY");
    EEPROM.begin(EEPROM_SIZE); 
    for (int i = 0; i < EEPROM_SIZE; i++) {
        EEPROM.write(i, 0);
    }
  EEPROM.write(EEPROM_SIZE-1, 0);
  strcpy(config.ssid, ""); 
  strcpy(config.password, "");
  strcpy(config.hostname, "ARR_102301");
  strcpy(config.place, "Installation place");
  config.dhcp = 1;
  strcpy(config.iptest, "8.8.8.8");
  strcpy(config.ip, "192.168.0.10");
  strcpy(config.subnet, "255.255.255.0");
  strcpy(config.gateway, "192.168.0.1");   
  strcpy(config.dns, "8.8.8.8");
  strcpy(config.user, "admin");
  strcpy(config.pass, "admin");
  config.timeoffset = 1;
  config.enable = 1;
  config.confMode = 1;
  config.relayTime = 3;
  config.timeout = 20;
  config.starttest = 30;
  config.mqtt = 0;
  strcpy(config.mqttBroker, "");
  config.mqttPort = 1883;
  strcpy(config.mqttUser, "mqtt_user"); 
  strcpy(config.mqttPass, "password");  
  EEPROM.put(0, config); 
  EEPROM.write(EEPROM_SIZE-1, 252);
  saveConfig();
  readConfig();
  isRebootRequired = true;
  }

void ConfigManager::showConfig(){
  Logger::getInstance().log(LOG_INFO, "==============");
  Logger::getInstance().log(LOG_INFO, "CONFIG EEPROM:");
  Logger::getInstance().log(LOG_INFO, "==============");
  String ii = "SIZE: " + String(sizeof(config)) + "B";
  Logger::getInstance().log(LOG_INFO, ii.c_str());

  if(config.confMode)
   ii = "MODE: AP SETUP";
  else
   ii = "MODE: NORMAL";
  Logger::getInstance().log(LOG_INFO, ii.c_str());

  ii = "TIME OFFSET: " + String(config.timeoffset);
  Logger::getInstance().log(LOG_INFO, ii.c_str());

  ii = "PLACE: " + String(config.place);
  Logger::getInstance().log(LOG_INFO, ii.c_str());
  ii = "HOSTNAME: " + String(config.hostname);
  Logger::getInstance().log(LOG_INFO, ii.c_str());

  ii = "WIFI SSID: " + String(config.ssid);
  Logger::getInstance().log(LOG_INFO, ii.c_str());

  //ii = "WIFI PASS: " + String(config.password);
  //Logger::getInstance().log(LOG_INFO, ii.c_str());

  ii = "DHCP: " + String(config.dhcp);
  Logger::getInstance().log(LOG_INFO, ii.c_str());

  ii = "IP: " + String(config.ip);
  Logger::getInstance().log(LOG_INFO, ii.c_str());

  ii = "MASK: " + String(config.subnet);
  Logger::getInstance().log(LOG_INFO, ii.c_str());

  ii = "GATEWAY: " + String(config.gateway);
  Logger::getInstance().log(LOG_INFO, ii.c_str());

  ii = "DNS: " + String(config.dns);
  Logger::getInstance().log(LOG_INFO, ii.c_str());

  ii = "USER: " + String(config.user);
  Logger::getInstance().log(LOG_INFO, ii.c_str());

  ii = "USER PASS: " + String(config.pass);
  Logger::getInstance().log(LOG_INFO, ii.c_str());

  ii = "ENABLED: " + String(config.enable);
  Logger::getInstance().log(LOG_INFO, ii.c_str());

  ii = "TIME RELAY: " + String(config.relayTime);
  Logger::getInstance().log(LOG_INFO, ii.c_str());

  ii = "TIMEOUT: " + String(config.timeout);
  Logger::getInstance().log(LOG_INFO, ii.c_str());

  ii = "IP TEST: " + String(config.iptest);
  Logger::getInstance().log(LOG_INFO, ii.c_str());

  ii = "DELAY TEST: " + String(config.starttest);
  Logger::getInstance().log(LOG_INFO, ii.c_str());

  ii = "MQTT: " + String(config.mqtt);
  Logger::getInstance().log(LOG_INFO, ii.c_str());
  if(config.mqtt) {
    ii = "BROKER: " + String(config.mqttBroker);
    Logger::getInstance().log(LOG_INFO, ii.c_str());
    ii = "PORT: " + String(config.mqttPort);
    Logger::getInstance().log(LOG_INFO, ii.c_str());
    ii = "USER: " + String(config.mqttUser);
    Logger::getInstance().log(LOG_INFO, ii.c_str());
  }

  Logger::getInstance().log(LOG_INFO, "=================");
}
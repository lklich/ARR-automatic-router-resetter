#include <network.h>
#include <logger.h>

WiFiManager::WiFiManager() {}

void WiFiManager::begin() {
  Logger::getInstance().logInfo("START WIFI");
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  isAccessPoint = config.confMode;
  if (isAccessPoint) {
    setupAccessPoint(ssidAP.c_str(), passwordAP.c_str());
  } else {
    Logger::getInstance().logInfo("MODE: CLIENT");
    ReadConnection();
    connectToWiFi();
  }
  getRSSI();
  setupMDNS();
}

void WiFiManager::ledBlink(){
  digitalWrite(LED_PIN, LOW);
  delay(100);
  digitalWrite(LED_PIN, HIGH);
  delay(100);
}

// Conver char to IPAddress
bool WiFiManager::convertCharToIPAddress(const char* str, IPAddress& ip) {
  uint8_t octets[4];
  int parsed = sscanf(str, "%hhu.%hhu.%hhu.%hhu", &octets[0], &octets[1], &octets[2], &octets[3]);
  if (parsed == 4) {
    ip = IPAddress(octets[0], octets[1], octets[2], octets[3]);
    return true;
  }
  return false;
}

void WiFiManager::ReadConnection() {
  Logger::getInstance().logInfo("ERROR READ NETWORK CONFIG");
  isAccessPoint = config.confMode;
  useDHCP = config.dhcp;
  convertCharToIPAddress(config.ip, local_IP);
  convertCharToIPAddress(config.gateway, gateway);
  convertCharToIPAddress(config.subnet, subnet);
  convertCharToIPAddress(config.dns, dns);
}

void WiFiManager::connectToWiFi() {
  Logger::getInstance().logInfo("START WIFI CLIENT");
  WiFi.mode(WIFI_STA);
  WiFi.begin(config.ssid, config.password);

  if (!useDHCP) {
    if (!WiFi.config(local_IP, gateway, subnet, dns)) {
      Logger::getInstance().log(LOG_ERROR, "NETWORK STATIC CONFIG FAILED");
    }
  }

  Logger::getInstance().logInfo("WIFI CONNECTING");
   int retries = 0;
   while (WiFi.status() != WL_CONNECTED && retries < 20) {
     delay(500);
     retries++;
     updateLED();
    }

   if (WiFi.status() == WL_CONNECTED) {
    String infoWiFi = "WIFI CONNECTED: " + String(config.ssid);
    Logger::getInstance().logInfo(infoWiFi.c_str());
    String ipString = "IP: " + WiFi.localIP().toString();
    Logger::getInstance().logInfo(ipString.c_str());
    String gatewayInfo = "GATEWAY: " + WiFi.gatewayIP().toString();
    Logger::getInstance().logInfo(gatewayInfo.c_str());
    String dnsInfo = "DNS: " + WiFi.dnsIP().toString();
    Logger::getInstance().logInfo(dnsInfo.c_str());
   } else {
    String infoWiFi = "WIFI CONNECTION ERROR: " + String(config.ssid);
    Logger::getInstance().log(LOG_ERROR, infoWiFi.c_str());
    ledBlink();
  }
  updateLED();
}

void WiFiManager::setupAccessPoint(const char *newSSID, const char *newPassword) {
  Logger::getInstance().logInfo("START DEVICE AP CONFIG MODE");
  // Disable store WIFI config in flash memory
  //WiFi.persistent(false);
  // Remove WIFI config from chip
  //WiFi.disconnect(true);
  //delay(1000);
  //WiFi.eraseAP();
  //WiFi.enableAP(false);
  //WiFi.enableAP(true);
  WiFi.mode(WIFI_AP);
  WiFi.softAPsetHostname(config.hostname);
  WiFi.softAP(newSSID, newPassword);
  delay(1000);
  String ssi = "AP SSID: " + String(WiFi.softAPSSID());
  Logger::getInstance().logInfo(ssi.c_str());
  String sspass = "AP PASS: " + String(newPassword);
  Logger::getInstance().logInfo(sspass.c_str());

  IPAddress IP = WiFi.softAPIP();
  String ipString = "AP IP: " + IP.toString();
  Logger::getInstance().logInfo(ipString.c_str());
  digitalWrite(LED_PIN, HIGH); // LED on for CONFIG AP mode
  setupMDNS();
  getRSSI();
}

bool WiFiManager::isWiFiOK(){
  if (WiFi.status() != WL_CONNECTED)
   return false;
  else
   return true;
}

void WiFiManager::checkWiFiConnection() {
  if (!isAccessPoint) {
     getRSSI();
     if (WiFi.status() != WL_CONNECTED) {
       String infoWiFi = "WIFI RECONNECTING: " + String(config.ssid);
       Logger::getInstance().log(LOG_INFO, infoWiFi.c_str());
       WiFi.reconnect();
       int retries = 0;
       while (WiFi.status() != WL_CONNECTED && retries < 20) {
         delay(500);
         retries++;
         ledBlink();
         updateLED();
        }
       if (WiFi.status() == WL_CONNECTED) {
         String infoWiFi = "WIFI RECONNECTED: " + String(config.ssid);
         Logger::getInstance().logInfo(infoWiFi.c_str());
         String ipString = "IP: " + WiFi.localIP().toString();
         Logger::getInstance().logInfo(ipString.c_str());
         getRSSI();
         setupMDNS();
      } else {
        String infoWiFi = "WIFI ERROR RECONECT: " + String(config.ssid);
        Logger::getInstance().log(LOG_ERROR, infoWiFi.c_str());
        getRSSI();
      }
    }
    updateLED();
  }
}

int WiFiManager::rssiToPercent(int rssi) {
  if (rssi <= -100) {
    return 0;
  } else 
      if (rssi >= -50) {
        return 100;
      }
  else {
    return 2 * (rssi + 100);
  }
}

void WiFiManager::updateLED() {
  getRSSI();
  if (WiFi.status() == WL_CONNECTED) {
     int8_t rssi = WiFi.RSSI();
     if (rssi > -70) {
       digitalWrite(LED_PIN, HIGH); // Strong signal
  } else {
    String signalInfo = "WIFI WEAK SIGNAL: " + String(rssi) + " " + rssiToPercent(rssi) + "%";
    Logger::getInstance().log(LOG_WARNING, signalInfo.c_str());
    ledBlink();
  } } else {
     digitalWrite(LED_PIN, LOW); // No Wifi signal
  }
}

int8_t WiFiManager::getRSSI() {
   if (WiFi.status() == WL_CONNECTED) {
     rssi = WiFi.RSSI();
     return rssi;
  }
  return 0;
}

void WiFiManager::setupMDNS() {
  Logger::getInstance().log(LOG_INFO, "START MDNS");
  if (!MDNS.begin(config.hostname)) {
    Logger::getInstance().log(LOG_ERROR, "MDNS ERROR");
  } else {
    String mdnsstr = "MDNS: http://" + String(config.hostname) + ".local";
    Logger::getInstance().logInfo(mdnsstr.c_str());
  }
}

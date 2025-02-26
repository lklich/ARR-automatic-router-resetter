#include "www.h"

AsyncWebServer server(80);
size_t content_len;
bool rebooting = false;

void handleSSIDJson(AsyncWebServerRequest *request) {
  Logger::getInstance().log(LOG_INFO, "SCANNING WIFI NETWORKS");
  int n = WiFi.scanComplete();
  if (n == -2) {
    request->send(200, "application/json", "{\"status\":\"scanning\"}");
    return;
  }
  if (n == -1) {
    WiFi.scanNetworks(true);  // start asynchronicznego skanowania
    request->send(200, "application/json", "{\"status\":\"started scan\"}");
    return;
  }
  
  // Gdy skanowanie zostało zakończone (n >= 0), budujemy wynikowy JSON
  DynamicJsonDocument doc(1024);
  JsonArray networks = doc.createNestedArray("networks");
  for (int i = 0; i < n; i++) {
    JsonObject network = networks.createNestedObject();
    network["ssid"] = WiFi.SSID(i);
    network["rssi"] = WiFi.RSSI(i);
  }
  
  String jsonOutput;
  serializeJson(doc, jsonOutput);
  request->send(200, "application/json", jsonOutput);
  
  // Usuwamy wyniki poprzedniego skanowania i od razu rozpoczynamy nowe
  WiFi.scanDelete();
  WiFi.scanNetworks(true);
}

void fsStart(){
  if (!LittleFS.begin()) {
    Logger::getInstance().log(LOG_ERROR, "FS MOUNT ERROR");
    while (true) { 
      delay(4000); 
      digitalWrite(LED_PIN, HIGH); delay(200); 
      digitalWrite(LED_PIN, LOW); delay(200);
      digitalWrite(LED_PIN, HIGH); delay(200); 
      digitalWrite(LED_PIN, LOW); delay(200);   
    }
  } else {
    Logger::getInstance().log(LOG_INFO, "FILESYSTEM OK");
    // Getting filesystem info
    size_t totalBytes = LittleFS.totalBytes();   // Total size
    size_t usedBytes = LittleFS.usedBytes();     // Used 
    size_t freeBytes = totalBytes - usedBytes;   // Free

    // Calculate MB
    float totalMB = totalBytes / 1048576.0;  // 1 MB = 1024 * 1024 = 1048576 bytes
    float usedMB = usedBytes / 1048576.0;
    float freeMB = freeBytes / 1048576.0;

    // Show results in terminal and log file
    String tmp = "STORAGE MEM: " + String(totalMB) + "MB";
    Logger::getInstance().log(LOG_INFO, tmp.c_str());
    tmp = "STORAGE USED: " + String(usedMB) + "MB";
    Logger::getInstance().log(LOG_INFO, tmp.c_str());
    tmp = "STORAGE FREE: " + String(freeMB) + "MB";
    Logger::getInstance().log(LOG_INFO, tmp.c_str());
  }
}

String readFile(const char *path) {
  File file = LittleFS.open(path, "r");
  Logger::getInstance().log(LOG_INFO, path);
  if (!file) {
    Logger::getInstance().log(LOG_ERROR, "ERROR OPEN FILE");
    return "";
  }
  String content = file.readString();
  file.close();
  return content;
}

void routingError() {
  Logger::getInstance().log(LOG_INFO, "ROUTING ERROR");
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = readFile("/error.html");
    html.replace("{{ip}}", WiFi.localIP().toString());
    html.replace("{{version}}", String(VERSION));
    html.replace("{{name}}", String(DEVICENAME));
    html.replace("{{wifi}}", String(act_rssi_percent) + "%");
    html.replace("{{iptest}}", String(config.iptest));
    html.replace("{{timeout}}", String(config.timeout));
    request->send(200, "text/html", html);
  });
  
  server.serveStatic("/", LittleFS, "/").setDefaultFile("error.html");
  server.onNotFound(notFound);

  // CorsORIGIN - for AJAX correct from outside
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*"); 
  Logger::getInstance().log(LOG_INFO, "RUN WWW ERROR");
  server.begin();
}

void routing() {
  Logger::getInstance().log(LOG_INFO, "START ROUTING");
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = readFile("/index.html");
    String logmessage = "REQUEST: " + request->client()->remoteIP().toString();
    Logger::getInstance().log(LOG_INFO, logmessage.c_str());
    html.replace("{{ip}}", WiFi.localIP().toString());
    html.replace("{{iptest}}", String(config.iptest));
    html.replace("{{ssid}}", String(config.ssid));
    html.replace("{{timeout}}", String(config.timeout));
    html.replace("{{timeoffset}}", String(config.timeoffset));
    html.replace("{{version}}", String(VERSION));
    html.replace("{{name}}", String(DEVICENAME));
    html.replace("{{wifi}}", String(act_rssi_percent) + "%");
    html.replace("{{state}}", String(connected));
    if (config.enable == true)
      html.replace("{{enable}}", "Yes");
    else
      html.replace("{{enable}}", "No");
    html.replace("{{place}}", String(config.place));
    request->send(200, "text/html", html);
  });

  server.on("/log", HTTP_GET, [](AsyncWebServerRequest *request) {
    String logmessage = "REQUEST: " + request->client()->remoteIP().toString();
    Logger::getInstance().log(LOG_INFO, logmessage.c_str());
    if (!request->authenticate(config.user, config.pass)) {
      return request->requestAuthentication();
    }
    String html = readFile("/log.html");
    html.replace("{{version}}", String(VERSION));
    html.replace("{{name}}", String(DEVICENAME));
    html.replace("{{place}}", String(config.place));
    request->send(200, "text/html", html);
  });

  // Return JSON with SSID and RSSI
  server.on("/ssid", HTTP_GET, handleSSIDJson);
  
  server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request) {
    Logger::getInstance().log(LOG_INFO, "START CONFIG");
    if (!request->authenticate(config.user, config.pass)) {
      return request->requestAuthentication();
    }
    request->send(LittleFS, "/setup.html", String(), false, proconfig);
  });
  
  // Load CSS from FS
  server.on("/style", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/pico.min.css", "text/css");
  });

  // Load config JS from FS
  server.on("/configjs", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/config.js", "text/javascript");
  });

  // Load normal JS from FS
  server.on("/scriptjs", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/script.js", "text/javascript");
  });

// Load LOG file from FS
 server.on("/logtxt", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(LittleFS, "/log.txt", "text/plain");
 });

// Load NETTEST.TXT file from FS
 server.on("/nettest", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(LittleFS, "/nettest.txt", "text/plain");
 });


  server.on("/reboot", HTTP_GET, [](AsyncWebServerRequest * request) {
    isRebootRequired = true;
    request->redirect("/");
  });

  server.on("/clearlog", HTTP_GET, [](AsyncWebServerRequest * request) {
    Logger::getInstance().log(LOG_INFO, "CLEAR TEST LOG");
    if (!request->authenticate(config.user, config.pass)) {
      return request->requestAuthentication();
    }
    isClearLog = true;
    request->redirect("/");
  });

  server.on("/logout", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Logged out");
    response->addHeader("Set-Cookie", "session_id=; expires=Thu, 01 Jan 1970 00:00:00 GMT");
    request->send(401);
  });

  server.on("/logged-out", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->redirect("/");
  });

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request) {
    String en = "";
    String conn = "0";
    if (config.enable) en = "Tak"; else en = "Nie";
    if (connected) conn = "1"; else conn = "0";
    String resp = "{ \"date\":\"" + actDate + "\",  \"time\":\"" + actTime + "\",  \"enable\":\"" + en + "\", \"rssi\":\"" + String(rssi) 
                    + "\", \"rssip\":\"" + String(act_rssi_percent) + "\", \"connected\":\"" + String(connected) 
                    + "\", \"countok\":\"" + String(countConnect) + "\", \"counterr\":\"" + String(countDisconnect) 
                    + "\", \"started\":\"" + String(started) + "\" }"; 
      request->send_P(200, "text/plain", resp.c_str());
    });

  // Save config from HTML form to EEPROM
  server.on("/configsave", HTTP_GET, [](AsyncWebServerRequest *request) {
    Logger::getInstance().log(LOG_INFO, "SAVE EEPROM");
    strcpy(config.ssid, request->arg("ssid").c_str());
    strcpy(config.password, request->arg("wifipass").c_str());
    strcpy(config.place, request->arg("place").c_str());
    //strcpy(config.hostname, request->arg("hostname").c_str());
    strcpy(config.user, request->arg("user").c_str());
    strcpy(config.pass, request->arg("pass").c_str());
    strcpy(config.place, request->arg("place").c_str());
    config.enable = (request->arg("enabled") == "on") ? 1 : 0;
    config.dhcp = (request->arg("dhcp") == "on") ? 1 : 0;
    strcpy(config.iptest, request->arg("iptest").c_str());
    strcpy(config.ip, request->arg("ip").c_str());
    strcpy(config.subnet, request->arg("netmask").c_str());
    strcpy(config.gateway, request->arg("gateway").c_str());
    strcpy(config.dns, request->arg("dns").c_str());
    config.relayTime = request->arg("time").toInt();   
    //config.timeoffset = request->arg("timeoffset").toInt();
    config.timeout = request->arg("timeout").toInt();
    config.starttest = request->arg("starttest").toInt(); 
    config.mqtt = (request->arg("mqtt") == "on") ? 1 : 0;
    config.mqttPort = request->arg("mqttport").toInt(); 
    strcpy(config.mqttBroker, request->arg("mqttbroker").c_str());
    strcpy(config.mqttUser, request->arg("mqttuser").c_str());
    strcpy(config.mqttPass, request->arg("mqttpass").c_str());
    config.confMode = 0; // Wyłącz tryb AP       
    EEPROM.put(0, config); 
    EEPROM.write(EEPROM_SIZE-1, 252);
    if (EEPROM.commit()) {
      Logger::getInstance().log(LOG_INFO, "CONFIG SAVED");
    } else {
      Logger::getInstance().log(LOG_ERROR, "CONFIG SAVE ERROR");
    }
    EEPROM.end(); 
    isRebootRequired = true;
    request->redirect("/");
  });

  ///////////////////////////////////////////////////////////////////////////////////////////
  // OTA update configuration 
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
    Logger::getInstance().log(LOG_INFO, "REQUEST UPDATE");
    if (!request->authenticate(config.user, config.pass)) {
      return request->requestAuthentication();
    }
    String html = readFile("/update.html");
    html.replace("{{version}}", String(VERSION));
    html.replace("{{name}}", String(DEVICENAME));
    html.replace("{{place}}", String(config.place));
    request->send(200, "text/html", html);
  });

// OTA Endpoint - secure with Basic Auth
server.on("/doUpdate", HTTP_POST, [](AsyncWebServerRequest *request) {
  if (!request->authenticate(config.user, config.pass)) {
    return request->requestAuthentication();
  }
  Logger::getInstance().log(LOG_INFO, "UPDATE UPLOAD FIRMWARE");
  isRebootRequired = !Update.hasError();
  AsyncWebServerResponse *response = request->beginResponse(200, "text/html", isRebootRequired ? "<h1><strong>Aktualizacja zakończona pomyślnie</strong></h1><br><a href='/'>Strona główna</a>" : "<h1><strong>Błąd aktualizacji</strong></h1><br><a href='/doUpdate'>Ponów?</a>");
  response->addHeader("Connection", "close");
  request->send(response); }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
if (!index) {
  Logger::getInstance().log(LOG_INFO, "UPDATING");
  if (!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)) {
    Logger::getInstance().log(LOG_ERROR, "UPDATE ERROR EUP03");
  }
}
if (!Update.hasError()) {
  if (Update.write(data, len) != len) {
    Logger::getInstance().log(LOG_ERROR, "UPDATE ERROR EUP04");
  }
}
if (final) {
  if (Update.end(true)) {
    Logger::getInstance().log(LOG_INFO, "UPDATE SUCCESS");
  } else {
    Update.printError(Serial);
    Logger::getInstance().log(LOG_ERROR, "ERROR UPDATE EUP05");
  }
} 
});


  // CorsORIGIN - for outsite AJAX
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*"); 
  Logger::getInstance().log(LOG_INFO, "START WWW SERVER");
  server.begin();
}

void notFound(AsyncWebServerRequest *request) {
  Logger::getInstance().log(LOG_ERROR, "404 - NOT EXISTS");
  request->send(404, "text/plain", "Strona nie istnieje :(");
}

// Set HTML template variables
String proconfig(const String &var) {
  if (var == "SSID") {
    return String(config.ssid);
  } else if (var == "WIFIPASS") {
    return String(config.password);
  } else if (var == "USER") {
    return String(config.user);
  } else if (var == "PASS") {
    return String(config.pass);
  } else if (var == "VERSION") {
    return VERSION;
  } else if (var == "IP") {
    return String(config.ip);
  } else if (var == "NETMASK") {
    return String(config.subnet);
  } else if (var == "GATEWAY") {
    return String(config.gateway);
  } else if (var == "DNS") {
    return String(config.dns);
  } else if (var == "DHCP") {
    return String(config.dhcp);
  } else if (var == "HOSTNAME") {
    return String(config.hostname);
  } else if (var == "NAME") {
    return DEVICENAME;
  } else if (var == "ENABLED") {
    return String(config.enable);
  } else if (var == "PLACE") {
    return String(config.place);
  } else if (var == "TIME") {
    return String(config.relayTime);
  } else if (var == "TIMEOUT") {
    return String(config.timeout);
  } else if (var == "STARTTEST") {
      return String(config.starttest);
  } else if (var == "IPTEST") {
    return String(config.iptest);
  } else if (var == "CONNECTED") {
    return String(connected);
  } else if (var == "TIMEOFFSET") {
    return String(config.timeoffset);
  } else if (var == "MQTTBROKER") {
    return String(config.mqttBroker);
  } else if (var == "MQTT") {
    return String(config.mqtt);
  } else if (var == "MQTTUSER") {
    return String(config.mqttUser);
  } else if (var == "MQTTPASS") {
    return String(config.mqttPass);
  } else if (var == "MQTTPORT") {
    return String(config.mqttPort);
  }
  return String();
}

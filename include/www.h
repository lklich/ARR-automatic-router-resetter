#ifndef WWW_H 
#define WWW_H

#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#include <Update.h>
#elif defined(ESP8266)
#include <ESPAsyncTCP.h>
#include <ESP8266WiFi.h>
#include <Updater.h>
#endif

#include <config.h>
#include <logger.h>

#include <LittleFS.h>
#include <pinout.h>
#include "ESPAsyncWebServer.h"
#include <ArduinoJson.h>
#include "version.h"
#include <time.h>

#define U_PART U_FS

extern String act_rssi_percent;

void fsStart();
String proconfig(const String &var);
String prosettings(const String &var);
String readFile(const char *path);
void notFound(AsyncWebServerRequest *request);
void onRequest(AsyncWebServerRequest *request);
void routing();
void routingError();
void handleUpdate(AsyncWebServerRequest *request);
void handleRoot(AsyncWebServerRequest *request);
void handleDoUpdate(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final);
void printProgress(size_t prg, size_t sz);
void handleSSIDJson(AsyncWebServerRequest *request);
#endif
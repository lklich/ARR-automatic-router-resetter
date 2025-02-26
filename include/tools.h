#ifndef TOOLS_H
#define TOOLS_H

#include <Arduino.h>
#include <math.h>
#include <Wire.h>
#include <pinout.h>
#include <logger.h>
#ifdef ESP32
  #include <ESP.h> // Dla ESP32
#else
  #include <ESP8266WiFi.h> // Dla ESP8266
#endif


class Tools {
  public:
    Tools();
    void displayWelcome();  // Wyświetla powitanie początkowe
    void checkFlash();      // Kontrola i informacje o pamięci flash
    void getChipType();     // Wyświetla informacje o chipie.
    
  private:
    double CalculateSquare(int number);
};

#endif // TOOLS_H
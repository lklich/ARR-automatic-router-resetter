#ifndef RELAY_H
#define RELAY_H

#include <Arduino.h>
#include <pinout.h>
#include <logger.h>
#include <config.h>  // do odczytu konfiguracji - config.relayLength (czas włączenia przekaźnika)

// w main musi być w loop: relay.update();

class Relay {
  public:
    // Konstruktor
    Relay(int pin);

    // Inicjalizacja przekaźnika
    void begin();

    // Włącza przekaźnik
    void turnOn();

    // Wyłącza przekaźnik
    void turnOff();

    // Sprawdza stan przekaźnika
    bool isOn();

    void update();
     
  private:
    int _pin;
    bool _state;
    unsigned long relayOnTime;
    unsigned long relayDuration;
};

#endif // RELAYMANAGER_H

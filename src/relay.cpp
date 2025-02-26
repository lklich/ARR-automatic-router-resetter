#include "relay.h"

// Konstruktor
Relay::Relay(int pin) : _pin(pin), _state(false) {}

// Inicjalizacja przekaźnika
void Relay::begin() {
  Logger::getInstance().log(LOG_INFO, "BEGIN RELAY");
  pinMode(_pin, OUTPUT);
  turnOff(); // Ustawienie domyślnego stanu na wyłączony przekaźnik typu HIGH->ON
}

// Delaj bez delay, aby nie zatrzymywać systemu podczas dzwonienia
void Relay::update() {
  if (_state && (millis() - relayOnTime >= relayDuration)) {
    turnOff();
  }
}

// Włącza przekaźnik na czas z config.relayTime
void Relay::turnOn() {
  Logger::getInstance().log(LOG_INFO, "ON RELAY");
  digitalWrite(_pin, HIGH);
  relayOnTime = millis();
  _state = true;
  relayDuration = config.relayTime * 1000; // w config są sekundy, czas relayDuration to ms -> konwertuj na sekundy
}

// Wyłącza przekaźnik
void Relay::turnOff() {
  Logger::getInstance().log(LOG_INFO, "OFF RELAY");
  digitalWrite(_pin, LOW);
  _state = false;
}

// Sprawdza stan przekaźnika
bool Relay::isOn() {
  return _state;
}
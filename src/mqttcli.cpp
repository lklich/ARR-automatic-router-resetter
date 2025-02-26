#include <mqttcli.h>

MQTTCli::MQTTCli() : mqtt_server(nullptr), mqtt_port(1883), mqtt_user(nullptr), mqtt_password(nullptr), hostname(nullptr), client(espClient), 
   connectionAttempts(0), enabled(false) {}


   void MQTTCli::begin(const char* mqtt_server, uint16_t mqtt_port, const char* mqtt_user, const char* mqtt_password, const char* hostname) {
    if (!enabled) return;
    this->mqtt_server = mqtt_server;
    this->mqtt_port = mqtt_port;
    this->mqtt_user = mqtt_user;
    this->mqtt_password = mqtt_password;
    this->hostname = hostname;

    // Dynamic generate MQTT topic from hostname for Home Assisstant
    mqtt_topic = "homeassistant/switch/" + String(hostname) + "/state";
    config_topic = "homeassistant/switch/" + String(hostname) + "/config";
    availability_topic = "homeassistant/switch/" + String(hostname) + "/availability";

    client.setServer(mqtt_server, mqtt_port);
    connectMQTT();
}

void MQTTCli::connectMQTT() {
  if (!enabled) return; // If disabled, do nothing

  String inn = "";
  while (!client.connected() && connectionAttempts < 4) {
      inn = "MQTT CONNECTING TRY: " + String(connectionAttempts + 1);
      Logger::getInstance().log(LOG_INFO, inn.c_str());

      if (client.connect(hostname, mqtt_user, mqtt_password)) {
          Logger::getInstance().log(LOG_INFO, "MQTT CONNECTED");
          sendDiscoveryConfig();
          client.publish(availability_topic.c_str(), "online", true);
          connectionAttempts = 0;  // Reset connection counter
          return;
      } else {
          inn = "MQTT CONNECT ERROR: " + String(client.state());
          Logger::getInstance().log(LOG_ERROR, inn.c_str());
          Serial.println(client.state());
          connectionAttempts++;
          delay(2000); // how many time delay for next MQTT connect
      }
  }

  if (connectionAttempts >= 4) {
      Logger::getInstance().log(LOG_ERROR, "MQTT MAX TRY CONNECTIONS");
      enabled = false;
  }
}

void MQTTCli::sendDiscoveryConfig() {
  String configPayload = "{"
      "\"name\": \"" + String(hostname) + "\","
      "\"uniq_id\": \"" + String(hostname) + "\","
      "\"stat_t\": \"" + mqtt_topic + "\","
      "\"avty_t\": \"" + availability_topic + "\","
      "\"pl_on\": \"ON\","
      "\"pl_off\": \"OFF\","
      "\"dev\": {"
          "\"name\": \"" + String(hostname) + "\","
          "\"identifiers\": [\"" + String(hostname) + "\"],"
          "\"manufacturer\": \"ESPHome Custom\","
          "\"model\": \"ESP32 MQTT Plug\","
          "\"sw_version\": \"1.0.0\","
          "\"suggested_area\": \"Salon\","
          "\"hw_version\": \"Rev A\""
      "},"
      "\"icon\": \"mdi:power-socket-eu\""
  "}";

  client.publish(config_topic.c_str(), configPayload.c_str(), true);
}

bool MQTTCli::publishState(bool state) {
  if (!enabled || !client.connected()) {
      Logger::getInstance().log(LOG_INFO, "MQTT DISABLED OR NOT CONNECT");
      return false;
  }

  client.publish(mqtt_topic.c_str(), state ? "ON" : "OFF", true);
  return true;
}

void MQTTCli::loop() {
  if (!enabled) return;

  if (!client.connected()) {
      connectMQTT();
  }
  client.loop();
}
#ifndef MQTTCLI_H
#define MQTTCLI_H

#include <WiFiClient.h>
#include <PubSubClient.h>
#include <logger.h>
#include <version.h>

class MQTTCli {
    public:
        bool enabled = false;

        MQTTCli();
        void begin(const char* mqtt_server, uint16_t mqtt_port, const char* mqtt_user, const char* mqtt_password, const char* hostname);
        void loop();
        bool publishState(bool state);

    private:
        const char* mqtt_server;
        uint16_t mqtt_port;
        const char* mqtt_user;
        const char* mqtt_password;
        const char* hostname;  // Unikalna nazwa hosta dla MQTT

        String mqtt_topic;
        String config_topic;
        String availability_topic;

        WiFiClient espClient;
        PubSubClient client;
        int connectionAttempts;  // Licznik prób połączenia

        void connectMQTT();
        void sendDiscoveryConfig();
  };
#endif
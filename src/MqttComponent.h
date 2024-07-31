#ifndef __MQTT_COMPONENT_H__
#define __MQTT_COMPONENT_H__

#include <Arduino.h>
#include <PubSubClient.h>
#include "Specific_ESP_Wifi.h"

#include "components.h"

class MqttComponent: public Component {
  private:
    String _username;
    String _password;
    String _clientId;
    PubSubClient _mqttClient;
    std::function<void(PubSubClient *)> const _onConnected;
    unsigned long _intervalMs;
    unsigned long _lastCheckTime;
    String _willTopic;
    String _willMessage;
    bool _willRetain; 
    uint8_t _willQos;

    void reconnect();

  public:
    MqttComponent(
      Client *client, 
      const char *broker, 
      uint16_t portNumber,
      const char *username,
      const char *password,
      const char *clientId,
      std::function<void(PubSubClient *)> const onConnected = NULL,
      std::function<void(const char *topic, const byte *payload, unsigned int length)> const onReceived = NULL,
      unsigned long intervalMs = 30000,
      const char *willTopic = NULL,
      const char *willMessage = NULL,
      bool willRetain = true, 
      uint8_t willQos = MQTTQOS0
    );
    PubSubClient *mqttClient() { return &this->_mqttClient; }

    void setup();
    void loop();
};
#endif
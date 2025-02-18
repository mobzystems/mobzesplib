#ifndef __MQTT_APPLICATION_H__
#define __MQTT_APPLICATION_H__

#include "Application.h"
#include "MqttComponent.h"
#include "MqttLogComponent.h"

#include <WiFiClientSecure.h>

class MqttApplication: public Application {
private:
  static MqttApplication *_app;

  MqttComponent *_mqtt = NULL;
  String _mqttPrefix;
  String _onlinetopic;
  long _loopCount;
  long _autoRestartTimeout;
  bool _isFirstConnect;

  MqttLogComponent *_mqttLog = NULL;

  std::function<void(PubSubClient *client)> const _onMqttConnected;
  std::function<void(const char *topic, const byte *payload, unsigned int length)> const _onMqttReceived;

  WiFiClientSecure _wifiSecure;

public:
  MqttApplication(const char *title, const char *version, const char *mqttPrefix, uint16_t otaPortNumber = 80, const char *configuration = NULL);
  MqttApplication(const char *title, const char *version, const char *mqttPrefix, std::function<void(PubSubClient *client)> const onConnected, std::function<void(const char *topic, const byte *payload, unsigned int length)> const onReceived, uint16_t otaPortNumber = 80, const char *configuration = NULL);

  MqttComponent *mqtt() { return this->_mqtt; }

  void setup();
  void loop();

  void setBootTimeUtc(time_t utc);

  void publishData(const char *channel, const char *property, const char *value, bool retained);
  void publishProperty(const char *property, const char *value, bool retained = false);

  MqttLogComponent *mqttLog() { return this->_mqttLog; }
  
  // WiFiClientSecure *client() { return &this->_wifiSecure; }

  // void onMqttConnected(void (*onConnected)()) { this->_onMqttConnected = onConnected; }
  // void onMqttReceived(void (*onReceived)(const char *topic, const byte *payload, unsigned int length)) { this->_onMqttReceived = onReceived; }
};
#endif

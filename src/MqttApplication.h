#ifndef __MQTT_APPLICATION_H__
#define __MQTT_APPLICATION_H__

#include "Application.h"
#include "MqttComponent.h"

class MqttApplication: public Application {
private:
  static MqttApplication *_app;

  MqttComponent *_mqtt = NULL;
  String _mqttPrefix;
  String _onlinetopic;
  long _loopCount;
  long _autoRestartTimeout;

  void (*_onMqttConnected)();
  void (*_onMqttReceived)(const char *topic, const byte *payload, unsigned int length);

public:
  MqttApplication(const char *title, const char *version, const char *mqttPrefix, uint16_t otaPortNumber = 80, size_t maxConfigValues = 50);
  MqttApplication(const char *title, const char *version, const char *mqttPrefix, void (*onConnected)(), void (*onReceived)(const char *topic, const byte *payload, unsigned int length), uint16_t otaPortNumber = 80, size_t maxConfigValues = 50);

  MqttComponent *mqtt() { return this->_mqtt; }

  void setup();
  void loop();

  void setBootTimeUtc(time_t utc);

  void publishData(const char *channel, const char *property, const char *value, bool retained);
  void publishProperty(const char *property, const char *value, bool retained = false);

  // void onMqttConnected(void (*onConnected)()) { this->_onMqttConnected = onConnected; }
  // void onMqttReceived(void (*onReceived)(const char *topic, const byte *payload, unsigned int length)) { this->_onMqttReceived = onReceived; }
};
#endif

#include "MqttLogComponent.h"

MqttLogComponent::MqttLogComponent(MqttComponent *mqtt, const char *topic, int size, Log::LOGLEVEL level)
  : Component("MqttLog"),
  _mqttLogger(new MqttLogger(mqtt, topic, size, level))
{
  // Add the MQTT logger to the logger list
  Log::addLogger(this->_mqttLogger);
}

void MqttLogComponent::setup() {
  // Nothing
}

void MqttLogComponent::loop() {
  _mqttLogger->loop();
}

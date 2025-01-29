#ifndef __MQTTLOG_COMPONENT_H__
#define __MQTTLOG_COMPONENT_H__

#include <Arduino.h>
#include "components.h"
#include "MqttComponent.h"
#include "MqttLogger.h"

class MqttLogComponent: public Component {
  protected:
    MqttLogger *_mqttLogger;

  public:
    MqttLogComponent(MqttComponent *mqtt, const char *topic, int size, Log::LOGLEVEL level);

    void setup();
    void loop();

    MqttLogger *logger() { return this->_mqttLogger; }
};
#endif
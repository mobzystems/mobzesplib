#ifndef _DHT_COMPONENT_H__
#define _DHT_COMPONENT_H__

#include <Arduino.h>
#include "components.h"
#include <DHT.h>

class DHTComponent: public Component {
  private:
    DHT _dht;
    uint8_t _pullupTimeUs;

  public:
    DHTComponent(uint8_t pinNumber, uint8_t dhtType, uint8_t sensorCount = 6, uint8_t pullupTimeUs = 55);

    void setup();
    void loop();

    float readTemperature(bool fahrenheit = false, bool force = false) { return this->_dht.readTemperature(fahrenheit, force); }
    float readHumidity(bool force = false) { return this->_dht.readHumidity(force); }
};
#endif
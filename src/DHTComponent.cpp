#include <Adafruit_Sensor.h>
#include <DHT.h>

#include "DHTComponent.h"

DHTComponent::DHTComponent(uint8_t pinNumber, uint8_t dhtType, uint8_t sensorCount, uint8_t pullupTimeUs) :
  Component("DHT"),
  _dht(pinNumber, dhtType, sensorCount),
  _pullupTimeUs(pullupTimeUs)
{}

void DHTComponent::setup()
{
  this->_dht.begin(this->_pullupTimeUs);
}

void DHTComponent::loop() {}
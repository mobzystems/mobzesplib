#include "BluetoothSerialComponent.h"

BluetoothSerialComponent::BluetoothSerialComponent(const char *hostname, void (*onDataAvailable)(int)) :
  Component("BTSerial"),
  _hostname(hostname),
  _onDataAvailable(onDataAvailable)
{
}

void BluetoothSerialComponent::setup() {
  this->_serialBT.begin(this->_hostname);
}

void BluetoothSerialComponent::loop() {
  // Read a single byte from the data stream (if available) and pass it to _onDataAvailable
  if (this->_serialBT.available())
    this->_onDataAvailable(this->_serialBT.read());
}
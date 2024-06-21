#include "PinWatcherComponent.h"
#include "logging.h"

DigitalPinWatcherComponent::DigitalPinWatcherComponent(uint8_t pinNumber, uint8_t inputMode, void (*onValueChanged)(int, int)) :
  Component("DigitalPin"),
  _pinNumber(pinNumber),
  _inputMode(inputMode),
  _onValueChanged(onValueChanged)
{}

void DigitalPinWatcherComponent::setup() {
  // Configure the ping
  pinMode(this->_pinNumber, this->_inputMode);
  this->_lastValue = digitalRead(this->_pinNumber);
  Log::logDebug("[%s] Pin %d is initially %d", this->name(), this->_pinNumber, this->_lastValue);
}

void DigitalPinWatcherComponent::loop() {
  int currentValue = digitalRead(this->_pinNumber);
  if (currentValue != this->_lastValue) {
    Log::logDebug("[%s] Pin %d is now %d", this->name(), this->_pinNumber, currentValue);
    this->_onValueChanged(this->_lastValue, currentValue);
    this->_lastValue = currentValue;
  }
}

AnalogPinWatcherComponent::AnalogPinWatcherComponent(uint8_t pinNumber, void (*onValueChanged)(uint16_t, uint16_t)) :
  Component("AnalogPin"),
  _pinNumber(pinNumber),
  _onValueChanged(onValueChanged)
{}

void AnalogPinWatcherComponent::setup() {
  this->_lastValue = analogRead(this->_pinNumber);
  Log::logDebug("[%s] Pin %d is initially %d", this->name(), this->_pinNumber, this->_lastValue);
}

void AnalogPinWatcherComponent::loop() {
  uint16_t currentValue = analogRead(this->_pinNumber);
  if (currentValue != this->_lastValue) {
    Log::logDebug("[%s] Pin %d is now %d", this->name(), this->_pinNumber, currentValue);
    this->_onValueChanged(this->_lastValue, currentValue);
    this->_lastValue = currentValue;
  }
}

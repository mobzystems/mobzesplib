#include "RotaryEncoderWatcherComponent.h"

RotaryEncoderWatcherComponent::RotaryEncoderWatcherComponent(uint8_t pinNumber, uint8_t pinNumber2, RotaryEncoder::LatchMode latchMode, std::function<void(int, int)> const onValueChanged, const char *name) :
  PinWatcherComponent(name == NULL ? "RotaryEncoder" : name, pinNumber, onValueChanged),
  _encoder(pinNumber, pinNumber2, latchMode)
{
}

void RotaryEncoderWatcherComponent::setup() {
  this->_lastValue = this->_encoder.getPosition();
  // Report the initial value
  PinWatcherComponent::setup();
}

void RotaryEncoderWatcherComponent::loop() {
  this->_encoder.tick();
  int currentValue = this->_encoder.getPosition();
  if (currentValue != this->_lastValue) {
    Log::logDebug("[%s] Position is now %d", this->name(), currentValue);
    this->_onValueChanged(this->_lastValue, currentValue);
    this->_lastValue = currentValue;
  }
}
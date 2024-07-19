#include "PinWatcherComponent.h"
#include "logging.h"

DigitalPinWatcherComponent::DigitalPinWatcherComponent(uint8_t pinNumber, uint8_t inputMode, void (*onValueChanged)(int, int)) :
  Component("DigitalPin"),
  _pinNumber(pinNumber),
  _inputMode(inputMode),
  _onValueChanged(onValueChanged)
{}

void DigitalPinWatcherComponent::setup() {
  // Configure the pin
  pinMode(this->_pinNumber, this->_inputMode);
  // Read its initial value
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

DigitalClickWatcherComponent::DigitalClickWatcherComponent(uint8_t pinNumber, uint8_t inputMode, int detectionLevel, unsigned long maxInterval, void (*onClickDetected)(int numberOfclicks), int maxClicks) :
  Component("ClickWatcher"), 
  _pinNumber(pinNumber), 
  _inputMode(inputMode),
  _detectionLevel(detectionLevel),
  _maxInterval(maxInterval),
  _onClickDetected(onClickDetected),
  _maxClicks(maxClicks),
  _clickCount(0),
  _lastTimestamp(0)
{}

void DigitalClickWatcherComponent::setup() {
  // Configure the pin
  pinMode(this->_pinNumber, this->_inputMode);
  // Read its initial value
  this->_lastValue = digitalRead(this->_pinNumber);
  Log::logDebug("[%s] Pin %d is initially %d", this->name(), this->_pinNumber, this->_lastValue);
}

void DigitalClickWatcherComponent::loop() {
  unsigned long timestamp = millis();

  int currentValue = digitalRead(this->_pinNumber);
  if (currentValue != this->_lastValue) {
    // Do we now have the desired level (HIGH/LOW)?
    if (currentValue == this->_detectionLevel) {
      // Also: check that we're not within 100 ms of the last click
      if (this->_clickCount != 0 && timestamp - this->_lastTimestamp < 100) {
        Log::logTrace("[%s] Ignoring click on pin %d within %ld ms", this->name(), this->_pinNumber, timestamp - this->_lastTimestamp);

      } else {
        Log::logDebug("[%s] Click detected on pin %d (now %d)", this->name(), this->_pinNumber, currentValue);
        this->_clickCount++;    
        this->_lastTimestamp = timestamp;
      }
    } else {
      Log::logTrace("[%s] Pin %d back to starting level", this->name(), this->_pinNumber);
    }
    // Always keep this value
    this->_lastValue = currentValue;
  }

  // If we have detected one or more clicks, wait for the maximum interval to pass OR he maximum click count to be reached
  // If this happens, register the clicks and reset the click count
  if ((this->_maxClicks > 0 && this->_clickCount >= this->_maxClicks) || (this->_clickCount > 0 && timestamp > this->_lastTimestamp + this->_maxInterval)) {
    Log::logDebug("[%s] Detected %d click", this->name(), this->_clickCount);
    // We're done! Notify client
    this->_onClickDetected(this->_clickCount);
    // Reset the click count for the next series
    this->_clickCount = 0;
  }
}

AnalogPinWatcherComponent::AnalogPinWatcherComponent(uint8_t pinNumber, void (*onValueChanged)(uint16_t, uint16_t), uint16_t delta, unsigned long timeBetweenSamples) :
  Component("AnalogPin"),
  _pinNumber(pinNumber),
  _delta(delta),
  _timeBetweenSamples(timeBetweenSamples),
  _onValueChanged(onValueChanged),
  _lastReadTimestamp(millis())
{}

void AnalogPinWatcherComponent::setup() {
  this->_lastValue = analogRead(this->_pinNumber);
  Log::logDebug("[%s] Pin %d is initially %d", this->name(), this->_pinNumber, this->_lastValue);
}

void AnalogPinWatcherComponent::loop() {
  // Do not read analog ports too often. On d1 mini this can bring down the wifi!
  auto ms = millis();
  if (ms > this->_lastReadTimestamp + this->_timeBetweenSamples) {
    this->_lastReadTimestamp = ms;
    uint16_t currentValue = analogRead(this->_pinNumber);
    // Makesure uint16_t does now wrap below 0
    uint16_t minValue = this->_lastValue >= this->_delta ? this->_lastValue - this->_delta : 0;
    if (currentValue < minValue || currentValue > this->_lastValue + this->_delta) {
      Log::logDebug("[%s] Pin %d is now %d", this->name(), this->_pinNumber, currentValue);
      this->_onValueChanged(this->_lastValue, currentValue);
      this->_lastValue = currentValue;
    }
  }
}



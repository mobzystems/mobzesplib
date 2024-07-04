#ifndef __PINWATCHER_COMPONENT_H__
#define __PINWATCHER_COMPONENT_H__

#include "Components.h"

class DigitalPinWatcherComponent: public Component {
  private:
    uint8_t _pinNumber;
    uint8_t _inputMode;
    void (*_onValueChanged)(int, int);

    int _lastValue;

  public:
    DigitalPinWatcherComponent(uint8_t pinNumber, uint8_t inputMode, void (*onValueChanged)(int previous, int current));

    void setup();
    void loop();
};

class DigitalClickWatcherComponent: public Component {
  private:
    uint8_t _pinNumber;
    uint8_t _inputMode;
    int _detectionLevel; // HIGH or LOW
    unsigned long _maxInterval;
    void (*_onClickDetected)(int numberOfclicks);
    // Always register immediately when this number of clicks detected. 0: no maximum
    int _maxClicks;

    int _lastValue;
    int _clickCount;
    unsigned long _lastTimestamp;

  public:
    DigitalClickWatcherComponent(uint8_t pinNumber, uint8_t inputMode, int detectionLevel, unsigned long maxInterval, void (*onClickDetected)(int numberOfclicks), int maxClicks = 0);

    void setup();
    void loop();
};

class AnalogPinWatcherComponent: public Component {
  private:
    uint8_t _pinNumber;
    void (*_onValueChanged)(uint16_t, uint16_t);

    uint16_t _lastValue;

  public:
    AnalogPinWatcherComponent(uint8_t pinNumber, void (*onValueChanged)(uint16_t previous, uint16_t current));

    void setup();
    void loop();
};

#endif
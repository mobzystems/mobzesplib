#ifndef __PINWATCHERCOMPONENT_H__
#define __PINWATCHERCOMPONENT_H__

#include "components.h"

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
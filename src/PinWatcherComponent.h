#ifndef __PINWATCHER_COMPONENT_H__
#define __PINWATCHER_COMPONENT_H__

#include "Components.h"

template<typename T> class PinWatcherComponent: public Component {
  protected:
    // The pin number to read
    uint8_t _pinNumber;
    // The last value read
    T _lastValue;

    std::function<void(T previous, T current)> const _onValueChanged;

    PinWatcherComponent<T>(const char *name, uint8_t pinNumber, std::function<void(T previous, T current)> const onValueChanged):
      Component(name),
      _pinNumber(pinNumber),
      _onValueChanged(onValueChanged)
    {}
};

/*
  Analog pin watcher. Calls the provided onValueChanged function when the analog input value changes
*/
class DigitalPinWatcherComponent: public PinWatcherComponent<int> {
  protected:
    // The input pin's input mode (e.g. INPUT_PULLUP)
    uint8_t _inputMode;

  public:
    DigitalPinWatcherComponent(uint8_t pinNumber, uint8_t inputMode, std::function<void(int previous, int current)> const onValueChanged, const char *name = NULL);

    void setup();
    void loop();
};

/*
  Analog pin watcher. Calls the provided onValueChanged function when the analog input value changes
*/
class AnalogPinWatcherComponent: public PinWatcherComponent<uint16_t> {
  private:
    // The minimum value change that must occur before onValueChanged is called
    uint16_t _delta;
    // The minimum time between samples. When the analog pin is read too often, WiFi disconnects!
    unsigned long _timeBetweenSamples;
    // The time stamp of the last read
    unsigned long _lastReadTimestamp;

  public:
    AnalogPinWatcherComponent(uint8_t pinNumber, std::function<void(uint16_t previous, uint16_t current)> const onValueChanged, uint16_t delta, unsigned long timeBetweenSamples = 20);

    void setup();
    void loop();
};

class DigitalClickWatcherComponent: public DigitalPinWatcherComponent {
  private:
    // register a click on HIGH or LOW
    int _detectionLevel;
    // After this number of ms after the first click we always detect one
    unsigned long _maxInterval;
    std::function<void(int numberOfclicks)> const  _onClickDetected;
    // Always register immediately when this number of clicks detected. 0: no maximum
    int _maxClicks;
    // The number of clicks detected so far
    int _clickCount;
    // The time stamp of the last detected click
    unsigned long _lastTimestamp;

  public:
    DigitalClickWatcherComponent(uint8_t pinNumber, uint8_t inputMode, int detectionLevel, unsigned long maxInterval, std::function<void(int numberOfclicks)> const onClickDetected, int maxClicks = 0);

    void setup();
    void loop();
};

#endif
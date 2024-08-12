#ifndef __ROTARY_ENCODER_COMPONENT__
#define __ROTARY_ENCODER_COMPONENT__

#include "PinWatcherComponent.h"
#include <RotaryEncoder.h>

/*
  Rotary encoder watcher. Calls the provided onValueChanged function when the encoder's position changes
*/
class RotaryEncoderWatcherComponent: public PinWatcherComponent<int> {
  protected:
    // The second pin number, _pinNumber is the first
    uint8_t _pinNumber2;
    RotaryEncoder _encoder;
    
  public:
    RotaryEncoderWatcherComponent(uint8_t p1, uint8_t p2, RotaryEncoder::LatchMode latchMode, std::function<void(int previous, int current)> const onValueChanged, const char *name = NULL);

    void setup();
    void loop();
};
#endif
#ifndef __TIME_COMPONENT_H__
#define __TIME_COMPONENT_H__

#include <Arduino.h>
#include <eztime.h>
#include "components.h"
#include "logging.h"

class TimeComponent: public Component
{
  private:
    String _timezoneName;
    Timezone _TZ;

  public:
    TimeComponent(const char *timezoneName);
    Timezone *TZ();

    void setup();
    void loop();
};

#endif

#ifndef __TIME_COMPONENT_H__
#define __TIME_COMPONENT_H__

#include <Arduino.h>
#include <eztime.h>
#include "components.h"
#include "logging.h"

/***
 * Component-version of eztime
 */
class TimeComponent: public Component
{
  private:
    String _timezoneName;
    Timezone _TZ;
    uint16_t _syncTimeout;

  public:
    // Constructor with a time zone name, e.g. Europe/Amsterdam
    TimeComponent(const char *timezoneName, uint16_t syncTimeout = 0);
    // A pointer to the interal timezone object
    Timezone *TZ();

    // Required by Component
    void setup();
    void loop();
};

#endif

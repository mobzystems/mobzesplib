#ifndef __WIFI_COMPONENT_H__
#define __WIFI_COMPONENT_H__

#include "components.h"

class WifiComponent: public Component {
  private:
    String _hostname;
    String _ssid;
    String _password;
    unsigned long _lastCheckTime;
    unsigned long _intervalMs;
    uint32_t _waitMs;

  public:
    WifiComponent(const char *hostname, const char *ssid, const char *password, unsigned long checkInterval = 30000, uint32_t waitTime = 2000);
    void setup();
    void loop();
};

#endif
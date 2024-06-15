#ifndef __WIFI_COMPONENT_H__
#define __WIFI_COMPONENT_H__

#include "components.h"

/***
 * A Wifi component that connects to a WiFi network and optionally reconnects
 * periodically if the connection is lost
*/
class WifiComponent: public Component {
  private:
    String _hostname;
    String _ssid;
    String _password;
    unsigned long _intervalMs;
    uint32_t _waitMs;

    unsigned long _lastCheckTime;

  public:
    WifiComponent(const char *hostname, const char *ssid, const char *password, unsigned long checkInterval = 30000, uint32_t waitTime = 2000);

    // Required by Component
    void setup();
    void loop();
};

#endif
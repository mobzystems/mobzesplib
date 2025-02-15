#ifndef __WIFI_COMPONENT_H__
#define __WIFI_COMPONENT_H__

#include "components.h"
#include "WiFiClient.h"

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
    static WiFiClient _wifiClient;

    uint16_t _watchdogTimeoutSeconds;

  public:
    WifiComponent(const char *hostname, const char *ssid, const char *password, int watchdogTimeoutSeconds, unsigned long checkInterval = 30000, uint32_t waitTime = 2000);

    WiFiClient *wifiClient() { return &this->_wifiClient; }

    // Required by Component
    void setup();
    void loop();

    // Scan WiFi networks for the configured SSID and connect to the strongest BSSID
    String connectToStrongest(); // --> BSSID
};

#endif
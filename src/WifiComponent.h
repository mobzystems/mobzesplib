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

    String _ap_ssid;
    String _ap_password;
    bool _fallbackOnly;

    bool setupSoftAP();
    
  public:
    WifiComponent(
      const char *hostname, 
      const char *ssid, const char *password,
      int watchdogTimeoutSeconds, 
      unsigned long checkInterval = 30000, uint32_t waitTime = 2000,
      const char *ap_ssid = NULL, const char *ap_password = NULL, // SSID/Password of a Soft-AP. Only active if ssid supplied, password is optional
      bool fallbackOnly = true // If true, the soft-AP is only used when no connection is posssible with the "normal" ssid (or none is present)
    );

    WiFiClient *wifiClient() { return &this->_wifiClient; }

    // Required by Component
    void setup();
    void loop();

    // Scan WiFi networks for the configured SSID and connect to the strongest BSSID
    String connectToStrongest(); // --> BSSID
};

#endif
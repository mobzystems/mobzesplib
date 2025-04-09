#include "WifiComponent.h"
#include "logging.h"

#include "Specific_ESP_Wifi.h"

WiFiClient WifiComponent::_wifiClient;

// Helper function to restart after a delay
void Restart() {
  delay(2000);
  ESP.restart();
}

// Create a new WiFiComponent with host name and wifi credentials, plus an optional connection check
WifiComponent::WifiComponent(
  // The host name to use
  const char *hostname, 
  // The SSID of the network to connect to
  const char *ssid, 
  // WiFi password
  const char *password,
  int watchdogTimeoutSeconds,
  // The interval in ms to check the connection
  unsigned long checkInterval,
  // The wait time during a reconnect
  uint32_t waitTime,
  // SSID/Password of a Soft-AP. Only active if ssid supplied, password is optional
  const char *ap_ssid, 
  const char *ap_password,
  // If false, the soft-AP is only used when no connection is posssible with the "normal" ssid (or none is present)
  bool ap_permanent
) :
  Component("Wifi"),
  _hostname(hostname),
  _ssid(ssid),
  _password(password),
  _intervalMs(checkInterval),
  _waitMs(waitTime),
  _lastCheckTime(0),
  _watchdogTimeoutSeconds(watchdogTimeoutSeconds),
  _ap_ssid(ap_ssid),
  _ap_password(ap_password),
  _fallbackOnly(!ap_permanent)
{
}

/**
 * Set up a soft AP. Return true if successful
 */
bool WifiComponent::setupSoftAP() {
  if (this->_ap_ssid.isEmpty()) {
    Log::logCritical("[%s] Cannot set up soft AP: no SSID configured", this->name());
    return false;
  }

  Log::logInformation("[%s] Setting up soft AP with SSID '%s' and password '%s'", this->name(), this->_ap_ssid, this->_ap_password);

  // Switch to AP mode or even AP+STA
  if (this->_fallbackOnly || this->_ssid.isEmpty())
    // Fallback only, or no Station WiFi: AP only
    WiFi.mode(WIFI_AP);
  else
    // Else (i.e. permanent): both AP and STA
    WiFi.mode(WIFI_AP_STA);

  // Set up the AP
  bool result = WiFi.softAP(this->_ap_ssid, this->_ap_password);
  
  if (!result)
    Log::logCritical("[%s] Setting up soft AP with SSID '%s' failed!", this->name(), this->_ap_ssid);
  else
    Log::logInformation("[%s] Soft AP set up at %s.", this->name(), WiFi.softAPIP().toString().c_str());

  return result;
}

/**
 * Try to connect to the WiFi network. Scans for the strongest BSSID. If none is found,
 * a soft AP is set up if configured. If not, the ESP is restarted.
 * 
 * If a suitable BSSID is found, a connection is attempted. If a watchdog timeout was
 * specified and it expires, a soft AP is set up if configured, else the ESPS is restarted.
 */
void WifiComponent::setup()
{
  // --- Configure WIFI ---

  // Do we have a WiFi network we want to connect to in "station mode"?
  bool haveStationWifi = !this->_ssid.isEmpty();

#if defined(ESP32)
  // Make sure we connect to the strongest access point
  WiFi.setSortMethod(WIFI_CONNECT_AP_BY_SIGNAL);
  WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);
#endif
  // Set Wifi host name BEFORE calling WiFi.mode, otherwise it won't work on ESP32
  WiFi.setHostname(this->_hostname.c_str());
  WiFi.persistent(false);
#if defined(ESP32)
  // This causes "IP unset" on ESP8266!
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
#endif

  bool haveSoftAP = false;

  // Handle soft AP setup when _fallbackOnly == false (i.e. permanent soft AP)
  if (!this->_ap_ssid.isEmpty() && this->_fallbackOnly == false) {
    // We have a PERMANENT soft AP configured
    // WiFi.mode(WIFI_AP_STA);
    if (this->setupSoftAP())
      haveSoftAP = true;
  } else {
    // Station only, no AP
    WiFi.mode(WIFI_STA);
  }

  // Without station WiFi, we don't have a BSSID
  String bssid = haveStationWifi ? this->connectToStrongest() : "";

  if (bssid.isEmpty()) {

    // We were not able to find a network OR there was none configured. See if we need to set up a soft access point
    if (!this->_ap_ssid.isEmpty()) {
      // Skip setting ip a soft AP is we already have one
      if (!haveSoftAP) {
        // We have an AP SSID. Set it up:
        Log::logWarning("[%s] No networks '%s' found, setting up soft AP...", this->name(), this->_ssid.c_str());
        if (!this->setupSoftAP()) {
          // Soft AP set up failed! Restart the ESP
          Log::logCritical("[%s] No networks '%s' found, soft AP failed: giving up...", this->name(), this->_ssid.c_str());
          Restart();
        } else {
          haveSoftAP = true;
        }
      }
    } else {
      // We have no soft AP configured. Restart the ESP
      Log::logCritical("[%s] No networks '%s' found, no soft AP configured: giving up...", this->name(), this->_ssid.c_str());
      Restart();
    }

  } else {

    // We found a suitable BSSID. try connecting to it:
    Log::logDebug("[%s] Connecting to WiFi network '%s' with timeout %d, interval %d, wait %d seconds...",
      this->name(),
      this->_ssid.c_str(),
      this->_watchdogTimeoutSeconds,
      this->_intervalMs / 1000,
      this->_waitMs / 1000
    );
    setStatus(1000, Log::LOGLEVEL::Information, "Connecting");
    unsigned long ms = millis();

    while (WiFi.status() != WL_CONNECTED)
    {
      if (this->_watchdogTimeoutSeconds != 0 && millis() > ms + this->_watchdogTimeoutSeconds * 1000) {
        // We have a watchdog set up - give up after it expires, either by setting up an AP or restarting
        Serial.println();
        
        // If we have a soft AP configured, set it up now (unless already done)
        if (!this->_ap_ssid.isEmpty()) {

          if (haveSoftAP) {
            // If we have a soft AP already, just warn
            Log::logCritical("[%s] No connection after %d seconds, relying on soft AP '%s'", this->name(), this->_watchdogTimeoutSeconds, this->_ap_ssid);
            break;
          } else {
            // Attempt a soft AP
            setStatus(1010, Log::LOGLEVEL::Critical, this->_ap_ssid.c_str());
            Log::logCritical("[%s] No connection after %d seconds, falling back to soft AP '%s'...", this->name(), this->_watchdogTimeoutSeconds, this->_ap_ssid);
            if (!this->setupSoftAP()) {
              setStatus(1010, Log::LOGLEVEL::Critical, "Giving up!");
              Log::logCritical("[%s] Soft AP setup failed, restarting...", this->name());
              Restart();
            } else {
              break;
            }
          }

        } else {

          // No soft AP configured, give up
          setStatus(1010, Log::LOGLEVEL::Critical, "Giving up!");
          Log::logCritical("[%s] No connection after %d seconds, restarting...", this->name(), this->_watchdogTimeoutSeconds);
          Restart();

        }
      }

      // If there is no watchdog timeout, connection is retried indefinitely
      Log::logTrace("[%s] Still connecting...", this->name());
      Serial.print(".");
      setStatus(1000, Log::LOGLEVEL::Trace, ("Connecting " + String(1 + (millis() - ms) / 1000) /* + "/" + String(this->_watchdogTimeoutSeconds) */).c_str());
      delay(500);
    }

    // A connection was made OR a soft AP was set up
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
      setStatus(2000, Log::LOGLEVEL::Information, "Connected");
      Log::logInformation("[%s] Connected '%s' to '%s' (%s) at %s (MAC %s)", this->name(), WiFi.getHostname(), WiFi.SSID().c_str(), WiFi.BSSIDstr().c_str(), WiFi.localIP().toString().c_str(), WiFi.macAddress().c_str());
    } else {
      // We should have a soft AP here
      setStatus(9000, Log::LOGLEVEL::Error, "NOT connected");
      Log::logCritical("[%s] Cannot connect to WiFi", this->name());
    }
  }
}

// Check for a lost connection and reconnect, but only if the interval is not 0 AND we have station WiFi
void WifiComponent::loop()
{
  // If we have no check interval OR no station WiFi, skip this check
  if (this->_intervalMs == 0 || this->_ssid.isEmpty())
    return;

  if (millis() > this->_lastCheckTime + this->_intervalMs) {
    Log::logDebug("[%s] Checking connection... (%d)", this->name(), WiFi.status());

    // if WiFi is down, try reconnecting
    if (WiFi.status() != WL_CONNECTED)
    {
      Log::logWarning("[%s] Disconnected! Reconnecting...", this->name());

      this->connectToStrongest();

      // WiFi.reconnect(); // Includes a disconnect()
      
      // Wait for the reconnection:
      unsigned int ms = 0;
      do {
        delay(1000);
        ms += 1000;
      } while (ms < this->_waitMs && WiFi.status() != WL_CONNECTED);

      if (WiFi.status() == WL_CONNECTED) {
        Log::logWarning("[%s] Reconnected after %d seconds to %s (%d dBm).", this->name(), ms / 1000, WiFi.localIP().toString().c_str(), WiFi.RSSI());
      } else {
        Log::logWarning("[%s] *Not* reconnected!", this->name());
      }
    } else {
      Log::logDebug("[%s] Wifi connected to %s (%d dBm).", this->name(), WiFi.localIP().toString().c_str(), WiFi.RSSI());

      // We have WiFi - if we have a non-permanent soft AP set up, DISCONNECT THAT HERE
      if ((WiFi.getMode() & WIFI_AP) && this->_fallbackOnly) {
        Log::logInformation("[%s] Disabling fallback-only soft AP", this->name());
        WiFi.mode(WIFI_STA);
      }
    }

    this->_lastCheckTime = millis();
  }
}

/**
 * Perform a WiFi scan to determine the strongest BSSID for the configured SSID
 * If a BSSID is found, try connecting to it and return it.
 * If no BSSID is found OR no SSID was configured, return ""
 */
String WifiComponent::connectToStrongest() {
  if (this->_ssid.isEmpty()) {
    Log::logCritical("[%s] Cannot perform WiFi-scan, no SSID configured", this->name());
    return "";
  }

  Log::logDebug("[%s] Starting WiFi-scan for SSID '%s'...", this->name(), this->_ssid.c_str());
#ifdef ESP32
  int n = WiFi.scanNetworks(false, false, false, 300U, 0, this->_ssid.c_str());
#else
  int n = WiFi.scanNetworks(false, false, 0, (uint8*)this->_ssid.c_str());
#endif
  uint8_t *bssid = NULL;

  if (n == 0) {
    Log::logWarning("[%s] No networks with SSID '%s' found", this->name(), this->_ssid.c_str());
    return "";
  }

  int bestMatch = 0;
  int bestRSSI = WiFi.RSSI(0);
  for (int i = 0; i < n; i++) {
    // Look for a stronger BSSID
    if (i > 0 && WiFi.RSSI(i) > bestRSSI) {
      bestMatch = i;
      bestRSSI = WiFi.RSSI(i);
    }
    Log::logDebug("[%s] %2d: %s (%d dBm) BSSID %s", this->name(), i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i), WiFi.BSSIDstr(i).c_str());
  }
  Log::logInformation("[%s] Choosing BSSID '%s'", this->name(), WiFi.BSSIDstr(bestMatch).c_str());
  bssid = WiFi.BSSID(bestMatch);
  WiFi.begin(this->_ssid.c_str(), this->_password.c_str(), 0, bssid);
  return WiFi.BSSIDstr(bestMatch);
}

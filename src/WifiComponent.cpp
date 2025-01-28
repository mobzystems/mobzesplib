#include "WifiComponent.h"
#include "logging.h"

#include "Specific_ESP_Wifi.h"

WiFiClient WifiComponent::_wifiClient;

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
  uint32_t waitTime
) :
  Component("Wifi"),
  _hostname(hostname),
  _ssid(ssid),
  _password(password),
  _intervalMs(checkInterval),
  _waitMs(waitTime),
  _lastCheckTime(0),
  _watchdogTimeoutSeconds(watchdogTimeoutSeconds)
{
}

// Try to connect to the WiFi network. Does not return until a connection is made!
void WifiComponent::setup()
{
  // --- Configure WIFI ---

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
  WiFi.mode(WIFI_STA);

  this->connectToStrongest();

  Log::logDebug("[%s] Connecting to WiFi network '%s' with timeout %d, interval %d, wait %d seconds...",
    name(),
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
      Serial.println();
      setStatus(1010, Log::LOGLEVEL::Critical, "Giving up!");
      Log::logCritical("[%s] No connection after %d seconds, restarting...", name(), this->_watchdogTimeoutSeconds);
      delay(2000);
      // Restart the server
      ESP.restart();
    }
    Log::logTrace("[%s] Still connecting...", name());
    Serial.print(".");
    setStatus(1000, Log::LOGLEVEL::Trace, ("Connecting " + String(1 + (millis() - ms) / 1000) /* + "/" + String(this->_watchdogTimeoutSeconds) */).c_str());
    delay(500);
  }

  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    setStatus(2000, Log::LOGLEVEL::Information, "Connected");
    Log::logInformation("[%s] Connected '%s' to '%s' (%s) at %s (MAC %s)", name(), WiFi.getHostname(), WiFi.SSID().c_str(), WiFi.BSSIDstr().c_str(), WiFi.localIP().toString().c_str(), WiFi.macAddress().c_str());
  } else {
    setStatus(9000, Log::LOGLEVEL::Error, "NOT connected");
    Log::logCritical("[%s] Cannot connect to WiFi", name());
  }
}

// Check for a lost connection and reconnect, but only if the interval is not 0
void WifiComponent::loop()
{
  if (this->_intervalMs == 0)
    return;

  if (millis() > this->_lastCheckTime + this->_intervalMs)
  {
    Log::logDebug("[%s] Checking connection...", name());

    // if WiFi is down, try reconnecting
    if (WiFi.status() != WL_CONNECTED)
    {
      Log::logWarning("[%s] Disconnected! Reconnecting...", name());

      this->connectToStrongest();

      // WiFi.reconnect(); // Includes a disconnect()
      
      // Wait for the reconnection:
      unsigned int ms = 0;
      do {
        delay(1000);
        ms += 1000;
      } while (ms < this->_waitMs && WiFi.status() != WL_CONNECTED);
      // delay(this->_waitMs);
      if (WiFi.status() == WL_CONNECTED)
        Log::logInformation("[%s] Reconnected after %d seconds to %s (%d dBm).", name(), ms / 1000, WiFi.localIP().toString().c_str(), WiFi.RSSI());
      else
        Log::logWarning("[%s] *Not* reconnected!", name());
    }
    else
      Log::logDebug("[%s] Wifi connected to %s (%d dBm).", name(), WiFi.localIP().toString().c_str(), WiFi.RSSI());

    this->_lastCheckTime = millis();
  }
}

String WifiComponent::connectToStrongest() {
  #if !defined(ESP32)
    Log::logDebug("[%s] Starting WiFi-scan...", this->name());
    int n = WiFi.scanNetworks(false, false, 0, (uint8*)this->_ssid.c_str());
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
  #else
  // Just connect, let WiFi do its thing
  WiFi.begin(this->_ssid.c_str(), this->_password.c_str());
  #endif
}

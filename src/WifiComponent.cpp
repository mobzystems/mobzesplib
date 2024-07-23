#include "WifiComponent.h"
#include "logging.h"

#include "Specific_ESP_Wifi.h"

// Create a new WiFiComponent with host name and wifi credentials, plus an optional connection check
WifiComponent::WifiComponent(
  // The host name to use
  const char *hostname, 
  // The SSID of the network to connect to
  const char *ssid, 
  // WiFi password
  const char *password,
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
  _watchdogTimeoutSeconds(0)
{
}

// Try to connect to the WiFi network. Does not return until a connection is made!
void WifiComponent::setup()
{
  // --- Configure WIFI ---
  WiFi.persistent(false);
  // Set Wifi host name BEFORE calling WiFi.mode, otherwise it won't work on ESP32
  WiFi.setHostname(this->_hostname.c_str());
  WiFi.mode(WIFI_STA);

  WiFi.begin(this->_ssid.c_str(), this->_password.c_str());

  Log::logDebug("[%s] Connecting to WiFi network '%s'...", name(), this->_ssid.c_str());
  setStatus(1000, Log::LOGLEVEL::Information, "Connecting");
  unsigned long ms = millis();

  while (WiFi.status() != WL_CONNECTED)
  {
    if (this->_watchdogTimeoutSeconds != 0 && millis() > ms + this->_watchdogTimeoutSeconds * 1000) {
      setStatus(1010, Log::LOGLEVEL::Critical, "Giving up!");
      Log::logCritical("[%s] No connection after %d seconds, restarting...", name(), this->_watchdogTimeoutSeconds);
      delay(2000);
      // Restart the server
      ESP.restart();
    }
    Log::logTrace("[%s] Still connecting...", name());
    setStatus(1000, Log::LOGLEVEL::Trace, ("Connecting " + String(1 + (millis() - ms) / 1000) /* + "/" + String(this->_watchdogTimeoutSeconds) */).c_str());
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
    setStatus(2000, Log::LOGLEVEL::Information, "Connected");
    Log::logInformation("[%s] Connected '%s' to '%s' at %s (MAC %s)", name(), WiFi.getHostname(), this->_ssid.c_str(), WiFi.localIP().toString().c_str(), WiFi.macAddress().c_str());
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
      Log::logInformation("[%s] Reconnecting...", name());
      // WiFi.disconnect();
      WiFi.reconnect();
      delay(this->_waitMs);
      if (WiFi.status() == WL_CONNECTED)
        Log::logInformation("[%s] Reconnected.", name());
      else
        Log::logInformation("[%s] *Not* reconnected!", name());
    }
    else
      Log::logDebug("[%s] Still connected.", name());

    this->_lastCheckTime = millis();
  }
}

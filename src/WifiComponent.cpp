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
  Component("Wifi Component"),
  _hostname(hostname),
  _ssid(ssid),
  _password(password),
  _intervalMs(checkInterval),
  _waitMs(waitTime),
  _lastCheckTime(0)
{
}

// Try to connect to the WiFi network. Does not return until a connection is made!
void WifiComponent::setup()
{
  // --- Configure WIFI ---
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(this->_hostname.c_str());

  WiFi.begin(this->_ssid.c_str(), this->_password.c_str());

  Log::logDebug("[WifiComponent] Connecting to WiFi network '%s'...", this->_ssid.c_str());
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(200);
    Log::logTrace("[WifiComponent] Still connecting...");
  }

  if (WiFi.status() == WL_CONNECTED)
    Log::logInformation("[WifiComponent] Connected '%s' to '%s' at %s (MAC %s)", WiFi.getHostname(), this->_ssid.c_str(), WiFi.localIP().toString().c_str(), WiFi.macAddress().c_str());
  else
    Log::logCritical("[WifiComponent] Cannot connect to WiFi");
}

// Check for a lost connection and reconnect, but only if the interval is not 0
void WifiComponent::loop()
{
  if (this->_intervalMs == 0)
    return;

  if (millis() > this->_lastCheckTime + this->_intervalMs)
  {
    Log::logDebug("[WifiComponent] Checking WiFi-connection...");

    // if WiFi is down, try reconnecting
    if (WiFi.status() != WL_CONNECTED)
    {
      Log::logInformation("[WifiComponent] Reconnecting to WiFi...");
      WiFi.disconnect();
      delay(this->_waitMs);
      WiFi.reconnect();
      Log::logInformation("[WifiComponent] Reconnected.");
      // publish_status("WiFi reconnected");
    }
    else
      Log::logDebug("[WifiComponent] WiFi is connected.");

    this->_lastCheckTime = millis();
  }
}
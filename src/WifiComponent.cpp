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

  Log::logDebug("[%s] Connecting to WiFi network '%s'...", name(), this->_ssid.c_str());
  setStatus(1000, Log::LOGLEVEL::Information, "Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(200);
    Log::logTrace("[%s] Still connecting...", name());
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
      WiFi.disconnect();
      delay(this->_waitMs);
      WiFi.reconnect();
      Log::logInformation("[%s] Reconnected.", name());
      // publish_status("WiFi reconnected");
    }
    else
      Log::logDebug("[%s] Connected.", name());

    this->_lastCheckTime = millis();
  }
}
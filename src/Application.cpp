#include <Arduino.h>
#include "Application.h"
#include "logging.h"

#include "Specific_ESP_Wifi.h"

const char *Application::configFileName = "/config.sys";

/**
 * Main Application class. Initializes LittleFS and reads configuration from /config.sys
 * if LittleFS available. When setup() is called, Wifi and Time services are enabled
 */
Application::Application(size_t maxConfigValues) :
  _configuration(NULL),
  _tasks(new Tasks()),
  _wifi(NULL),
  _time(NULL),
  _ota(NULL),
  _hostname("default-hostname"),
  _macAddress(WiFi.macAddress()),
  _restartDelay(-1)
{
  Log::logDebug("[Application] Starting...");

  if (!LittleFS.begin()) {
    Log::logCritical("Cannot start file system, con configuration available");
  } else {
    _configuration = new Configuration(&LittleFS, this->configFileName, maxConfigValues);
    _configuration->log(Log::LOGLEVEL::Trace);

    this->_hostname = this->config("hostname", "missing-hostname");
    // wifiSsid = conf.value("wifi-ssid", "[Missing wifi-ssid]");
    // wifiPassword = conf.value("wifi-password", "[Missing wifi-password]");    
  }

  Components::add(this->_tasks);
  Log::logDebug("[Application] created.");
}

/**
 * Read a value from configuration. the MAc address is used for specific configuration
 */
const char *Application::config(const char *key, const char *defaultValue) {
  // Read the "common value", i.e. without the mac address
  const char *commonValue = this->_configuration->value(key, defaultValue == NULL ? (String("[Missing ") + key + "]").c_str() : defaultValue);
  return this->_configuration->value((key + String("-") + this->_macAddress).c_str(), commonValue);
}

/**
 * Setup the application. Must be called after construction!
 */
void Application::setup() {
  // Set up wifi and time components
  Components::add(this->_wifi = new WifiComponent(_hostname, this->config("wifi-ssid"), this->config("wifi-password")));
  Components::add(this->_time = new TimeComponent(this->config("timezone", "Europe/Amsterdam")));

  this->_bootTimeUtc = UTC.tzTime();

  Components::add(this->_ota = new OtaComponent(80));

  Log::logDebug("[Application] booted at %s UTC", this->_time->TZ()->dateTime(this->bootTimeUtc(), "Y-M-d H:i:s").c_str());
}

/**
 * Call this method from your loop() method
 */
void Application::loop() {
  Components::loop();

  if (this->_restartDelay != -1) {
    Log::logWarning("[Application] Restart requested in %ld ms", this->_restartDelay);
    if (this->_restartDelay != 0)
      delay(this->_restartDelay);
    Log::logInformation("[Application] Restarting.");
    ESP.restart();
  }
}

/**
 * Add a component
 */
void Application::addComponent(Component *component) {
  Components::add(component);
}

/**
 * Add a periodic task, i.e. a callback
 */
void Application::addTask(String name, Milliseconds interval, void (*taskFunction)()) {
  this->_tasks->add(name, interval, taskFunction);
}

void Application::mapGet(const char *path, void (*handler)()) {
  this->_ota->webserver()->on(path, HTTP_GET, handler);
}

void Application::mapPost(const char *path, void (*handler)()) {
  this->_ota->webserver()->on(path, HTTP_POST, handler);
}
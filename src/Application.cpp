#include <Arduino.h>
#include "Application.h"
#include "logging.h"

#include "Specific_ESP_Wifi.h"

const char *Application::configFileName = "/config.sys";

/**
 * Main Application class. Initializes LittleFS and reads configuration from /config.sys
 * if LittleFS available. When setup() is called, Wifi and Time services are enabled
 */
Application::Application(uint16_t otaPortNumber, size_t maxConfigValues) :
  _configuration(NULL),
  _tasks(new Tasks()),
  _wifi(NULL),
  _time(NULL),
  _ota(NULL),
  _otaPortNumber(otaPortNumber),
  _hostname("default-hostname"),
  _macAddress(WiFi.macAddress()),
  _restartDelay((unsigned long)-1)
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

  Components::add(this->_ota = new OtaComponent(this->_otaPortNumber));

  const char *otaUsername = this->config("ota-username", NULL);
  const char *otaPassword = this->config("ota-password", NULL);
  if (otaUsername != NULL && otaPassword != NULL)
    ElegantOTA.setAuth(otaUsername, otaPassword);

  Log::logDebug("[Application] booted at %s UTC", this->_time->TZ()->dateTime(this->bootTimeUtc(), "Y-M-d H:i:s").c_str());
}

/**
 * Call this method from your loop() method
 */
void Application::loop() {
  Components::loop();

  if (this->_restartDelay != (unsigned long)-1) {
    Log::logWarning("[Application] Restart requested in %ld ms", this->_restartDelay);
    if (this->_restartDelay != 0)
      delay(this->_restartDelay);
    Log::logInformation("[Application] Restarting.");
    this->webserver()->stop();
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

String Application::makeHtml(const char *message) {
    auto f = LittleFS.open("/config.sys", "r");
    auto s = f.readString();
    f.close();
    String html = R"###(
<html>
  <head>
    <title>Change configuration</title>
  </head>
  <body>
    <h1>#HOSTNAME# configuration</h1>
    #MESSAGE#
    <form method="POST">
      <p>
        <textarea name="text" style="width: 100%; height: 80vh; text-wrap: nowrap">#TEXT#</textarea>
      </p>
      <p>
        <input type="submit" name="submit" value="Save" />
      </p>
    </form>
    <form method="POST">
      <p>
        <input type="submit" name="submit" value="Reset" />
      </p>
    </form>
  </body>
</html>
)###";
    if (message == NULL)
      html.replace("#MESSAGE#", "");
    else
      html.replace("#MESSAGE#", String("<h2>") + message + "</h2>");

    html.replace("#HOSTNAME#", this->hostname());
    html.replace("#TEXT#", s);

    return html;
}

void Application::enableConfigEditor(const char *path) {
  this->_ota->webserver()->on(path, HTTP_GET, [&]() { 
    this->webserver()->sendContent(this->makeHtml(NULL));
  });

  this->_ota->webserver()->on(path, [&]() {
    auto t = this->webserver()->arg("submit");
    if (t == "Save") {
      auto s = this->webserver()->arg("text");
      auto f = LittleFS.open("/config.sys", "w");
      f.print(s);
      f.close();
      this->webserver()->sendContent(this->makeHtml("Contents were changed."));
    } else if (t == "Reset") {
      this->webserver()->sendContent("Reset requested.");
      this->requestReset(3000);
    } else {
      this->webserver()->sendContent("GOT:" + t);
    }
  });
}
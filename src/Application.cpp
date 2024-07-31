#include <Arduino.h>
#include "Application.h"
#include "logging.h"

#include "Specific_ESP_Wifi.h"

const char *Application::configFileName = "/config.sys";

/**
 * Main Application class. Initializes LittleFS and reads configuration from /config.sys
 * if LittleFS available. When setup() is called, Wifi and Time services are enabled
 */
Application::Application(const char *title, const char *version, uint16_t otaPortNumber, size_t maxConfigValues) :
  _title(title),
  _version(version),
  _configuration(NULL),
  _tasks(new Tasks()),
  _wifi(NULL),
  _time(NULL),
  _oled(NULL),
  _ota(NULL),
  _otaPortNumber(otaPortNumber),
  _hostname("default-hostname"),
  _macAddress(WiFi.macAddress()),
  _bootTimeUtc(0),
  _restartDelay((unsigned long)-1)
{
  Log::logDebug("[Application] Starting...");

  if (!LittleFS.begin()) {
    Log::logCritical("[Application] Cannot start file system, no configuration available");
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
  Components::add(this->_wifi = new WifiComponent(
    _hostname, 
    this->config("wifi-ssid"), this->config("wifi-password"), 
    atoi(this->config("wifi-watchdog-timeout", "30"))
  ));

  // Set up the time component with a default timeout
  Components::add(this->_time = new TimeComponent(
    this->config("timezone", "Europe/Amsterdam"),
    atoi(this->config("time-timeout", "5"))
  ));

  // OTA:
  Components::add(this->_ota = new OtaComponent(this->_otaPortNumber));

  const char *otaUsername = this->config("ota-username", NULL);
  const char *otaPassword = this->config("ota-password", NULL);
  if (otaUsername != NULL && otaPassword != NULL)
    ElegantOTA.setAuth(otaUsername, otaPassword);

  // if (timeStatus() == timeSet) {
  //   this->setBootTime(UTC.tztime());
  // } else {
  //   Log::logWarning("[Application] Cannot set boot time, time not set");
  // }

  this->webserver()->enableCORS(true);
}

void Application::setBootTimeUtc(time_t utc) {
  this->_bootTimeUtc = UTC.tzTime();
  Log::logDebug("[Application] Booted at %s UTC", this->_time->TZ()->dateTime(this->bootTimeUtc(), "Y-m-d H:i:s").c_str());
}

/**
 * Call this method from your loop() method
 */
void Application::loop() {
  // Get the first available boot time
  if (this->_bootTimeUtc == 0 && timeStatus() == timeSet) {
    this->setBootTimeUtc(UTC.tzTime());
  }

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
void Application::addTask(String name, Milliseconds interval, std::function<void()> const taskFunction) {
  this->_tasks->add(name, interval, taskFunction);
}

void Application::mapGet(const char *path, std::function<void(WebServer *)> const handler) {
  this->webserver()->on(path, HTTP_GET, [this, handler]() { handler(this->webserver()); });
}

void Application::mapPost(const char *path, std::function<void(WebServer *)> const handler) {
  this->webserver()->on(path, HTTP_POST, [this, handler]() { handler(this->webserver()); });
}

String readFile(const char *path) {
  auto f = LittleFS.open(path, "r");
  if (!f)
    return String(""); // TODO: this is not an empty file...
    
  auto s = f.readString();
  f.close();
  return s;
}

bool writeFile(const char *path, const char *s) {
  auto f = LittleFS.open(path, "w");
  if (!f)
    return false;
  f.print(s);
  f.close();
  return true;
}

String Application::HtmlEncode(const char *s) {
  String html(s);
  html.replace("<", "&lt;");
  html.replace(">", "&gt;");
  return html;
}

String Application::makeHtml(const char *file, const char *message) {
    String s = readFile(file);

    String html = LittleFS.exists("/wwwroot/edit-file.html")
      ? readFile("/wwwroot/edit-file.html")
      : R"###(
<html>
  <head>
    <title>#FILE# - Edit</title>
    <style>
      body { font-family: helvetica, arial, sans-serif; display: grid; grid-template-rows: auto 1fr auto; }
      p { margin: 0; }
      textarea { width: 100%; height: 100%; text-wrap: nowrap; }
      #topform { display: grid; grid-template-rows: 1fr auto; gap: 0.5rem; }
      .message { color: red; }
    </style>
  </head>
  <body>
    <div>
      <h1>File <em>#FILE#</em></h1>
      <h2>on <em>#HOSTNAME# (#MACADDRESS#)</em></h2>
    </div>
    <form method="POST" id="topform">
      <p>
        <textarea name="text">#TEXT#</textarea>
      </p>
      <p>
        <input type="submit" name="submit" value="Save" />
        <span class="message">#MESSAGE#</span>
      </p>
    </form>
    <form method="POST">
      <p>
        <input type="submit" name="submit" value="Reset" />
        #APPTITLE#
      </p>
    </form>
  </body>
</html>
)###";
    if (message == NULL)
      html.replace("#MESSAGE#", "");
    else
      html.replace("#MESSAGE#", this->HtmlEncode(message).c_str());

    html.replace("#FILE#", this->HtmlEncode(file));
    html.replace("#HOSTNAME#", this->HtmlEncode(this->hostname()));
    html.replace("#MACADDRESS#", this->HtmlEncode(this->_macAddress.c_str()));
    html.replace("#TEXT#", this->HtmlEncode(s.c_str()));

    String appTitle = this->_title;
    if (this->_version.length() > 0)
      appTitle += " v" + this->_version;
    html.replace("#APPTITLE#", appTitle);

    return html;
}

void Application::enableConfigEditor(const char *path) {
  this->mapGet(path, [this](WebServer *server) {
    server->sendContent(this->makeHtml("/config.sys", NULL));
  });

  this->mapPost(path, [this](WebServer *server) {
    auto t = server->arg("submit");
    if (t == "Save") {
      auto s = server->arg("text");
      writeFile("/config.sys", s.c_str());
      server->sendContent(this->makeHtml("/config.sys", "Contents were changed."));
    } else if (t == "Reset") {
      server->sendContent("Reset requested.");
      this->requestReset(3000);
    } else {
      server->sendContent("GOT:" + t);
    }
  });
}

void Application::enableFileEditor(const char *readPath, const char *writePath, const char *editPath) {
  if (readPath != NULL) {
    this->mapGet(readPath, [this](WebServer *server) {
      auto path = server->arg("f");
      if (path.length() == 0)
        server->send(400);
      else if (LittleFS.exists(path))
        server->sendContent(readFile(path.c_str()));
      else
        server->send(404, "text/plain", "File not found: " + path);
    });
  }

  if (writePath != NULL) {
    this->mapPost(writePath, [this](WebServer *server) {
      auto path = server->arg("f");
      if (path.length() == 0)
        server->send(400);
      else {
        auto s = server->arg("text");
        writeFile(path.c_str(), s.c_str());
        server->send(200);
      }
    });
  }

  if (editPath != NULL) {
    this->mapGet(editPath, [this](WebServer *server) { 
      auto path = server->arg("f");
      if (path.length() == 0)
        server->send(400);
      else
        server->sendContent(this->makeHtml(path.c_str(), NULL));
    });

    this->mapPost(editPath, [this](WebServer *server) {
      auto path = server->arg("f");
      auto s = server->arg("text");
      if (path.length() == 0)
        server->send(400);
      else {
        auto s = server->arg("text");
        writeFile(path.c_str(), s.c_str());
        server->sendContent(this->makeHtml(path.c_str(), "Contents were changed."));
      }
    });
  }
}

const String &Application::chipModelName() {
#if defined(ESP8266)
  static const String _esp8266("ESP8266");
  return _esp8266;
#elif defined(ESP32)
  static const String _esp32(ESP.getChipModel());
  return _esp32;
#else
  #error Unknown chip type
#endif
}

void Application::enableInfoPage(const char *path) {
  this->mapGet("/info", [this](WebServer *server) {
    // if(!_webServer->authenticate("123", "456"))
    //   _webServer->requestAuthentication();
    // else

    // _webServer->enableCORS(true); // Already enabled thruogh application
    server->send(200, "text/plain",
      String("Hostname: ") + String(this->hostname()) + 
      "\r\nApplication: " + this->title() + 
      "\r\nVersion: " + this->version() + 
      "\r\nIP: " + this->wifi()->wifiClient()->localIP().toString() + 
      "\r\nMAC: " + WiFi.macAddress() +
      "\r\nCPU: " + this->chipModelName() +
      "\r\nBootUTC: " + this->bootTimeLocalString() +
      "\r\nUTC: " + UTC.dateTime("Y-m-d H:i:s")
    );
  });  
}

void Application::addOledDisplay(int sda, int scl, uint8_t address) {
  this->addComponent(this->_oled = new OledComponent(sda, scl, address));
  auto display = _oled->getDisplay();
  // _display->setRotation(2); // 180
  display->setTextSize(1, 2);
  display->println("Starting");
  display->print(this->hostname());
  display->display();
}

Adafruit_SSD1306 *Application::display() {
  if (this->_oled == NULL) 
    return NULL;
  else
    return this->_oled->getDisplay();
}
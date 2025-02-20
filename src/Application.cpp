#include <Arduino.h>
#include "Application.h"
#include "logging.h"

#include "Specific_ESP_Wifi.h"

// The one and only application instance
Application *Application::_app = NULL;

const char *Application::configFileName = "/config.sys";

/**
 * Main Application class. Initializes LittleFS and reads configuration from /config.sys
 * if LittleFS available. When setup() is called, Wifi and Time services are enabled
 */
Application::Application(const char *title, const char *version, uint16_t otaPortNumber, const char *configuration) :
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
  _bootTimeLocal(0),
  _restartTimeMs((unsigned long)-1)
{
  // Set the singleton appliction object
  this->_app = this;

  Log::logDebug("[Application] Starting...");

  if (configuration != NULL) {
    Log::logDebug("[Application] Reading configuration from string");
    this->_configuration = new Configuration(configuration, 20);
  } else {
    if (!LittleFS.begin()) {
      Log::logCritical("[Application] Cannot start file system, no configuration available");
    } else {
      // Reserve 20 configuration variables initially
      this->_configuration = new Configuration(&LittleFS, this->configFileName, 20);
    }
  }

  if (this->_configuration != NULL) {
    this->_configuration->log(Log::LOGLEVEL::Trace);
    this->_hostname = this->config("hostname", "missing-hostname");
  }
  // wifiSsid = conf.value("wifi-ssid", "[Missing wifi-ssid]");
  // wifiPassword = conf.value("wifi-password", "[Missing wifi-password]");    

  Components::add(this->_tasks);
  Log::logDebug("[Application] created.");
}

/**
 * Read a value from configuration. the MAc address is used for specific configuration
 */
const char *Application::config(const char *key, const char *defaultValue) {
  // Read the "common value", i.e. without the mac address
  const char *commonValue = this->_configuration->value(key, defaultValue == NULL ? emptyString.c_str() : defaultValue);
  return this->_configuration->value((key + String("-") + this->_macAddress).c_str(), commonValue);
}

/**
 * Setup the application. Must be called after construction!
 */
void Application::setup() {
  Log::logDebug("[Application] Starting setup...");

  // Station SSID
  String ssid(this->config("wifi-ssid"));

  // Soft AP SSID
  String ap_ssid(this->config("wifi-ap-ssid"));
  if (!ap_ssid.isEmpty())
    ap_ssid.replace(F("#HOSTNAME#"), this->hostname());

  Components::add(this->_wifi = new WifiComponent(
    _hostname, 
    // STA: Connect to this SSID
    ssid.c_str(), this->config("wifi-password"), 
    Duration::parse(this->config("wifi-watchdog-timeout", "30")),
    Duration::parse(this->config("wifi-interval", "30")) * 1000,
    Duration::parse(this->config("wifi-wait", "20")) * 1000,
    // Soft AP
    ap_ssid.c_str(), this->config("wifi-ap-password"),
    atoi(this->config("wifi-ap-permanent", "0")) != 0 // Permanent soft AP
  ));

  if (ssid.isEmpty()) {
    Log::logWarning("[Application] Skipping time component because no SSID configured");
  } else {
    // Set up the time component with a default timeout
    Components::add(this->_time = new TimeComponent(
      this->config("timezone", "Europe/Amsterdam"),
      Duration::parse(this->config("time-timeout", "5"))
    ));
  }

  // OTA:
  Components::add(this->_ota = new OtaComponent(this->_otaPortNumber));

  const char *otaUsername = this->config("ota-username");
  const char *otaPassword = this->config("ota-password");
  if (*otaUsername && *otaPassword)
    ElegantOTA.setAuth(otaUsername, otaPassword);

  this->webserver()->enableCORS(true);

  // Get the first available boot time
  this->setBootTimeIfAvailable();

  Log::logDebug("[Application] Setup completed.");
}

void Application::setBootTimeIfAvailable() {
  // If we don't have a boot time yet and we have a valid time, set the boot time
  // This means the boot time really is "the moment the time became available" :-(
  if (this->_bootTimeUtc == 0 && timeStatus() == timeSet) {
    this->_bootTimeUtc = UTC.tzTime();
    this->_bootTimeLocal = this->_time->TZ()->tzTime();
    Log::logDebug("[Application] Booted at %s UTC", this->bootTimeUtcString().c_str());
  }  
}

/**
 * Call this method from your loop() method
 */
void Application::loop() {
  // Get the first available boot time if it wasn't available earlier
  if (this->_bootTimeUtc == 0 && timeStatus() == timeSet) {
    this->setBootTimeIfAvailable();
  }

  Components::loop();

  if (this->_restartTimeMs != (unsigned long)-1 && millis() > this->_restartTimeMs) {
    // Log::logWarning("[Application] Restart requested in %ld ms", this->_restartDelay);
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

void Application::mapGet(const char *path, std::function<void(WEBSERVER *)> const handler) {
  this->webserver()->on(path, HTTP_GET, [this, handler]() { handler(this->webserver()); });
}

void Application::mapPost(const char *path, std::function<void(WEBSERVER *)> const handler) {
  this->webserver()->on(path, HTTP_POST, [this, handler]() { handler(this->webserver()); });
}

String Application::readFile(const char *path) {
  if (!LittleFS.exists(path)) {
    Log::logWarning("[Application] File '%s' does not exist, returning ''");
    return emptyString;
  }

  auto f = LittleFS.open(path, "r");
  if (!f)
    return emptyString; // TODO: this is not an empty file...
    
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

bool deleteFile(const String &path) {
  return LittleFS.remove(path);
}

bool makeDirectory(const String &path) {
  return LittleFS.mkdir(path);
}

bool deleteDirectory(const String &path) {
  return LittleFS.rmdir(path); // Does remove() internally
}

String Application::HtmlEncode(const char *s) {
  String html(s);
  html.replace(F("<"), F("&lt;"));
  html.replace(F(">"), F("&gt;"));
  return html;
}

String dir(const String &path) {
  String output;
  output.reserve(2000);

  #ifdef ESP32

  auto dir = LittleFS.open(path);
  if (dir.isDirectory()) {
    File file = dir.openNextFile();
    while (file) {
      if (file.isDirectory()) {
        output.concat('/'); output.concat(file.name());
        output.concat('\t'); output.concat('0');
        output.concat('\t'); output.concat(file.getLastWrite());
      } else {
        output.concat(file.name());
        output.concat('\t'); output.concat('0');
        output.concat('\t'); output.concat(file.getLastWrite());
        output.concat('\t'); output.concat(file.size());
      }
      output.concat('\n');
      file = dir.openNextFile();
    }
  }

  #else

  auto dir = LittleFS.openDir(path);
  while (dir.next()) {
    if (dir.isFile()) {
      output.concat(dir.fileName());
      output.concat('\t'); output.concat(dir.fileCreationTime());
      output.concat('\t'); output.concat(dir.fileTime());
      output.concat('\t'); output.concat(dir.fileSize());
      output.concat('\n');
    } else if (dir.isDirectory()) {
      output.concat('/'); output.concat(dir.fileName());
      output.concat('\t'); output.concat(dir.fileCreationTime());
      output.concat('\t'); output.concat(dir.fileTime());
      output.concat('\n');
    }
  }

  #endif

  return output;
}

String Application::makeHtml(const char *file, const char *message) {
    String html = LittleFS.exists(F("/wwwroot/edit-file.html"))
      ? this->readFile("/wwwroot/edit-file.html")
      : F(R"###(
<html>
  <head>
    <title>#HOSTNAME#: #FILE# - Edit</title>
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <link rel="icon" href="/favicon.svg" mask="#fff" />
    <style>
      html { margin: 0; padding: 0; }
      body { font-family: helvetica, arial, sans-serif; margin: 0; padding: 0; height: 100%; }
      form { display: grid; grid-template-rows: auto 1fr auto auto; height: 100%; }
      div { margin: 0.25rem 0.5rem; }

      h1, h2, h3 { margin: 0.25rem 0; }
      #parent { position: relative; }
      textarea { position: absolute; top: 0; right: 0; bottom: 0; left: 0; text-wrap: nowrap; }
      .message { color: red; }
    </style>
  </head>
  <body>
    <form method="POST">
      <div>
        <h2>File <em>#FILE#</em></h1>
        <h3>on <em>#HOSTNAME# (#MACADDRESS#)</em></h2>
      </div>
      <div id="parent">
        <textarea name="text">#TEXT#</textarea>
      </div>
      <div>
        <input type="submit" name="submit" value="Save" />
        <span class="message">#MESSAGE#</span>
      </div>
      <div>
        <input type="submit" name="submit" value="Restart" />
        #APPTITLE#
      </div>
    </form>
  </body>
</html>
)###");

    if (message == NULL)
      html.replace(F("#MESSAGE#"), emptyString);
    else
      html.replace(F("#MESSAGE#"), this->HtmlEncode(message).c_str());

    html.replace(F("#FILE#"), this->HtmlEncode(file));
    html.replace(F("#HOSTNAME#"), this->HtmlEncode(this->hostname()));
    html.replace(F("#MACADDRESS#"), this->HtmlEncode(this->_macAddress.c_str()));

    String appTitle = this->_title;
    if (!this->_version.isEmpty())
      appTitle += " v" + this->_version;
    html.replace(F("#APPTITLE#"), appTitle);

    // Do this one LAST otherwise it will also replace in s!
    String s = this->readFile(file);
    html.replace(F("#TEXT#"), this->HtmlEncode(s.c_str()));

    return html;
}

void Application::enableConfigEditor(const char *path) {
  this->mapGet(path, [this](WEBSERVER *server) {
    server->send(200, F("text/html"), this->makeHtml(this->configFileName, NULL));
  });

  this->mapPost(path, [this](WEBSERVER *server) {
    auto t = server->arg(F("submit"));
    if (t == "Save") {
      auto s = server->arg("text");
      writeFile(this->configFileName, s.c_str());
      Log::logWarning("[Application] Configuration updated");
      server->send(200, F("text/html"), this->makeHtml(this->configFileName, "Contents were changed."));
    } else if (t == "Reset"|| t == "Restart") {
      Log::logWarning("[Application] Restart requested");
      server->send(200, F("text/plain"), F("Restart requested."));
      this->scheduleRestart(3000);
    } else {
      server->send(200, F("text/plain"), "GOT: " + t);
    }
  });
}

void Application::enableFileEditor(
  const char *readPath, 
  const char *writePath, 
  const char *editPath, 
  const char *dirPath,
  const char *deletePath,
  const char *mkdirPath,
  const char *rmdirPath
) {
  if (readPath != NULL) {
    this->mapGet(readPath, [this](WEBSERVER *server) {
      auto path = server->arg("f");
      if (path.isEmpty())
        server->send(400);
      else if (LittleFS.exists(path))
        server->send(200, F("text/plain"), this->readFile(path.c_str()));
      else
        server->send(404, F("text/plain"), "File not found: " + path);
    });
  }

  if (writePath != NULL) {
    this->mapPost(writePath, [this](WEBSERVER *server) {
      auto path = server->arg("f");
      if (path.isEmpty())
        server->send(400);
      else {
        auto s = server->arg(F("text"));
        writeFile(path.c_str(), s.c_str());
        server->send(200);
      }
    });
  }

  if (editPath != NULL) {
    this->mapGet(editPath, [this](WEBSERVER *server) { 
      auto path = server->arg("f");
      if (path.isEmpty())
        server->send(400);
      else
        server->send(200, F("text/html"), this->makeHtml(path.c_str(), NULL));
    });

    this->mapPost(editPath, [this](WEBSERVER *server) {
      auto path = server->arg("f");
      // Todo: base64
      auto s = server->arg(F("text"));
      if (path.isEmpty())
        server->send(400);
      else {
        auto s = server->arg(F("text"));
        writeFile(path.c_str(), s.c_str());
        server->send(200, F("text/html"), this->makeHtml(path.c_str(), "Contents were changed."));
      }
    });
  }

  if (dirPath != NULL) {
    this->mapGet(dirPath, [this](WEBSERVER *server) { 
      auto path = server->arg("f");
      if (path.isEmpty())
        server->send(400);
      else
        server->send(200, F("text/plain"), dir(path));
    });
  }

  if (deletePath != NULL) {
    this->mapGet(deletePath, [this](WEBSERVER *server) { 
      auto path = server->arg("f");
      if (path.isEmpty())
        server->send(400);
      else if (deleteFile(path))
        server->send(200);
      else
        server->send(404); // Literally "File not found"
    });
  }

  if (mkdirPath != NULL) {
    this->mapGet(mkdirPath, [this](WEBSERVER *server) { 
      auto path = server->arg("f");
      if (path.isEmpty())
        server->send(400);
      else if (makeDirectory(path))
        server->send(200);
      else
        server->send(404); // Literally "File not found"
    });
  }

  if (rmdirPath != NULL) {
    this->mapGet(rmdirPath, [this](WEBSERVER *server) { 
      auto path = server->arg("f");
      if (path.isEmpty())
        server->send(400);
      else if (deleteDirectory(path))
        server->send(200);
      else
        server->send(404); // Literally "File not found"
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

String Application::formatDuration(const Duration &d) {
  char buffer[] = "xxxxxd, HH:MM:SS";
  sprintf(buffer, "%hdd, %02hd:%02hd:%02hd", d.days % 100000, d.hours % 100, d.minutes % 100, d.seconds % 100);
  return String(buffer);
}

void Application::enableInfoPage(const char *path, std::function<void (String &)> const &postProcessInfo) {
  this->mapGet("/info", [this, postProcessInfo](WEBSERVER *server) {
    // if(!_webServer->authenticate("123", "456"))
    //   _webServer->requestAuthentication();
    // else
    // _webServer->enableCORS(true); // Already enabled through application

    Duration d(this->upTimeSeconds());

    String initialResponse = 
      String(F("Hostname: ")) + String(this->hostname()) + 
      F("\r\nApplication: ") + this->title() + 
      F("\r\nVersion: ") + this->version() + 
      F("\r\nIP: ") + WiFi.localIP().toString() + 
      ((WiFi.getMode() & WIFI_AP) ? "\r\nAP: " + WiFi.softAPIP().toString() : "") +
      // This is the IP of the MQTT Wifi client. When disconected, this can be anything :-/
      // \r\nClientIP: " + this->wifi()->wifiClient()->localIP().toString() +
      F("\r\nRSSI: ") + String(WiFi.RSSI()) + F(" dBm") +
      F("\r\nBSSID: ") +  WiFi.BSSIDstr() +
      F("\r\nMAC: ") + WiFi.macAddress() +
      F("\r\nCPU: ") + this->chipModelName() +
      F("\r\nBootUTC: ") + this->bootTimeUtcString() +
      F("\r\nUTC: ") + UTC.dateTime("Y-m-d H:i:s") + 
      F("\r\nUptime: ") + this->formatDuration(d)
    ;

    if (postProcessInfo != NULL)
      postProcessInfo(initialResponse);

    server->send(200, F("text/plain"), initialResponse.c_str());
  });  
}

void Application::addOledDisplay(int sda, int scl, uint8_t address) {
  this->addComponent(this->_oled = new OledComponent(sda, scl, address));
  auto display = _oled->getDisplay();
  // _display->setRotation(2); // 180
  display->setTextSize(1, 2);
  display->println(F("Starting"));
  display->print(this->hostname());
  display->display();
}

Adafruit_SSD1306 *Application::display() {
  if (this->_oled == NULL) 
    return NULL;
  else
    return this->_oled->getDisplay();
}

void Application::addU8Display(U8X8 *display) {
  this->addComponent(this->_u8x8 = new U8DisplayComponent(display));
  auto d = this->_u8x8->getDisplay();
  // _display->setRotation(2); // 180
  d->draw1x2String(0, 0, "Starting");
  d->drawString(0, 2, this->hostname());
  // display->display();
}

void Application::scheduleRestart(unsigned long delayMs) {
  Log::logWarning("[Application] Scheduling restart in %lu ms", delayMs);
  _restartTimeMs = millis() + delayMs;
}
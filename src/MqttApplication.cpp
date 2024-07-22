#include "MqttApplication.h"

#include "Logging.h"

MqttApplication *MqttApplication::_app = NULL;

MqttApplication::MqttApplication(const char *title, const char *version, const char *mqttPrefix, int wifiWatchdogTimout, uint16_t otaPortNumber, size_t maxConfigValues) :
  Application(title, version, otaPortNumber, maxConfigValues),
  _mqtt(NULL),
  _mqttPrefix(mqttPrefix),
  _onlinetopic(String(mqttPrefix) + "/status/" + this->hostname() + "/online"),
  _loopCount(0),
  _autoRestartTimeout(atoi(Application::config("auto-restart-timeout", "0")))
{
  Log::logDebug("[MqttApplication] Creating application '%s' v%s on '%s'", this->title().c_str(), this->version().c_str(), this->hostname(), this->_onlinetopic.c_str());
  // Configure the one and only app (also for lambdas)
  MqttApplication::_app = this,
  // Set the wifi watchdog timeout
  Application::setWifiWatchdogTimeoutSeconds(wifiWatchdogTimout);

  // Add the free memory/loop count task
  this->addTask("Show memory/loop status", atoi(this->config("memory-interval", "60")) * 1000, []()
  {
    static unsigned long lastMs = 0;

    unsigned long ms = millis();
    int loopSpeed = 0;
    if (lastMs != 0) {
      float seconds = (ms - lastMs) / 1000.0;
      loopSpeed = (int)(_app->_loopCount / seconds);
    }
    lastMs = ms;

    uint32_t freeSize = ESP.getFreeHeap();

    Log::logInformation("Free: %ld - Loop count: %d (%d/s)", freeSize, _app->_loopCount, loopSpeed);
    _app->publishProperty("free", String(freeSize).c_str());
    if (_app->_loopCount > 1)
      _app->publishProperty("loops", String(loopSpeed).c_str());
    _app->_loopCount = 0;
  });
}

void MqttApplication::enableInfoPage(const char *path) {
  this->mapGet("/info", []() {
    // if(!_webServer->authenticate("123", "456"))
    //   _webServer->requestAuthentication();
    // else

    // _webServer->enableCORS(true); // Already enabled thruogh application
    _app->webserver()->send(200, "text/plain",
      String("Hostname: ") + String(_app->hostname()) + 
      "\r\nApplication: " + _app->title() + 
      "\r\nVersion: " + _app->version() + 
      "\r\nIP: " + _app->wifi()->wifiClient()->localIP().toString() + 
      "\r\nMAC: " + WiFi.macAddress() +
      "\r\nCPU: " + _app->chipModelName() +
      "\r\nUTC: " + UTC.dateTime("Y-m-d H:i:s")
    );
  });  
}

void MqttApplication::setup() {
  Application::setup();

  // MQTT
  this->addComponent(_mqtt = new MqttComponent(
    _app->wifi()->wifiClient(),
    _app->config("mqtt-server"), atoi(_app->config("mqtt-port")),
    _app->config("mqtt-username"), _app->config("mqtt-password"),
    _app->hostname(),
    [](PubSubClient *client) -> void {
      // We have just connected to the broker. Subscribe to topics here if necessary
      // We subscribe to .../online to detect when someone else marks us as offline ("false")
      // This can happen when we reboot. Our last will is published after the existing session
      // disconnects but we have already marked ourselves as online by then so the will
      // message replaces ours. We will override the will message when we see it
      client->subscribe(_app->_onlinetopic.c_str());

      // Make sure we mark ourselves as online when we reconnect
      _app->publishProperty("online", "true", true);
    },
    [](const char *topic, const byte *payload, unsigned int length) -> void {
      // Unpack the message to a buffer
      char buffer[256];
      strncpy(buffer, (const char *)payload, length);
      buffer[length] = '\0';
      
      // If it is the online topic...
      if (strcmp(topic, _app->_onlinetopic.c_str()) == 0) {
        // See if we got "false"
        if (strcmp(buffer, "false") == 0)
          // If so, send "true"
          _app->publishProperty("online", "true", true);
      }
    }, 
    atoi(this->config("mqtt-interval", "300")) * 1000, // Check for lost connection every 5 minutes (default)
    this->_onlinetopic.c_str(),
    "false"
  ));

  // Auto-restart task
  if (this->_autoRestartTimeout > 0) {
    // Check every 15 minutes for an auto-restart
    _app->addTask("Check auto-restart", atoi(this->config("auto-restart-interval", "900")) * 1000, []()
    {
      if (_app->bootTimeUtc() == 0) {
        Log::logInformation("Uptime: time not available yet");
      } else {
        long uptime = _app->upTimeSeconds();
        long maxUptime = _app->_autoRestartTimeout * 60L;
        Log::logDebug("System uptime is %ld seconds (max. %ld)", uptime, maxUptime);
        // Have we reached out maximum up time?
        if (uptime >= maxUptime) {
          int autoRestartHour = atoi(_app->config("auto-restart-hour", "0"));
          if (_app->time()->TZ()->hour() >= autoRestartHour) {
            Log::logInformation("Uptime > %d minutes, restarting...", _app->_autoRestartTimeout);
            _app->publishProperty("autorestart", UTC.dateTime("Y-m-d H:i:s").c_str(), true);
            _app->mqtt()->mqttClient()->disconnect();
            delay(5000);
            ESP.restart();
          } else {
            Log::logInformation("Uptime > %ld minutes, but hour (%d) is not yet %d.", uptime / 60, _app->time()->TZ()->hour(), autoRestartHour);
          }
        }
      }
    });
  }

  // IP task: 10 minutes (600s)
  int ipInterval = atoi(this->config("ip-interval", "600"));
  if (ipInterval != 0) {
    this->addTask("Publish IP", ipInterval * 1000, []() {
      auto wifiAddress = WiFi.localIP().toString();
      Log::logInformation("IP-address is now %s", wifiAddress.c_str());
      _app->publishProperty("IP", wifiAddress.c_str());
    });
  }

  // Ping task: 15 minutes (900)
  int pingInterval = atoi(this->config("ping-interval", "900"));
  if (pingInterval != 0) {
    this->addTask("Ping", pingInterval * 1000, []() {
      if (_app->bootTimeUtc() != 0) {
        auto wifiAddress = WiFi.localIP().toString();
        auto pingMessage = (UTC.dateTime("Y-m-d H:i:s") + ": IP=" + wifiAddress + ";Up=" + _app->bootTimeUtcString() + ";");

        Log::logInformation("Ping '%s'", pingMessage.c_str());
        _app->publishData("ping", NULL, pingMessage.c_str(), true);
      }
    });
  }

  // Publish our application name and version
  this->publishProperty("application", this->title().c_str(), true);
  this->publishProperty("version", this->version().c_str(), true);
  // Mark ourselves as online (retained!)
  // publishpublishProperty("online", "true", true); // Do this on MQTT connected
}

void MqttApplication::loop() {
  Application::loop();
  // Increment our loop count
  this->_loopCount++;
}

void MqttApplication::setBootTimeUtc(time_t utc) {
  // Set the time in Application
  Application::setBootTimeUtc(utc);
  // Done initializing: publish our boot time
  this->publishData("boot", NULL, (this->bootTimeUtcString() + ": Up since " + this->bootTimeLocalString()).c_str(), true);
}

void MqttApplication::publishData(const char *channel, const char *property, const char *value, bool retained) {
  if (this->_mqtt != NULL) {
    char topic[256];
    if (property == NULL)
      snprintf(topic, sizeof(topic), "%s/%s/%s", this->_mqttPrefix.c_str(), channel, this->hostname());
    else
      snprintf(topic, sizeof(topic), "%s/%s/%s/%s", this->_mqttPrefix.c_str(), channel, this->hostname(), property);

    Log::logTrace("[MqttApplication] Publishing '%s' = '%s'%s", topic, value, (retained ? " (retained)": ""));
    _mqtt->mqttClient()->publish(topic, value, retained);
  }
}

void MqttApplication::publishProperty(const char *property, const char *value, bool retained)
{
  this->publishData("status", property, value, retained);
}

#include "MqttApplication.h"

#include "Logging.h"

MqttApplication::MqttApplication(
  const char *title, const char *version, const char *mqttPrefix,
  std::function<void(PubSubClient *client)> const onConnected,
  std::function<void(const char *topic, const byte *payload, unsigned int length)> const onReceived,
  uint16_t otaPortNumber,
  const char *configuration
) :
  Application(title, version, otaPortNumber, configuration),
  _mqtt(NULL),
  _mqttPrefix(mqttPrefix),
  _onlinetopic(String(mqttPrefix) + "/status/" + this->hostname() + "/online"),
  _loopCount(0),
  _autoRestartTimeout(Duration::parse(Application::config("auto-restart-timeout", "0"))),
  _isFirstConnect(true),
  _onMqttConnected(onConnected),
  _onMqttReceived(onReceived)
{
  Log::logDebug("[MqttApplication] Creating application '%s' v%s on '%s'", this->title().c_str(), this->version().c_str(), this->hostname(), this->_onlinetopic.c_str());
}

MqttApplication::MqttApplication(const char *title, const char *version, const char *mqttPrefix, uint16_t otaPortNumber, const char *configuration) :
  MqttApplication(title, version, mqttPrefix, NULL, NULL, otaPortNumber, configuration)
{
}

/*
 Set up the application. Calls the base class' setup(), then configures:

 - An MQTT component
 - Tasks for auto-reset, report IP, ping and memory/loop status
*/
void MqttApplication::setup() {
  Application::setup();

  // MQTT
  this->addComponent(_mqtt = new MqttComponent(
    this->wifi()->wifiClient(),
    this->config("mqtt-server"), atoi(this->config("mqtt-port")),
    this->config("mqtt-username"), this->config("mqtt-password"),
    this->hostname(),
    [this](PubSubClient *client) -> void {
      Log::logDebug("[MqttApplication] Connected to MQTT");

      // Publish our boot time when we connect for the first time
      if (this->_isFirstConnect && this->bootTimeUtc() != 0) {
        auto bootTime = (this->bootTimeUtcString() + ": Up since " + this->bootTimeLocalString());
        Log::logDebug("[MqttApplication] Publishing boot time: %s", bootTime.c_str());
        this->publishData("boot", NULL, bootTime.c_str(), true);
        Log::logDebug("[MqttApplication] Publishing MAC address: %s", WiFi.macAddress().c_str());
        this->publishProperty("MAC", WiFi.macAddress().c_str(), true);

        this->_isFirstConnect = false;
      }

      // We have just connected to the broker. Subscribe to topics here if necessary
      // We subscribe to .../online to detect when someone else marks us as offline ("false")
      // This can happen when we reboot. Our last will is published after the existing session
      // disconnects but we have already marked ourselves as online by then so the will
      // message replaces ours. We will override the will message when we see it
      client->subscribe(this->_onlinetopic.c_str());

      // Make sure we mark ourselves as online when we reconnect
      this->publishProperty("online", "true", true);

      if (this->_onMqttConnected != NULL) {
        Log::logTrace("[MqttApplication] Calling onMqttConnected");
        this->_onMqttConnected(client);
      }
    },
    [this](const char *topic, const byte *payload, unsigned int length) -> void {
      // Unpack the message to a buffer
      char buffer[256];
      strncpy(buffer, (const char *)payload, length);
      buffer[length] = '\0';

      Log::logDebug("[MqttApplication] Received '%s': '%s'", topic, buffer);

      // If it is the online topic...
      if (strcmp(topic, this->_onlinetopic.c_str()) == 0) {
        // See if we got "false"
        if (strcmp(buffer, "false") == 0)
          // If so, send "true"
          this->publishProperty("online", "true", true);
      } else {
        if (this->_onMqttReceived != NULL) {
          Log::logTrace("[MqttApplication] Calling onMqttReceived");
          this->_onMqttReceived(topic, payload, length);
        }
      }
    }, 
    // Check for lost connection every 5 minutes (default)
    Duration::parse(this->config("mqtt-interval", "5m")) * 1000,
    this->_onlinetopic.c_str(),
    "false"
  ));

  // Auto-restart task
  if (this->_autoRestartTimeout > 0) {
    // Check every 15 minutes for an auto-restart
    this->addTask("Check auto-restart", Duration::parse(this->config("auto-restart-interval", "15m")) * 1000, [this]()
    {
      if (this->bootTimeUtc() == 0) {
        Log::logInformation("Uptime: time not available yet");
      } else {
        long uptimeSeconds = this->upTimeSeconds();
        long maxUptime = this->_autoRestartTimeout;
        Log::logDebug("System uptime is %ld seconds (max. %ld)", uptimeSeconds, maxUptime);
        // Have we reached our maximum up time?
        if (uptimeSeconds >= maxUptime) {
          // Default time range is [0, 23] which is 'anytime'
          int autoRestartHourMin = atoi(this->config("auto-restart-hour-min", "0"));
          int autoRestartHourMax = atoi(this->config("auto-restart-hour-max", "23"));
          int localHour = this->time()->TZ()->hour();

          bool inRestartTimeRange = false;
          // Do we have 3-4? (Or 3-3)
          if (autoRestartHourMin <= autoRestartHourMax)
            // Then we're in range if 3 <= h <= 4
            inRestartTimeRange = localHour >= autoRestartHourMin && localHour <= autoRestartHourMax;
          else if (autoRestartHourMin > autoRestartHourMax)
            // We have 22-3 (meaning 22-03h), so we need >= 22 *OR* <= 3
            inRestartTimeRange = localHour >= autoRestartHourMin || localHour <= autoRestartHourMax;

          if (inRestartTimeRange) {
            Log::logInformation("Uptime > %d minutes, restarting...", this->_autoRestartTimeout);
            this->publishProperty("autorestart", (UTC.dateTime("Y-m-d H:i:s") + " (" + String(uptimeSeconds) + "s)").c_str(), true);
            // Perform a clean disconnect from MQTT
            this->mqtt()->mqttClient()->disconnect();
            // Wait a bit
            delay(5000);
            // Then restart
            ESP.restart();
          } else {
            Log::logInformation("Uptime > %ld minutes, but hour (%d) is not %d-%d.", uptimeSeconds / 60, this->time()->TZ()->hour(), autoRestartHourMin, autoRestartHourMax);
          }
        }
      }
    });
  }

  // IP task: 10 minutes (600s)
  int ipInterval = Duration::parse(this->config("ip-interval", "10m"));
  if (ipInterval > 0) {
    this->addTask("Publish IP", ipInterval * 1000, [this]() {
      auto wifiAddress = WiFi.localIP().toString();
      Log::logInformation("IP-address is now %s", wifiAddress.c_str());
      this->publishProperty("IP", wifiAddress.c_str());
    });
  }

  int rssiInterval = Duration::parse(this->config("rssi-interval", "1m"));
  if (rssiInterval > 0) {
    this->addTask("Publish RSSI", rssiInterval * 1000, [this]() {
      auto rssi = WiFi.RSSI();
      Log::logInformation("RSSI is now %d", rssi);
      this->publishProperty("RSSI", String(rssi).c_str());
    });
  }

  // Ping task: 15 minutes (900)
  int pingInterval = Duration::parse(this->config("ping-interval", "15m"));
  if (pingInterval > 0) {
    this->addTask("Ping", pingInterval * 1000, [this]() {
      if (this->bootTimeUtc() != 0) {
        auto wifiAddress = WiFi.localIP().toString();
        auto pingMessage = (UTC.dateTime("Y-m-d H:i:s") + ": IP=" + wifiAddress + ";Up=" + this->bootTimeUtcString() + ";");

        Log::logInformation("Ping '%s'", pingMessage.c_str());
        this->publishData("ping", NULL, pingMessage.c_str(), true);
      }
    });
  }

  // Free memory/loop count task
  auto memoryInterval = Duration::parse(this->config("memory-interval", "1m"));
  if (memoryInterval > 0) {
    this->addTask("Show memory/loop status", memoryInterval * 1000, [this]()
    {
      static unsigned long lastMs = 0;

      unsigned long ms = millis();
      int loopSpeed = 0;
      if (lastMs != 0) {
        float seconds = (ms - lastMs) / 1000.0;
        loopSpeed = (int)(this->_loopCount / seconds);
      }
      lastMs = ms;

      uint32_t freeSize = ESP.getFreeHeap();

      Log::logInformation("Free: %ld - Loop count: %d (%d/s)", freeSize, this->_loopCount, loopSpeed);
      this->publishProperty("free", String(freeSize).c_str());
      if (this->_loopCount > 1)
        this->publishProperty("loops", String(loopSpeed).c_str());
      this->_loopCount = 0;
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

// // Override of setBootTimeUtc, which is called when the time becomes available.
// // First set the time in the base class, then publish it using MQTT
// void MqttApplication::setBootTimeUtc(time_t utc) {
//   // Set the time in Application
//   Application::setBootTimeUtc(utc);
//   // Done initializing: publish our boot time
//   Log::logDebug("[MQttApplication] Publishing boot time");
//   this->publishData("boot", NULL, (this->bootTimeUtcString() + ": Up since " + this->bootTimeLocalString()).c_str(), true);
// }

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

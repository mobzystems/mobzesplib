/**
 * Complete example of an MQTT-enabled sensor-application (without the sensor)
 * 
 * Requires LittleFS to read /config.sys
 * 
 * config.sys should at least have
 
hostname=some-<your hostname>

wifi-ssid=<Your Wifi network name>
wifi-password=<Your Wifi password>

mqtt-username=<your MQTT username>
mqtt-password=<your MQTT password>
mqtt-server=<your MQTT broker>
mqtt-port=<your MQTT port number>

 * MQTT topics are:

MQTT_PREFIX/boot/<device> - boot time
MQTT_PREFIX/ping/<device> - ping
MQTT_PREFIX/status/<device>/<property> - a sensor reading or a property

IP is a predefined proerty

 */

#include <Application.h>
#include <MqttComponent.h>

// TODO: set application title and version
#define APP_TITLE "MQTT Application Example"
#define APP_VERSION "0.0.1"

#define MQTT_PREFIX "YOUR_MQTT_PREFIX/" // TODO

Application *_app;
MqttComponent *_mqtt = NULL;
String _boottimeUTC;
String _boottimeLocal;

void publishData(const char *channel, const char *property, const char *value, bool retained) {
  if (_mqtt != NULL) {
    char topic[256];
    if (property == NULL)
      snprintf(topic, sizeof(topic), MQTT_PREFIX "%s/%s", channel, _app->hostname());
    else
      snprintf(topic, sizeof(topic), MQTT_PREFIX "%s/%s/%s", channel, _app->hostname(), property);

    _mqtt->mqttClient()->publish(topic, value, retained);
  }
}

void publishProperty(const char *property, const char *value) {
  publishData("status", property, value, false);
}

void setup() {
  Serial.begin(230400);		// Initialize serial communications with the PC
  // while (!Serial);		    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)

  // TODO: choose a log level. Information is default
  Log::setSerialLogLevel(Log::LOGLEVEL::Debug);

  // TODO: set up hardware

  _app = new Application(APP_TITLE, APP_VERSION);
  _app->setup();

  // --- MQTT ---
  static String onlinetopic(String(MQTT_PREFIX "status/") + _app->hostname() + "/online");

  _app->addComponent(_mqtt = new MqttComponent(
    _app->wifi()->wifiClient(),
    _app->config("mqtt-server"), atoi(_app->config("mqtt-port")),
    _app->config("mqtt-username"), _app->config("mqtt-password"),
    _app->hostname(),
    [](PubSubClient *client) -> void {
      // We have just connected to the broker. Subscribe to topics here if necessary
    },
    [](const char *topic, const byte *payload, unsigned int length) -> void {
      // // Unpack the message to a buffer
      // char buffer[256];
      // strncpy(buffer, (const char *)payload, length);
      // buffer[length] = '\0';
    }, 
    5 * 60 * 1000, // Check for lost connection every 5 minutes
    onlinetopic.c_str(), // Last will topic
    "false" // Last will message
  ));
  // --- End MQTT ---

  // Wifi and time avilable: sample boot time
  _boottimeUTC = UTC.dateTime(_app->bootTimeUtc(), "Y-m-d H:i:s");
  _boottimeLocal = _app->time()->TZ()->dateTime("Y-m-d H:i:s");

  _app->enableConfigEditor("/config.sys");
  // _app->enableFileEditor("/read", "/write", "/edit");
  // _app->webserver()->serveStatic("/", LittleFS, "/wwwroot/");

  // IP task: 10 minutes
  _app->addTask("Publish IP", 10 * 60 * 1000, []() {
    auto wifiAddress = WiFi.localIP().toString();
    Log::logInformation("IP-address is now %s", wifiAddress.c_str());
    publishProperty("IP", wifiAddress.c_str());
  });

  // Ping task: 15 minutes. Sends IP-address and UTC boot time
  _app->addTask("Ping", 15 * 60 * 1000, []() {
    auto wifiAddress = WiFi.localIP().toString();
    auto pingMessage = (UTC.dateTime("Y-m-d H:i:s") + ": IP=" + wifiAddress + ";Up=" + _boottimeUTC + ";");

    Log::logInformation("Ping '%s'", pingMessage.c_str());
    publishData("ping", NULL, pingMessage.c_str(), true);
  });

  // TODO: sensor task
  // _app->addTask("Read Sensor", 1 * 60 * 1000, []() { // Every minute
  //   Log::logInformation("TODO: read sensor and publish to MQTT");
  //   publishProperty("YOUR_PROPERTY_HERE", "SENSOR_VALUE");
  // });

  // Done initializing: publish our boot time
  publishData("boot", NULL, (_boottimeUTC + ": Up since " + _boottimeLocal).c_str(), true);
  // Mark ourselves as online (retained!)
  publishData("status", "online", "true", true);
}

void loop() {
  _app->loop();
}

/**
 * Complete example of an MQTT-enabled sensor-application (without the sensor)
 * using MqttApplication.
 * 
 * Requires LittleFS to read /config.sys
 * 
 * config.sys should at least have
 
hostname=some-<your hostname>

wifi-ssid=<Your Wifi network name>
wifi-password=<Your Wifi password>

# Optional - this is the default
timezone=Europe/Amsterdam

mqtt-password=<your MQTT password>
mqtt-server=<your MQTT broker>
mqtt-port=<your MQTT port number>

 * MQTT topics are:

MQTT_PREFIX/boot/<device> - boot time
MQTT_PREFIX/ping/<device> - ping
MQTT_PREFIX/status/<device>/<property> - a sensor reading or a property

IP is a predefined property
*/

#include <MqttApplication.h>

// TODO: set application title and version
#define APP_TITLE "MQTT Application Example"
#define APP_VERSION "0.0.1"
 // TODO: set MQTT prefix
#define MQTT_PREFIX "YOUR_MQTT_PREFIX"

// The one and only global Application
MqttApplication *_app;

void setup() {
  Serial.begin(115200); // Initialize serial communications with the PC
  // while (!Serial);   // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)

  // TODO: choose a log level. Information is default
  Log::setSerialLogLevel(Log::LOGLEVEL::Debug);

  // Create the application
  // Use this if you don't need to subscribe to any topics:
  // _app = new MqttApplication(APP_TITLE, APP_VERSION, MQTT_PREFIX);
  // Use this version if you do need to subscribe and receive from Mqtt:
  _app = new MqttApplication(
    APP_TITLE, APP_VERSION, MQTT_PREFIX,
    // onConnected: called when the connection to the MQTT broker is (re)established
    [](PubSubClient *client) -> void {
      // We have just connected to the broker. Subscribe to topics here if necessary
      // using the client provided:
      auto commandTopic = (String(MQTT_PREFIX "/command/") + _app->hostname());
      Log::logInformation("Subscribing to command topic '%s'", commandTopic.c_str());
      client->subscribe(commandTopic.c_str());
    },
    // onReceived: called when a payload arrived in one of the topics we subsribed to
    [](const char *topic, const byte *payload, unsigned int length) -> void {
      // Unpack the message to a buffer
      // char buffer[256];
      // strncpy(buffer, (const char *)payload, length);
      // buffer[length] = '\0';
    }
  );
  _app->setup();

  // TODO: set up hardware

  // Optional features:
  _app->enableConfigEditor("/config.sys");
  // _app->enableFileEditor("/read", "/write", "/edit");
  // _app->webserver()->serveStatic("/", LittleFS, "/wwwroot/");

  // TODO: sensor task
  // _app->addTask("Read Sensor", 1 * 60 * 1000, []() { // Every minute
  //   Log::logInformation("TODO: read sensor and publish to MQTT");
  //   publishProperty("YOUR_PROPERTY_HERE", "SENSOR_VALUE");
  // });
}

void loop() {
  _app->loop();
}

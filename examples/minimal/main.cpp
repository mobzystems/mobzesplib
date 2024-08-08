/**
 * Complete example of an MQTT-enabled sensor-application (without the sensor)
 * 
 * Requires LittleFS to read /config.sys
 * 
 * config.sys should at least have
 
hostname=some-<your hostname>

wifi-ssid=<Your Wifi network name>
wifi-password=<Your Wifi password>

ota-username=admin
ota-password=test

# Optional - this is the default
timezone=Europe/Amsterdam

 * Library dependencies:

lib_deps = 
	knolleary/PubSubClient@^2.8
	ropg/ezTime@^0.8.3
	ayushsharma82/ElegantOTA@^3.1.2
	adafruit/DHT sensor library@^1.4.6
	adafruit/Adafruit Unified Sensor@^1.1.14
	adafruit/Adafruit SSD1306@^2.5.10
	https://github.com/mobzystems/mobzesplib

 */

#include <Application.h>

// TODO: set application title and version
#define APP_TITLE "Minimal mobzesplib Application Example"
#define APP_VERSION "0.0.1"

Application *_app;

void setup() {
  Serial.begin(115200);		// Initialize serial communications with the PC
  // while (!Serial);		    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)

  // TODO: choose a log level. Information is default
  Log::setSerialLogLevel(Log::LOGLEVEL::Debug);

  // TODO: set up hardware

  _app = new Application(APP_TITLE, APP_VERSION);
  _app->setup();

  _app->enableConfigEditor("/config.sys");
  // _app->enableFileEditor("/read", "/write", "/edit");
  // _app->webserver()->serveStatic("/", LittleFS, "/wwwroot/");

  // TODO: sensor task
  // _app->addTask("Read Sensor", 1 * 60 * 1000, []() { // Every minute
  //   Log::logInformation("TODO: read sensor and publish to MQTT");
  // });

  // Boot time may not be vailable yet -- check this->bootTimeUtc() == 0 if you want to make sure it's set
  Log::logInformation("Application booted at %s (%s UTC)", _app->bootTimeLocalString().c_str(), _app->bootTimeUtcString().c_str());
}

void loop() {
  _app->loop();
}

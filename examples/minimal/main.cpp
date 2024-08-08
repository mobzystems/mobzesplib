/**
 * Example of an bare sensor-application (without the sensor)
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

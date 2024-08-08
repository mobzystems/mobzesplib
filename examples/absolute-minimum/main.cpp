/**
 * Bare minimum mobzesplib application
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

Application _app("Application title", "0.0.1");

void setup() {
  // Initialize serial communications
  Serial.begin(115200);		

  // Choose a log level for the serial output. Default is Information
  Log::setSerialLogLevel(Log::LOGLEVEL::Debug);

  _app.setup();
}

void loop() {
  _app.loop();
}

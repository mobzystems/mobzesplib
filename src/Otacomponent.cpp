#include <Application.h>
#include <OtaComponent.h>

OtaComponent::OtaComponent(uint16_t portNumber) :
  Component("OTA"),
  _webServer(new WEBSERVER(portNumber))
{}

void OtaComponent::setup() {
  Log::logInformation("[%s] Configuring OTA", this->name());
  this->_webServer->begin();
  
  // We manage reboots ourselves
  ElegantOTA.setAutoReboot(false);

  // Start handler: log
  ElegantOTA.onStart([this]() {
    Serial.println("OTA update process started.");
    Log::logWarning("[%s] Starting OTA update", this->name());
  });

  // End handler: log, restart if successful
  ElegantOTA.onEnd([this](bool success) {
    if (success) {
      // Serial.println("OTA update completed successfully.");
      Log::logWarning("[%s] OTA update succeeded. Restarting...", this->name());
      Application::app()->scheduleRestart(3000);
    } else {
      // Serial.println("OTA update failed.");
      Log::logError("[%s] OTA update failed!", this->name());
    }
  });

  // Progress handler: log to serial. DO NOT log to Log because that will crash!
  // ElegantOTA.onProgress([](size_t current, size_t final) {
  //   Serial.printf("OTA update progress: %u%%\n", (current * 100) / final);
  //   // Log::logDebug("[%s] OTA update progress: %u%%\n", this->name(), (current * 100) / final);
  // });

  // Start the OTA web server
  ElegantOTA.begin(this->_webServer);
}

void OtaComponent::loop() {
  _webServer->handleClient();
  ElegantOTA.loop();
}
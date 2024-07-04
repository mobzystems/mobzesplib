#include "OtaComponent.h"

OtaComponent::OtaComponent(uint16_t portNumber) :
  Component("OTA"),
  _webServer(new WEBSERVER(portNumber))
{}

void OtaComponent::setup() {
  Log::logInformation("Configure OTA");
  this->_webServer->begin();
  ElegantOTA.begin(this->_webServer);
}

void OtaComponent::loop() {
  _webServer->handleClient();
  ElegantOTA.loop();
}
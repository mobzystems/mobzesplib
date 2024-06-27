#include "OledComponent.h"

#include <Wire.h>

OledComponent::OledComponent(int sda, int scl, uint8_t address) :
  Component("OLED"),
  _sda(sda),
  _scl(scl),
  _address(address)
{}

void OledComponent::setup() {
  // Start I2C Communication SDA = 5 and SCL = 4 on Wemos Lolin32 ESP32 with built-in SSD1306 OLED
  Wire.begin(this->_sda, this->_scl);

  this->_display = new Adafruit_SSD1306(128, 64, &Wire, -1);
  if(!this->_display->begin(SSD1306_SWITCHCAPVCC, this->_address, false, false)) {
    Log::logError("SSD1306 allocation failed");
  } else {
    this->_display->clearDisplay();
    this->_display->display();

    this->_display->invertDisplay(false);
    delay(1000);
    this->_display->invertDisplay(true);
    delay(1000);
    this->_display->invertDisplay(false);

    this->_display->setTextColor(WHITE);
    this->_display->setTextSize(1);
    this->_display->setCursor(0, 0);
    Log::logDebug("Display initialized.");
  }
}

void OledComponent::loop() {
}

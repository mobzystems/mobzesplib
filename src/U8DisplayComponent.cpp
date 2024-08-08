#include "U8DisplayComponent.h"

U8DisplayComponent::U8DisplayComponent(U8X8 *display) :
  Component("U8Display"),
  _display(display)
{}

void U8DisplayComponent::setup() {
  this->_display->begin();
  this->_display->setPowerSave(0);

  this->_display->setFont(u8x8_font_chroma48medium8_r);
  // this->_display->setFont(u8x8_font_7x14B_1x2_f);
  this->_display->clear();
  // this->_display->drawString(0, 1,"Hello World!");
  // this->_display->setInverseFont(1);
  // this->_display->drawString(0, 0,"012345678901234567890123456789");
  // this->_display->setInverseFont(0);
  // this->_display->drawString(0, 8,"Line 8");
  // this->_display->drawString(0, 9,"Line 9");
  this->_display->refreshDisplay();		// only required for SSD1606/7  
  // delay(4000);

    // this->_display->clearDisplay();
    // this->_display->display();

    // this->_display->invertDisplay(false);
    // delay(1000);
    // this->_display->invertDisplay(true);
    // delay(1000);
    // this->_display->invertDisplay(false);

    // this->_display->setTextColor(WHITE);
    // this->_display->setTextSize(1);
    // this->_display->setCursor(0, 0);

  Log::logDebug("U8Display initialized.");
}

void U8DisplayComponent::loop() {
}

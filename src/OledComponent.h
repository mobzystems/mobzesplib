#ifndef __OLED_COMPONENT_H__
#define __OLED_COMPONENT_H__

#include "components.h"
#include <Adafruit_SSD1306.h>

class OledComponent: public Component {
  private:
    int _sda;
    int _scl;
    int _address;
    Adafruit_SSD1306 *_display;

  public:
    OledComponent(int sda, int scl, uint8_t address);

    Adafruit_SSD1306 *getDisplay() { return this->_display; }
    
    void setup();
    void loop();
};
#endif
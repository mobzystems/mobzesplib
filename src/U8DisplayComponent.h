#ifndef __U8DISPLAY_COMPONENT_H__
#define __U8DISPLAY_COMPONENT_H__

#include "components.h"
#include <U8x8lib.h>

class U8DisplayComponent: public Component {
  private:
    U8X8 *_display;

  public:
    U8DisplayComponent(U8X8 *display);

    U8X8 *getDisplay() { return this->_display; }
    
    void setup();
    void loop();
};
#endif
#ifndef __OTA_COMPONENT_H__
#define __OTA_COMPONENT_H__

#include "Components.h"
#include "ESP_WebServer.h"
#include <ElegantOTA.h>

class OtaComponent: public Component {
  private:
    WEBSERVER *_webServer;

  public:
    OtaComponent(uint16_t portNumber);

    void setup();
    void loop();

    WEBSERVER *webserver() { return this->_webServer; }
};
#endif
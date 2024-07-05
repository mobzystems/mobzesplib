#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <LittleFS.h>

#include "Logging.h"
#include "Components.h"
#include "Configuration.h"
#include "WifiComponent.h"
#include "TimeComponent.h"
#include "TaskComponent.h"
#include "OtaComponent.h"

class Application {
  private:
  protected:
    Configuration *_configuration;
    Tasks *_tasks;
    WifiComponent *_wifi;
    TimeComponent *_time;
    OtaComponent *_ota;
    uint16_t _otaPortNumber;
    const char *_hostname;
    const String _macAddress;
    time_t _bootTimeUtc;
    unsigned long _restartDelay;

  protected:
    String makeHtml(const char *file, const char *message);
    String HtmlEncode(const char *s);

  public:
    Application(uint16_t otaPortNumber = 80, size_t maxConfigValues = 50);
    void addTask(String name, Milliseconds interval, void (*taskFunction)());
    void addComponent(Component *component);
    const char *config(const char *key, const char *defaultValue = NULL);

    void setup();
    void loop();

    TimeComponent *time() { return this->_time; }
    WifiComponent *wifi() { return this->_wifi; }
    WEBSERVER *webserver() { return this->_ota->webserver(); }

    const char *hostname() { return this->_hostname; }

    time_t bootTimeUtc() { return this->_bootTimeUtc; }
  
    static const char *configFileName;

    void mapGet(const char *path, void (*handler)());
    void mapPost(const char *path, void (*handler)());

    void requestReset(unsigned long delay) { _restartDelay = delay; }

    void enableConfigEditor(const char *path = "/config");
    void enableFileEditor(const char *readPath, const char *writePath, const char *editPath);
};

#endif

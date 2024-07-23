#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <LittleFS.h>

#include "Logging.h"
#include "Components.h"
#include "Configuration.h"
#include "WifiComponent.h"
#include "TimeComponent.h"
#include "TaskComponent.h"
#include "OledComponent.h"
#include "OtaComponent.h"

class Application {
  private:
  protected:
    String _title;
    String _version;
    Configuration *_configuration;
    Tasks *_tasks;
    WifiComponent *_wifi;
    TimeComponent *_time;
    OledComponent *_oled;
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
    Application(const char *title, const char *version = "", uint16_t otaPortNumber = 80, size_t maxConfigValues = 50);
    void addTask(String name, Milliseconds interval, void (*taskFunction)());
    void addComponent(Component *component);
    const char *config(const char *key, const char *defaultValue = NULL);

    virtual void setup();
    virtual void loop();
    virtual void setBootTimeUtc(time_t utc);

    TimeComponent *time() { return this->_time; }
    WifiComponent *wifi() { return this->_wifi; }
    WEBSERVER *webserver() { return this->_ota->webserver(); }

    const char *hostname() { return this->_hostname; }

    // The system boot time - can be 0 when time not (yet) available
    time_t bootTimeUtc() { return this->_bootTimeUtc; }
    long upTimeSeconds()  { return UTC.now() - this->bootTimeUtc(); }
  
    String bootTimeLocalString() { return this->bootTimeUtc() == 0 ? "" : this->time()->TZ()->dateTime(this->bootTimeUtc(), "Y-m-d H:i:s"); }
    String bootTimeUtcString() { return this->bootTimeUtc() == 0 ? "" : UTC.dateTime(this->bootTimeUtc(), "Y-m-d H:i:s"); }

    static const char *configFileName;

    void mapGet(const char *path, void (*handler)());
    void mapPost(const char *path, void (*handler)());

    void requestReset(unsigned long delay) { _restartDelay = delay; }

    void enableConfigEditor(const char *path = "/config");
    void enableFileEditor(const char *readPath, const char *writePath, const char *editPath);

    const String &title() { return this->_title; }
    const String &version() { return this->_version; }

    const String &chipModelName();

    void enableInfoPage(const char *path);
  
    void addOledDisplay(int sda, int scl, uint8_t address);
    Adafruit_SSD1306 *display();
};
#endif

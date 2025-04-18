#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <LittleFS.h>
#include "ESP_HttpClient.h"

#include "Duration.h"
#include "Logging.h"
#include "Components.h"
#include "Configuration.h"
#include "WifiComponent.h"
#include "TimeComponent.h"
#include "TaskComponent.h"
#include "DhtComponent.h"
#include "BluetoothSerialComponent.h"
#include "MqttComponent.h"
#include "OledComponent.h"
#include "U8DisplayComponent.h"
#include "OtaComponent.h"
#include "RotaryEncoderWatcherComponent.h"

#include <functional>

class Application {
  protected:
    // Application title and version
    String _title;
    String _version;
    // Various components:
    Configuration *_configuration;
    Tasks *_tasks;
    WifiComponent *_wifi;
    TimeComponent *_time;
    OledComponent *_oled;
    U8DisplayComponent *_u8x8;
    OtaComponent *_ota;
    // Other variables
    uint16_t _otaPortNumber;
    const char *_hostname;
    const String _macAddress;
    time_t _bootTimeUtc;
    time_t _bootTimeLocal;
    unsigned long _restartTimeMs;

    String makeHtml(const char *file, const char *message);
    String HtmlEncode(const char *s);
    void setBootTimeIfAvailable();

    static Application *_app;

    String readFile(const String &path, FS *fs = NULL);
    bool writeFile(const String &path, const char *s, FS *fs = NULL);
    bool deleteFile(const String &path, FS *fs = NULL);
    bool makeDirectory(const String &path, FS *fs = NULL);
    bool deleteDirectory(const String &path, FS *fs = NULL);
    String dir(const String &path, FS *fs = NULL);

    typedef struct FILESYSTEM_PREFIX {
      const char *prefix;
      FS *fileSystem;
      std::function<uint64_t()> const usedBytes;
      std::function<uint64_t()> const totalBytes;
    } FILESYSTEM_PREFIX;
    std::vector<FILESYSTEM_PREFIX> _fileSystems;

    void handleUpload(WEBSERVER *server);

  public:
    // Constructor
    Application(const char *title, const char *version = "", uint16_t otaPortNumber = 80, const char *configuration = NULL);

    // Get the single Application instance
    static Application *app() { return _app; }

    // Get a configuration value
    const char *config(const char *key, const char *defaultValue = NULL);

    // Components/tasks
    void addComponent(Component *component);
    void addTask(String name, Milliseconds interval, std::function<void()> const taskFunction);

    WifiComponent *wifi() { return this->_wifi; }
    TimeComponent *time() { return this->_time; }
    WEBSERVER *webserver() { return this->_ota->webserver(); }
  
    void addOledDisplay(int sda, int scl, uint8_t address);
    Adafruit_SSD1306 *display();

    void addU8Display(U8X8 *display);
    U8X8 *u8display() { return this->_u8x8 == NULL ? NULL : this->_u8x8->getDisplay(); }

    // Overridables
    virtual void setup();
    virtual void loop();

    // Informational
    const char *hostname() { return this->_hostname; }
    const String &title() { return this->_title; }
    const String &version() { return this->_version; }
    const String &chipModelName();

    // The system boot time - can be 0 when time not (yet) available
    time_t bootTimeUtc() { return this->_bootTimeUtc; }
    time_t bootTimeLocal() { return this->_bootTimeLocal; }
    long upTimeSeconds()  { return UTC.now() - this->bootTimeUtc(); }
  
    String bootTimeLocalString() { return this->bootTimeUtc() == 0 ? "" : this->time()->TZ()->dateTime(this->bootTimeLocal(), LOCAL_TIME, "Y-m-d H:i:s"); }
    String bootTimeUtcString() { return this->bootTimeUtc() == 0 ? "" : UTC.dateTime(this->bootTimeUtc(), "Y-m-d H:i:s"); }

    // Web server related
    void mapGet(const char *path, std::function<void(WEBSERVER *)> const handler);
    void mapPost(const char *path, std::function<void(WEBSERVER *)> const handler);

    // Add a file system with a prefix. The prefix / is already registered for LittleFS
    void addFileSystem(const char *prefix, FS *fs, std::function<uint64_t()> const usedBytes, std::function<uint64_t()> const totalBytes);
    // Get the file system and associated path for a file name
    void getFileSystemForPath(const String &path, FS **fsOut, String *pathOut);

    void enableConfigEditor(const char *path = "/config.sys");
    void enableFileEditor(const char *readPath = "/read", const char *writePath = "/write", const char *editPath = "/edit", const char *dirPath = "/dir", const char *deletePath = "/delete", const char *mkdirPath = "/mkdir", const char *rmdirPath = "/rmdir", const char *uploadPath = "/upload");

    void enableInfoPage(const char *path, std::function<void (String &)> const &postProcessInfo = NULL);

    // The name of the config file. Can be overriden BEFORE constructing the Application
    static const char *configFileName;
    // Schedule a reset after a delay
    void scheduleRestart(unsigned long delayMs);

    // Format a Duration as ....d, hh:mm:ss
    String formatDuration(const Duration &d);
};
#endif

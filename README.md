# mobzesplib

This is a sort-of application framework for ESP32/ESP8266 Arduino/[PlatformIO](https://platformio.org/) projects. Essentially, it provides a single Application class that offers a number of features, all configurable using a single text file in LittleFS. Most important among these are a WiFi connection, the current time, a built-in web server, OTA updating, and periodic task execution.

Also, there is a en extended class called MqttApplication, that layers Mqtt support on top of Application. It can connect to an Mqtt broker, and send and receive Mqtt messages. This makes it a great starting point for "sensor apps": applications that monitor some external data and report periodically to a central location.

## Installation

Just add a the URL of this repository to your platform.ini file:

`lib_deps = https://github.com/mobzystems/mobzesplib`

mobzesplib depends on other libraries but these will be installed automatically.

If not present already, configure your file system to LittleFS:

`board_build.filesystem = littlefs`

**Warning**: If you see errors about header files not being found, add the following to platform.ini:

`lib_ldf_mode = deep`

## Usage

### The application object

The main idea behing mobzesplib is to have a single instance of the `Application` class in your project. In the examples we call it `_app` but you can choose any other name.

#### Features

The application object has a number of features built-in:

- WiFi - it *connects to a WiFi network* on startup and periodically checks the connection, restoring it if possible
- Time - using WiFi, it queries the *current time* (using [ezTime](https://github.com/ropg/ezTime))
- A *web server* on port 80 (configurable in the `Application` constructor)
- *Over The Air (OTA) updating* of both the firmware and the contents of the file system (using [ElegantOTA](https://github.com/ayushsharma82/ElegantOTA))
- (optional) Configuration via a web page
- (optional) File system editing via a web page

#### Logging

To facilitate trouble shooting, mobzesplib uses logging throughout. Logging is independent of the application object: all funcionality is
in the `Log` class.

- `Log::setSerialLogLevel(Log::LOGLEVEL::Debug);`: Configure a minimal log level for `Serial` (the serial or USB port). All log messages with a *higher or equal* level are shown; those lower are suppressed. By default this is `Information`. The various log levels are:
    - Trace - Extremely verbose
    - Debug - Verbose
    - Information - Normal
    - Warning - Important
    - Critical - **Very** important
- `Log::logInformation("The value of the sensor is %d", value);` Do your own logging. Change `Information` to `Debug` etc. for the level you want. Note that this is a `printf()`-like method that **always adds a newline**. Note: the maximu length of a single log meesage is 256, as defined by `MAX_LOGMESSAGE_SIZE`. To override this, `#define MAX_LOGMESSAGE_SIZE xxx` to some other value before including `Application.h`.

#### Minimal code

You configure the application object, then call `_app.setup()` on it and add a call to its `_app.loop()` method in you own `loop()` method.

So, the absolute minimum to get the code for a running application is:

```
#include <Application.h>

Application _app("Application title", "0.0.1");

void setup() {
  // Initialize serial communications
  Serial.begin(115200);		

  // Choose a log level for the serial output. Default is Information
  Log::setSerialLogLevel(Log::LOGLEVEL::Debug);

  _app.setup();
}

void loop() {
  _app.loop();
}
```

After including `Application.h` we create the `_app` object, supplying a title and a version. (Normally you'd create a `const char *` for this, or a `#define` but this is minimal, remember?)

Then, from `setup()` we call `_app.setup()` to initialize the application and from our own `loop()` we call `_app.loop()`.

When run, this application will connect to a WiFi network using its host name, and connect to a time server. But which WiFi-network?

### Configuration

Most configuration required to get the app up and running comes from a configuration file in the file system. mobzesplib requires the `LittleFS` file system, with a single text file in the root called `/config.sys`. The minimum configuration is:

```
# REQUIRED: WiFi
hostname=mobzhub-default
wifi-ssid=YOUR_WIFI_SSID
wifi-password=YOUR_WIFI_PASSWORD

# Optional: OTA
ota-username=admin
ota-password=test

# Optional: time zone (defaults to Europe/Amsterdam)
timezone=YOUR_TIMEZONE
```

Using the WiFi configuration, the application will connect to WiFi and enable OTA updates. When both `ota-username` and `ota-password` are set, these are used to secure the OTA web page. After `setup()` the application's boot time is available in both local and UTC form.

There is a sample `config.sys` file in the `data` folder in the examples.

#### Custom configuration

You can add your own configuration entries. The application will read them and make them available through

`_app.config("key", "default-value");`

This returns a `const char *` that either points to the value of the key, or to `"default-value"` if the key was not present. You can supply NULL as a default value.

Note: the **first** value of the key encountered is used. If your configuration contains

```
key=one
...
key=two
```

then the value returned will be `"one"`.

#### Extras

`_app.enableConfigEditor("/config");`

Enable editing of the `/config.sys` configuration file at the path `/config`. Note: you can change the path but the name of the configuration file is *always* `/config.sys`!

`_app.enableFileEditor("/read", "/write", "/edit");`

Enable reading, writing and editing of any file in the file system at these paths. Needless to say: **dangerous, use at your own risk**. Doesn't have authentication (yet).

`_app.webserver()->serveStatic("/", LittleFS, "/wwwroot/");`

Enable serving of all files in the `/wwwroot` folder. Should be safe.

```
_app.mapGet("/info", [](WEBSERVER *server) {
    // ...
});
```
```
_app.mapPost("/info", [](WEBSERVER *server) {
    // ...
});
```

Simple web server mappings using lambda functions. The single argument to the lambda function is a pointer to a `WEBSERVER` which is an alias to the correct web server type for ESP32 or ESP8266. From within the lambda you can call `server->send()` etc.

`_app.enableInfoPage("/info")`

Uses `mapGet()` to display a simple text-only page on the specified path. Handy for testing.

#### Adding tasks

Most applications require some kind of periodic task to run, like sampling a sensor or reporting a heart beat. You can add a task using a single statement with a lambda:

```
_app.addTask("Sample the sensor", 60 * 1000, []() {
  // Sample the sensor here every 60 seconds (60 * 1000 milliseconds)
});
```

You can add any number of tasks this way. The application will call the supplied lambda function for each when it's its turn.

Note: Tasks are run from `_app.loop()` and are **not** interrupt or timer based. Therefore they're not accurate at the millisecond level. But ESP8266s loop around 20,000 times per second, and ESP32s at around 1,000 times per second, so your tasks will probably run on time. Using a task, you can avoid calling `delay()` and keep your process responsive. The application will not call `delay()` from its own loop.

## Notes

### Set lib_ldf_mode

Make sure to add

`lib_ldf_mode = deep`

to platform.ini if you have problems loading header files. This configures the [Library dependency Finder](https://docs.platformio.org/en/latest/librarymanager/ldf.html#ldf) to `deep` and allows mobzesplib to find the other libraries it depends on.

(Note on note: this should not be neccessary anymore from version 0.3.1 onwards. By restructuring the inclusion of header files the normal LDF mode ('chain') will find all header files.)

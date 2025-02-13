#ifndef __LOGGING_H__
#define __LOGGING_H__

#include <Arduino.h>
#include <ezTime.h>
#include <Print.h>
#include <vector>
#include <functional>

#define LOG_DEFAULT_TIME_FORMAT "Y-m-d H:i:s "

#ifndef MAX_LOGMESSAGE_SIZE
  #define MAX_LOGMESSAGE_SIZE 256
#endif

class Logger;

class Log
{
private:
  // The minimum log level. Messages with a lower level will NOT be shown
  // static LOGLEVEL logLevel; MOVED TO LOGGER
  // The Timezone object to use for time stamps
  static Timezone *timezone;
  // The format to use for logging
  static const char *timeFormat;

  static std::vector<Logger *> _loggers;
  
public:
  // The various log levels
  enum LOGLEVEL
  {
    Trace,
    Debug,
    Information,
    Warning,
    Error,
    Critical,
    None // No logging will happen
  };

  // Set the minimum log level for the serial logger, if present
  static void setSerialLogLevel(LOGLEVEL level);
  // Set the Timezone object to use for time stamps
  static void setTimezone(Timezone *timezone, const char *format = LOG_DEFAULT_TIME_FORMAT);

  // The function that does the logging
  static void logMessage(LOGLEVEL level, const char *format, ...);

  static void addLogger(Logger *logger);

  static LOGLEVEL parseLogLevel(String name, LOGLEVEL defaultLevel);

  // A callback function that gets called for each call to logMessage()
  // static void (*callback)(LOGLEVEL level, const char *message);

  // Various logging shorthands

  // Log a trace message
  static void logTrace(const char *format, ...);
  // Log a debug message
  static void logDebug(const char *format, ...);
  // Log an informational message
  static void logInformation(const char *format, ...);
  // Log a warning
  static void logWarning(const char *format, ...);
  // Log an error
  static void logError(const char *format, ...);
  // Log a critical message
  static void logCritical(const char *format, ...);

protected:
  // Interal method to support logXXX shorthands
  static void va_logMessage(LOGLEVEL level, const char *format, va_list args);
};

class Logger {
  private:
    const char *_name;
    Log::LOGLEVEL _minLevel;
    Print *_destination;
    std::function<void(const char *)> const _printFunction;

  public:
    // A logger with a Print destination
    Logger(const char *name, Log::LOGLEVEL minLevel, Print *destination) : _name(name), _minLevel(minLevel), _destination(destination), _printFunction(NULL) {}
    // A logger with some other println() function
    Logger(const char *name, Log::LOGLEVEL minLevel, std::function<void(const char *)> const printFunction) : _name(name), _minLevel(minLevel), _destination(NULL), _printFunction(printFunction) {}
    bool println(Log::LOGLEVEL level, const char *message);
    bool is(const char *name) { return strcmp(this->_name, name) == 0; }
    void setLogLevel(Log::LOGLEVEL level) { this->_minLevel = level; }
};

class SerialLogger: public Logger {
  public:
    static const char *name;
    SerialLogger(Log::LOGLEVEL minLevel = Log::LOGLEVEL::Information) : Logger(name, minLevel, &Serial) {}
};
#endif
#ifndef __LOGGING_H__
#define __LOGGING_H__

#include <eztime.h>
#include <Print.h>
#include <vector>

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

  // A callback function that gets called for each call to logMessage()
  // static void (*callback)(LOGLEVEL level, const char *message);
};

class Logger {
  private:
    const char *_name;
    Log::LOGLEVEL _minLevel;
    Print *_destination;
    void (*_printFunction)(const char *);

  public:
    // A logger with a Print destination
    Logger(const char *name, Log::LOGLEVEL minLevel, Print *destination) : _name(name), _minLevel(minLevel), _destination(destination), _printFunction(NULL) {}
    // A logger with some other println() function
    Logger(const char *name, Log::LOGLEVEL minLevel, void (*printFunction)(const char *)) : _name(name), _minLevel(minLevel), _destination(NULL), _printFunction(printFunction) {}
    bool println(Log::LOGLEVEL level, const char *message);
    bool is(const char *name) { return strcmp(this->_name, name) == 0; }
    void setLogLevel(Log::LOGLEVEL level) { this->_minLevel = level; }
};

class SerialLogger: public Logger {
  public:
    static const char *name;
    SerialLogger(Log::LOGLEVEL minLevel = Log::LOGLEVEL::Information) : Logger(name, minLevel, &Serial) {}
};

// Various logging macros
#define logTrace(format, ...) logMessage(Log::Trace, format __VA_OPT__(, ) __VA_ARGS__)
#define logDebug(format, ...) logMessage(Log::Debug, format __VA_OPT__(, ) __VA_ARGS__)
#define logInformation(format, ...) logMessage(Log::Information, format __VA_OPT__(, ) __VA_ARGS__)
#define logWarning(format, ...) logMessage(Log::Warning, format __VA_OPT__(, ) __VA_ARGS__)
#define logError(format, ...) logMessage(Log::Error, format __VA_OPT__(, ) __VA_ARGS__)
#define logCritical(format, ...) logMessage(Log::Critical, format __VA_OPT__(, ) __VA_ARGS__)

#endif
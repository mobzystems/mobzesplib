#include <eztime.h>

#ifndef __LOGGING_H__
#define __LOGGING_H__

#define LOG_DEFAULT_TIME_FORMAT "Y-m-d H:i:s "

class Log
{
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

  // Set the minimum log level. Messages with a lower level will NOT be shown
  static void setLogLevel(LOGLEVEL level);
  // Set the Timezone object to use for time stamps
  static void setTimezone(Timezone *timezone, const char *format = LOG_DEFAULT_TIME_FORMAT);

  // The function that does the logging
  static void logMessage(LOGLEVEL level, const char *format, ...);

  // A callback function that gets called for each call to logMessage()
  static void (*callback)(LOGLEVEL level, const char *message);

// Various logging macros
#define logTrace(format, ...) logMessage(Log::Trace, format __VA_OPT__(, ) __VA_ARGS__)
#define logDebug(format, ...) logMessage(Log::Debug, format __VA_OPT__(, ) __VA_ARGS__)
#define logInformation(format, ...) logMessage(Log::Information, format __VA_OPT__(, ) __VA_ARGS__)
#define logWarning(format, ...) logMessage(Log::Warning, format __VA_OPT__(, ) __VA_ARGS__)
#define logError(format, ...) logMessage(Log::Error, format __VA_OPT__(, ) __VA_ARGS__)
#define logCritical(format, ...) logMessage(Log::Critical, format __VA_OPT__(, ) __VA_ARGS__)

private:
  // The minimum log level. Messages with a lower level will NOT be shown
  static LOGLEVEL logLevel;
  // The Timezone object to use for time stamps
  static Timezone *timezone;
  // The format to use for logging
  static const char *timeFormat;
};
#endif
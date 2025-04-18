#include <Arduino.h>

#include "logging.h"

Timezone *Log::timezone = NULL;
const char *Log::timeFormat = LOG_DEFAULT_TIME_FORMAT;

// The default logger is the serial logger with level Information
// Call setSerialLogLevel to change this
std::vector<Logger *> Log::_loggers = { new SerialLogger(Log::LOGLEVEL::Information) };

// Add a logger to the collection of loggers
void Log::addLogger(Logger *logger) {
  _loggers.push_back(logger);
}

void Log::setSerialLogLevel(LOGLEVEL level)
{
  for (auto logger: Log::_loggers) {
    if (logger->is(SerialLogger::name)) {
      logger->setLogLevel(level);
      break;
    }
  }
}

Log::LOGLEVEL Log::parseLogLevel(String name, LOGLEVEL defaultLevel)
{
  if (name.equalsIgnoreCase("N") || name.equalsIgnoreCase("Off") || name.equalsIgnoreCase("None"))
    return LOGLEVEL::None;
  if (name.equalsIgnoreCase("T") || name.equalsIgnoreCase("TRC") || name.equalsIgnoreCase("Trace"))
    return LOGLEVEL::Trace;
  if (name.equalsIgnoreCase("D") || name.equalsIgnoreCase("DBG") || name.equalsIgnoreCase("Debug"))
    return LOGLEVEL::Debug;
  if (name.equalsIgnoreCase("I") || name.equalsIgnoreCase("INF") || name.equalsIgnoreCase("Information"))
    return LOGLEVEL::Information;
  if (name.equalsIgnoreCase("W") || name.equalsIgnoreCase("WRN") || name.equalsIgnoreCase("Warning"))
    return LOGLEVEL::Warning;
  if (name.equalsIgnoreCase("E") || name.equalsIgnoreCase("ERR") || name.equalsIgnoreCase("Error"))
    return LOGLEVEL::Error;
  if (name.equalsIgnoreCase("C") || name.equalsIgnoreCase("CRT") || name.equalsIgnoreCase("Critical"))
    return LOGLEVEL::Critical;
  
  // Return default if none
  return defaultLevel;
}

void Log::setTimezone(Timezone *timezone, const char *format)
{
  Log::timezone = timezone;
  Log::timeFormat = format;
}

String Log::getTimeStamp()
{
  if (Log::timezone != NULL && Log::timeFormat != NULL && timeStatus() != timeStatus_t::timeNotSet)
    return timezone->dateTime(timeFormat);
  else
    return emptyString;
}

void Log::va_logMessage(LOGLEVEL level, const char *format, va_list args)
{
  char loc_buf[MAX_LOGMESSAGE_SIZE + 1] = "";

  char *p = stpcpy(loc_buf, getTimeStamp().c_str());

  // // Add time information if available
  // if (timezone != NULL && timeFormat != NULL && timeStatus() != timeStatus_t::timeNotSet)
  //   p = stpcpy(p, timezone->dateTime(timeFormat).c_str());

  // Add the log level information
  const char *lvl;
  switch (level)
  {
  case Trace:
    lvl = "TRC: ";
    break;
  case Debug:
    lvl = "DBG: ";
    break;
  case Information:
    lvl = "INF: ";
    break;
  case Warning:
    lvl = "WRN: ";
    break;
  case Error:
    lvl = "ERR: ";
    break;
  case Critical:
    lvl = "CRT: ";
    break;
  default:
    lvl = "???: ";
    break;
  }
  p = stpcpy(p, lvl);

  // Print the message info the rest of the buffer
  vsnprintf(p, sizeof(loc_buf) - (p - loc_buf), format, args);
    for (auto logger: Log::_loggers)
      logger->println(level, loc_buf);
}

void Log::logMessage(LOGLEVEL level, const char *format, ...)
{
  va_list args;
  va_start(args, format);

  Log::va_logMessage(level, format, args);

  va_end(args);
}

// #define the implementation of LogXXX as a macro calling va_logMessage
#define LOGMESSAGE(LVL) \
void Log::log##LVL(const char *format, ...) { \
  va_list args; \
  va_start(args, format); \
  Log::va_logMessage(Log::LVL, format, args); \
  va_end(args); \
}

// These declare real methods, e.g. Log::logTrace(const char *format, ...)
LOGMESSAGE(Trace)
LOGMESSAGE(Debug)
LOGMESSAGE(Information)
LOGMESSAGE(Warning)
LOGMESSAGE(Error)
LOGMESSAGE(Critical)

bool Logger::println(Log::LOGLEVEL level, const char *message) {
  if (level < this->_minLevel)
    return false;

  if (this->_destination != NULL)  
    this->_destination->println(message);
  else if (this->_printFunction != NULL)
    this->_printFunction(message);
    
  return true;
}

const char *SerialLogger::name = "SerialLogger";
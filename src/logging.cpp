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

void Log::setTimezone(Timezone *timezone, const char *format)
{
  Log::timezone = timezone;
  Log::timeFormat = format;
}

void Log::logMessage(LOGLEVEL level, const char *format, ...)
{
  static bool in_callback = false;

  va_list args;
  va_start(args, format);

  char loc_buf[MAX_LOGMESSAGE_SIZE + 1] = "";

  char *p = loc_buf;

  // Add time information if available
  if (timezone != NULL && timeFormat != NULL && timeStatus() != timeStatus_t::timeNotSet)
    p = stpcpy(p, timezone->dateTime(timeFormat).c_str());

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

  va_end(args);
}

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
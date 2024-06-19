#include <Arduino.h>

#include "logging.h"

Log::LOGLEVEL Log::logLevel = Log::Information;
Timezone *Log::timezone = NULL;
const char *Log::timeFormat = LOG_DEFAULT_TIME_FORMAT;
void (*Log::callback)(LOGLEVEL level, const char *message) = NULL;

void Log::setLogLevel(LOGLEVEL level)
{
  Log::logLevel = level;
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

  if (level >= logLevel)
  {
    if (timezone != NULL && timeFormat != NULL && timeStatus() != timeStatus_t::timeNotSet)
    {
      Serial.print(timezone->dateTime(timeFormat).c_str());
    }
    char loc_buf[MAX_LOGMESSAGE_SIZE];
    int len = vsnprintf(loc_buf, sizeof(loc_buf), format, args);
    if (len >= 0 && (size_t)len < sizeof(loc_buf))
    {
      switch (level)
      {
      case Trace:
        Serial.print("TRC: ");
        break;
      case Debug:
        Serial.print("DBG: ");
        break;
      case Information:
        Serial.print("INF: ");
        break;
      case Warning:
        Serial.print("WRN: ");
        break;
      case Error:
        Serial.print("ERR: ");
        break;
      case Critical:
        Serial.print("CRT: ");
        break;
      default:
        Serial.print("???: ");
        break;
      }
      Serial.print(loc_buf);
      Serial.print("\n");

      // Call the callback (but not recursively!)
      if (!in_callback && callback != NULL) {
        in_callback = true;
        (*callback)(level, loc_buf);
        in_callback = false;
      }
    }
    else
    {
      Serial.printf("[Log::logMessage] Error: vsnprintf returned %d\n", len);
    }
  }
  va_end(args);
}

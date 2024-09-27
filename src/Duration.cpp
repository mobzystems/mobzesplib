#include "Duration.h"

#include <string.h>
#include <ctype.h>

#include "Logging.h"

Duration::Duration(unsigned int totalSeconds) {
  this->seconds = (uint8_t)(totalSeconds % 60);
  totalSeconds /= 60;
  this->minutes = (uint8_t)(totalSeconds % 60);
  totalSeconds /= 60;
  this->hours = (uint8_t)(totalSeconds % 24);
  totalSeconds /= 24;
  this->days = (uint8_t)totalSeconds;
}

unsigned int Duration::totalSeconds() {
  return this->seconds + 60U * (this->minutes + 60U * (this->hours + 24U * this->days));
}

unsigned int Duration::parse(const char *input, char default_unit) {
  unsigned int seconds = 0;

  // Start parsing.
  const char *p = input;
  for (;;) {
    // Allow spaces and some punctuation characters as a delimiter, e.g. 1d,5h or even "   1d   ;  5m"
    // This is very permissive: leading delimiters are acceptable, e.g. ,,1d:5h
    if (*p == ' ' || *p == ',' || *p == '.' || *p == ';' || *p == ':' || *p == '/')
      p++;
    // No more? Done
    if (*p == '\0')
      break;

    // "Eat" digits to form a number
    unsigned int number = 0;
    while (isdigit(*p)) {
      int n = *p - '0';
      number = number * 10 + n;
      p++;
    }

  	// No spaces between number and unit!
    char unit = *p++;
    // Unit may be missing:
    if (unit == '\0') {
      unit = default_unit;
      // Undo the unit so we break after it
      p--;
    }
    // Log::logTrace("[Duration] Parsing %d [%c] in duration '%s'", number, unit, input);

    switch (unit) {
      case 's': seconds += number; break;
      case 'm': seconds += number * 60; break;
      case 'h': seconds += number * 60 * 60; break;
      case 'd': seconds += number * 60 * 60 * 24; break;
      case 'w': seconds += number * 60 * 60 * 24 * 7; break;
      // Unknown unit: ignore
      default:
        Log::logError("[Duration] Unknown unit '%c' in duration '%s'", unit, input);
        break;
    }
  }

  Log::logTrace("[Duration] '%s' -> %d seconds", input, seconds);
  return seconds;
}

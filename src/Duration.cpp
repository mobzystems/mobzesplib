#include "Duration.h"

#include <string.h>
#include <ctype.h>

#include "Logging.h"

int Duration::parse(const char *input, char default_unit) {
  int seconds = 0;

  // Start parsing.
  const char *p = input;
  for (;;) {
    // // Skip spaces
    // while (*p == ' ') p++;
    // // No more? Done
    // if (*p == '\0')
    //   break;

    // Allow spaces and some punctuation characters as a delimiter, e.g. 1d,5h or even "   1d   ;  5m"
    if (*p == ' ' || *p == ',' || *p == '.' || *p == ';' || *p == ':' || *p == '/')
      p++;
    // No more? Done
    if (*p == '\0')
      break;

    // "Eat" digits to form a number
    int number = 0;
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
};

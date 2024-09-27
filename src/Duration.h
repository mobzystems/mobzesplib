#ifndef __DURATION_H__
#define __DURATION_H__

#include <stdio.h>

/**
 * Duration class to parse string of the format 1d2h6m into a number of seconds.
 */
struct Duration {
  uint16_t days;
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;

  // Constructor with days, hours, minutes, seconds
  Duration(uint16_t days, uint8_t hours, uint8_t minutes, uint8_t seconds) :
    days(days), hours(hours), minutes(minutes), seconds(seconds) {}

  // Constructor with days = 0
  Duration(uint8_t hours, uint8_t minutes, uint8_t seconds) : Duration(0, hours, minutes, seconds) {}

  // Constructor from total number of seconds
  Duration(unsigned int totalSeconds);

  // Calculate total number of seconds, handy for normalized()
  unsigned int totalSeconds();

  /**
   * Parse a string into a number of seconds.
   * input: string of the form <group>[<delimiter><group>]+
   * where <delimiter> is one or more spaces, commas, periods, colon, semicolons or slashes
   * where <group> is a number followed by a unit. The number must be one or more digits long,
   * and not contain a decimal point or comma. The unit may be
   * 
   * s: seconds
   * m: minutes
   * h: hours
   * d: days
   * w: weeks
   * 
   * Valid durations are:
   * 
   * 1m -> 60
   * 1m20s -> 80
   * 1m20 -> 80 (when seconds is the default unit)
   * 1m;20s -> 80
   * 1m   ;  20s -> 80
   * 3h20m5s -> 3 hours, 20 minutes, 5 seconds = 12005 seconds
   */
  static unsigned int parse(const char *input, char default_unit = 's');

  // Return a normalized version of ourselves, e.g. one where 25h70m is 1d2h10m
  Duration normalized() { return Duration(this->totalSeconds()); }
};
#endif

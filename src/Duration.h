#ifndef __DURATION_H__
#define __DURATION_H__

#include <stdio.h>

/**
 * Duration class to parse string of the format 1d2h6m into a number of seconds.
 */
class Duration {
public:
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
  static int parse(const char *input, char default_unit = 's');
};
#endif

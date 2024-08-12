#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__

#include <vector>
#include <fs.h>
#include "logging.h"

class Configuration {
  private:
    size_t count;
    char *buffer;
    std::vector<const char *> keys;
    std::vector<const char *> values;

  public:
    // Read configuration from a file
    Configuration(FS *fileSystem, const char *filename, size_t initial_values);
    // Cleanup
    ~Configuration();

    // Log the configuration
    void log(Log::LOGLEVEL level = Log::Information);
    // Get a configuration value
    const char *value(const char *key, const char *defaultValue);
};
#endif
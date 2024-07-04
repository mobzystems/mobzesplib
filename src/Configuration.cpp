#include "configuration.h"

// Read the configuration from a file
Configuration::Configuration(FS *fileSystem, const char *filename, size_t max_values) :
  count(0),
  buffer(NULL),
  keys(NULL),
  values(NULL)
{
  File file = fileSystem->open(filename, "r");

  if (!file)
  {
    Log::logWarning("[Configuration] Could not open '%s'", filename);

    return;
  }

  // Allocate a buffer large enough to hold the entire file
  size_t size = file.size();
  this->buffer = new char[size + 1];
  // Read the file and 0-terminate it
  file.readBytes(this->buffer, size);
  buffer[size] = '\0';
  // Close the file
  file.close();

  // Create storage for the key/value pairs
  const char *keys[max_values];
  const char *values[max_values];

  size_t count = 0;

  // Split the data into individual lines and then key=value pairs

  // Tokenize with \n:
  char *token = strtok(buffer, "\n");
  while (token != NULL)
  {
    char *line = token;

    // Remove a trailing CR if present
    size_t n = strlen(line);
    if (n > 0 && line[n - 1] == '\r')
      line[n - 1] = '\0';

    if (*line == '\0')
    {
      // Skip empty lines
    }
    else if (line[0] == '#')
    {
      // Skip lines starting with #
      Log::logTrace("[Configuration] Skipping comment '%s'", line);
    }
    else
    {
      // Check for overflow:
      if (count >= max_values)
      {
        Log::logWarning("[Configuration] Too many key/value pairs, maximum is %d", max_values);
        break;
      }

      // Expect key=value (no spaces!) or simply key (no value)
      char *equal = strchr(line, '='); // Find separating =
      const char *value = "";          // Assume no value
      const char *key = line;          // And key == line
      if (equal != NULL)
      {
        // If separator found:
        *equal = '\0';     // Split key here
        value = equal + 1; // Value is rest of string
      }

      Log::logTrace("[Configuration] Found [%s] = '%s'", key, value);

      keys[count] = key;
      values[count] = value;
      count++;
    }

    // Get the next line
    token = strtok(NULL, "\n");
  }

  // Copy the results:
  this->keys = new const char *[count];
  this->values = new const char *[count];

  for (size_t i = 0; i < count; i++)
  {
    this->keys[i] = keys[i];
    this->values[i] = values[i];
  }

  this->count = count;
}

// Clean up configuration data. Free the file buffer
Configuration::~Configuration()
{
  Log::logTrace("[Configuration] Cleaning up");
  if (this->keys != NULL)
    delete this->keys;
  if (this->values != NULL)
    delete this->values;
  if (this->buffer != NULL)
    delete buffer;
  this->count = 0;
}

// Send the configuration to log
void Configuration::log(Log::LOGLEVEL level)
{
  for (size_t i = 0; i < this->count; i++)
    Log::logMessage(level, "Configuration: [%s] = '%s'", this->keys[i], this->values[i]);
}

// Get a configuration value from a key. If the key was not found, return a default value
const char *Configuration::value(const char *key, const char *defaultValue)
{
  for (size_t i = 0; i < this->count; i++)
  {
    if (strcmp(this->keys[i], key) == 0)
      return this->values[i];
  }

  return defaultValue;
}

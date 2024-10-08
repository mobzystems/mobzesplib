#include "configuration.h"

// Read the configuration from a file
Configuration::Configuration(const char *configuration, size_t initial_values) :
  count(0),
  buffer(new char[strlen(configuration) + 1])
{
  // Copy the configuration to the buffer
  strcpy(buffer, configuration);
  // Initialize from the buffer
  this->readFromBuffer(initial_values);
}

// Read the configuration from a file
Configuration::Configuration(FS *fileSystem, const char *filename, size_t initial_values) :
  count(0),
  buffer(NULL)
{
  // Read the configuration file into memory
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

  // Initialize our variables from the buffer
  this->readFromBuffer(initial_values);
}

// Clean up configuration data. Free the file buffer
Configuration::~Configuration()
{
  Log::logTrace("[Configuration] Cleaning up");
  if (this->buffer != NULL)
    delete buffer;
  this->count = 0;
}

void Configuration::readFromBuffer(size_t initial_values)
{
  // Create storage for the key/value pairs. This is only a first size to prevent too many allocactions
  this->keys.reserve(initial_values);
  this->values.reserve(initial_values);

  // Split the data into individual lines and then key=value pairs

  // Tokenize with \n:
  char *token = strtok(this->buffer, "\n");
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

      this->keys.push_back(key);
      this->values.push_back(value);
      this->count++;
    }

    // Get the next line
    token = strtok(NULL, "\n");
  }
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

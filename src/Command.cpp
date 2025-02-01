#include "Command.h"

Command::Command(const char *command) {
  // Make a copy of the command
  char *buffer = new char[strlen(command) + 1];
  strcpy(buffer, command);

  // Split buffer into arguments separated by /
  const char *slash = "/";

  char *token = strtok(buffer, slash);
  while (token != NULL)
  {
    // Add argument to list
    this->_arguments.push_back(String(token));
    // Get the next token
    token = strtok(NULL, slash);
  }

  delete buffer;
}

String Command::arg(int i, const char *defaultValue) {
  if (i <= 0)
    return "[Invalid]";
  if (i < this->_arguments.size())
    return this->_arguments.at(i);
  return defaultValue;
}
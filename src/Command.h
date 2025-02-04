#include <Arduino.h>
#include <vector>

class Command {
private:
  // A vector of strings. The first element is the command name, the arguments start at index 1
  std::vector<String> _arguments;
public:
  // Constructor from a single string, e.g. /test/1/2/3
  Command(const char *command);
  // Compare the name of the command (argument #0) to the specified name
  bool is(const char *name) { return this->_arguments.size() > 0 && strcmp(this->_arguments.at(0).c_str(), name) == 0; }
  // Get the number of arguments EXCLUDING THE COMMAND NAME
  size_t arguments() { return this->_arguments.size() - 1; }
  // Get the nth argument. If this more than the actual number of arguments. return the default value specified
  String arg(size_t n, const char *defaultValue);
};
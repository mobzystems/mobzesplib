#include "components.h"
#include "logging.h"

// Create a component with a name
Component::Component(const char *name) : _name(name) {}

// Get the name of a component
const char *Component::name() { return this->_name.c_str(); }

void Component::setStatus(int statusCode, Log::LOGLEVEL level, const char *format, ...) {
  va_list args;
  va_start(args, format);
  char loc_buf[MAX_LOGMESSAGE_SIZE];
  int len = vsnprintf(loc_buf, sizeof(loc_buf), format, args);
  if (len >= 0 && (size_t)len < sizeof(loc_buf))
  {
    if (Components::status == NULL)
      Log::logMessage(level, loc_buf);
    else {
      Log::logTrace("[Components] [%s] Setting status to '%s', (%d)", name(), loc_buf, level);
      Components::status(name(), statusCode, level, loc_buf);
    }
  }
  else
    Log::logWarning("[Component::setStatus] vsnprintf returned %d\n", len);
}

void Component::setStatus(int statusCode) {
  Component::setStatus(statusCode, Log::LOGLEVEL::Information, "Setting state to %d", statusCode);
}

// The "global" list of components
std::vector<Component *> Components::components;
// The global status handler to allow components to report stuff during setup()
void (*Components::status)(const char*name, int statusCode, Log::LOGLEVEL, const char *message) = NULL;

// Add a component to the list and return it
Component *Components::add(Component *component)
{
    Log::logDebug("[Components] Adding component '%s'", component->name());
    Components::components.push_back(component);
    Log::logTrace("[Components] Calling setup() on component '%s'", component->name());
    component->setup();
    return component;
}

// Call each component's setup() method
void Components::loop()
{
  for (auto component: components)
    component->loop();
}
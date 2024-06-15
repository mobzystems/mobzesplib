#include "components.h"
#include "logging.h"

// Create a component with a name
Component::Component(const char *name) : _name(name) {}
// Get the name of a component
const char *Component::name() { return this->_name.c_str(); }

// The "global" list of components
std::vector<Component *> Components::components;

// Add a component to the list
void Components::add(Component *component) {
    Log::logDebug("[Components] Adding component '%s'", component->name());
    Components::components.push_back(component);
}

void Components::setup() {
  for (int i = 0; i < components.size(); i++) {
    Log::logDebug("[Components] Calling setup() on component '%s'", components[i]->name());
    components[i]->setup();
  }
}

void Components::loop() {
  for (int i = 0; i < components.size(); i++)
    components[i]->loop();
}
#include "components.h"
#include "logging.h"

// Create a component with a name
Component::Component(const char *name) : _name(name) {}

// Get the name of a component
const char *Component::name() { return this->_name.c_str(); }

// The "global" list of components
std::vector<Component *> Components::components;

// Add a component to the list and return it
Component *Components::add(Component *component)
{
    Log::logDebug("[Components] Adding component '%s'", component->name());
    Components::components.push_back(component);
    Log::logTrace("[Components] Calling setup() on component '%s'", component->name());
    component->setup();
    return component;
}

// // Call each component's setup() method
// void Components::setup()
// {
//   for (auto component: components) {
//     Log::logDebug("[Components] Calling setup() on component '%s'", component->name());
//     component->setup();
//   }
// }

// Call each component's setup() method
void Components::loop()
{
  for (auto component: components)
    component->loop();
}
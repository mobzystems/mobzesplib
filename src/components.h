#ifndef __COMPONENTS_H__
#define __COMPONENTS_H__

#include <Arduino.h>
#include "logging.h"
#include <vector>

/***
 * Base class for a component that has a name, plus setup() and loop() functions
 */
class Component {
  private:
    String _name;

  public:
    Component(const char *name);
    const char *name();

    // Pure virtual (must-override) functions
    virtual void setup() = 0;
    virtual void loop() = 0;
};

/***
 * Static class for registering components and calling setup()/loop() on them
 * When a component is added, its setup() method is called.
 * 
 * Usage:
 *  In setup():
 *    Components::add(new SomeComponent(...));
 *    Components::add(new SomeOtherComponent(...));
 * 
 *  In loop():
 *    Components::loop();
 */
class Components {
  private:
    // The list of components
    static std::vector<Component *> components;

  public:
    // Add a component and return it
    static Component *add(Component *component);
    // Call loop() on all components
    static void loop();
};
#endif
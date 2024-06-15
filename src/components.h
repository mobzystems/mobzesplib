#ifndef __COMPONENTS_H__
#define __COMPONENTS_H__

#include <Arduino.h>
#include "logging.h"
#include <vector>

/*
  Base class for a component that has a name, plus setup() and loop() functions
*/
class Component {
  private:
    String _name;

  public:
    Component(const char *name);
    const char *name();

    // Pure virtual (must-oerride) functions
    virtual void setup() = 0;
    virtual void loop() = 0;
};

/*
  Static class for registering components and calling loop() on them
*/
class Components {
  private:
    static std::vector<Component *> components;

  public:
    static void add(Component *component);
    // Call setup() on all components
    static void setup();
    // Call loop() on all components
    static void loop();
};
#endif
#ifndef __TASK_COMPONENT_H__
#define __TASK_COMPONENT_H__

#include <Arduino.h>
#include <vector>

#include "components.h"

typedef unsigned long Milliseconds;

/***
 * A Task object is a named function that should run every [interval] milliseconds
 * Running Task objects is done by the Tasks component
*/
class Task
{
  private:
    String _name;
    Milliseconds _interval;
    Milliseconds _nextRunTime;
    std::function<void()> const _taskFunction;

  public:
    // Constructor with a task name, interval and function
    // If the task function returns true, it continues to run
    Task(String name, Milliseconds interval, std::function<void()> const taskFunction);

    // Run the task if it should
    void runIfRequired(Milliseconds currentMilliseconds);
};

/***
 * The Tasks component manages a list of Task objects
 * and runs them if in order. There should normally be a single Tasks object
 */
class Tasks: public Component {
  private:
    // The list of tasks
    std::vector<Task *> tasks;

  public:
    Tasks();

    // Add a task to the list
    int add(String name, Milliseconds interval, std::function<void()> const taskFunction);

    // Component implementations
    void setup();
    void loop();
};
#endif
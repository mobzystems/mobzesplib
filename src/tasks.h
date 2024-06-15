#ifndef __TASKS_H__
#define __TASKS_H__

#include <Arduino.h>
#include <vector>

#include "components.h"

typedef unsigned long Milliseconds;

class Task
{
  private:
    String _name;
    Milliseconds _interval;
    Milliseconds _nextRunTime;
    bool (*_taskFunction)(Task *task);

  public:
    // Task();
    // Constructor with a task. If the task function returns true, it continues to run
    Task(String name, Milliseconds interval, bool (*taskFunction)(Task *task));
    void runIfRequired(Milliseconds currentMilliseconds);
};

class Tasks: public Component {
  private:
    // The list of tasks
    std::vector<Task *> tasks;

  public:
    Tasks();

    int add(String name, Milliseconds interval, bool (*taskFunction)(Task *task));

    // Component implementations
    void setup();
    void loop();
};
#endif
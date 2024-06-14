#include <Arduino.h>

#ifndef __TASKS_H__
#define __TASKS_H__

#define MAX_TASKS 10
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

    // The global list of MAX_TASKS tasks
    static Task **tasks;
    // The number of global tasks
    static int taskCount;

    static int add(String name, Milliseconds interval, bool (*taskFunction)(Task *task));

    static void loop();
};

#endif
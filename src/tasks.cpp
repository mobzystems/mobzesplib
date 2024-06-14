#include "tasks.h"
#include "logging.h"

Task::Task(String name, Milliseconds interval, bool (*taskFunction)(Task *task))
{
  this->_name = name;
  this->_interval = interval;
  this->_taskFunction = taskFunction;
  this->_nextRunTime = 0; // ASAP
}

Task **Task::tasks = new Task *[MAX_TASKS];

int Task::taskCount = 0;

// Add a task. return the task index
int Task::add(String name, Milliseconds interval, bool (*taskFunction)(Task *task))
{
  if (taskCount >= MAX_TASKS) {
    Log::logCritical("Maximum number of tasks reached. Configure MAX_TASKS (now %d)", MAX_TASKS);
    return -1;
  }
  Task *newTask = new Task(name, interval, taskFunction);
  tasks[taskCount] = newTask;
  // Return AND INCREMENT task count
  return taskCount++;
}

void Task::loop()
{
  Milliseconds currentMilliseconds = millis();
  for (int i = 0; i < taskCount; i++)
  {
    Task *task = tasks[i];
    if (currentMilliseconds >= task->_nextRunTime)
    {
      if (task->_taskFunction != NULL)
      {
        Log::logTrace("Running task '%s'...", task->_name.c_str());
        bool result = (*task->_taskFunction)(task);
        if (result) {
          task->_nextRunTime = currentMilliseconds + task->_interval;
        } // TODO: else?
      }
    }
  }
}

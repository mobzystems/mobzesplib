#include "tasks.h"
#include "logging.h"

Task::Task(String name, Milliseconds interval, bool (*taskFunction)(Task *task)) :
  _name(name),
  _interval(interval),
  _taskFunction(taskFunction),
  _nextRunTime(0) // ASAP
{}

void Task::runIfRequired(Milliseconds currentMilliseconds) {
  if (currentMilliseconds >= this->_nextRunTime)
  {
    if (this->_taskFunction != NULL)
    {
      Log::logTrace("Running task '%s'...", this->_name.c_str());
      bool result = (*this->_taskFunction)(this);
      if (result) {
        this->_nextRunTime = currentMilliseconds + this->_interval;
      } else {
        // TODO: else?
        Log::logWarning("Task '%s' returned false so is stopped.", this->_name.c_str());
      }
    }
  }
}

Tasks::Tasks(): Component("Tasks")
{}

// Add a task. return the task index
int Tasks::add(String name, Milliseconds interval, bool (*taskFunction)(Task *task))
{
  Task *newTask = new Task(name, interval, taskFunction);
  this->tasks.push_back(newTask);
  // Return task index
  return this->tasks.size() - 1;
}

void Tasks::setup() {
  Log::logInformation("Setting up Tasks");
}

void Tasks::loop()
{
  Milliseconds currentMilliseconds = millis();
  for (int i = 0; i < this->tasks.size(); i++)
  {
    Task *task = tasks[i];
    task->runIfRequired(currentMilliseconds);
  }
}

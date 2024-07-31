#include "TaskComponent.h"
#include "logging.h"

// Create a new task with a name, an interval in milliseconds and a function to call
// when the task should run
Task::Task(String name, Milliseconds interval, std::function<void()> const taskFunction) :
  _name(name),
  _interval(interval),
  _nextRunTime(0), // ASAP
  _taskFunction(taskFunction)
{}

// Run the task if it should, i.e. when the interval has passed since the last time the
// task was run
void Task::runIfRequired(Milliseconds currentMilliseconds)
{
  if (currentMilliseconds >= this->_nextRunTime)
  {
    if (this->_taskFunction != NULL)
    {
      Log::logTrace("[%s] Running...", this->_name.c_str());
      this->_taskFunction();
      this->_nextRunTime = currentMilliseconds + this->_interval;
    }
  }
}

// Tasks constructor
Tasks::Tasks() : Component("Tasks")
{}

// Add a task. Return the tasks index
int Tasks::add(String name, Milliseconds interval, std::function<void()> const taskFunction)
{
  // Create a task with the parameters
  Task *newTask = new Task(name, interval, taskFunction);
  // Store it in the task list
  this->tasks.push_back(newTask);
  // Return task index
  return this->tasks.size() - 1;
}

// The setup() function is required by the Component base class
// but it doesn't actually do anything
void Tasks::setup()
{
  Log::logTrace("[%s] Setting up Tasks", name());
}

// The loop() function from Component. Asks each task to run if it should
void Tasks::loop()
{
  Milliseconds currentMilliseconds = millis();
  for (auto task: tasks)
    task->runIfRequired(currentMilliseconds);
}

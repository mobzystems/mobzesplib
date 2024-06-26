#include "TimeComponent.h"

TimeComponent::TimeComponent(const char *timezoneName, uint16_t syncTimeout) :
	Component("Time"),
	_timezoneName(timezoneName),
  _syncTimeout(syncTimeout)
{}

Timezone *TimeComponent::TZ()
{
	return &this->_TZ;
}

// Set up the internal Timezone object
void TimeComponent::setup()
{
  setStatus(100, Log::LOGLEVEL::Information, "Starting");
  this->_TZ.setLocation(this->_timezoneName);
  this->_TZ.setDefault();
	Log::logDebug("[%s] Waiting for synchronization (%d s)...", name(), this->_syncTimeout);
  // Wait for time synchronization. If this fails, report it but continue
  if (!waitForSync(this->_syncTimeout)) {
    setStatus(900, Log::LOGLEVEL::Information, "Failed to synchronize");
    Log::logTrace("[%s] Failed to synchronize time", name());
  } else {
    setStatus(300, Log::LOGLEVEL::Information, "Initialized");
    Log::logDebug("[%s] Time in time zone '%s' is '%s'", name(), this->_timezoneName.c_str(), this->_TZ.dateTime().c_str());
  }

  // Notify the logging system we have time
  Log::setTimezone(&this->_TZ);
}

// Call eztime's events()
void TimeComponent::loop()
{
  events();
}

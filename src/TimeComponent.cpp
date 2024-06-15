#include "TimeComponent.h"

TimeComponent::TimeComponent(const char *timezoneName) :
	Component("Time"),
	_timezoneName(timezoneName)
{}

Timezone *TimeComponent::TZ()
{
	return &this->_TZ;
}

// Set up the internal Timezone object
void TimeComponent::setup()
{
  this->_TZ.setLocation(this->_timezoneName);
  this->_TZ.setDefault();
  delay(250);
	Log::logTrace("[TimeComponent] Waiting for synchronization...");
  waitForSync(0);

	// Pass the time zone component to Log
	Log::setTimezone(&this->_TZ);

	Log::logTrace("[TimeComponent] Time in time zone '%s' is '%s'", this->_timezoneName.c_str(), this->_TZ.dateTime().c_str());
}

// Call eztime's events()
void TimeComponent::loop()
{
  events();
}

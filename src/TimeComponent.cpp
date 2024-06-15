#include "TimeComponent.h"

TimeComponent::TimeComponent(const char *timezoneName) :
	Component("Time"),
	_timezoneName(timezoneName)
{}

Timezone *TimeComponent::TZ() {
	return &this->_TZ;
}

void TimeComponent::setup() {
  this->_TZ.setLocation(this->_timezoneName);
  this->_TZ.setDefault();
  delay(250);
  waitForSync(0);

	Log::setTimezone(&this->_TZ);

	Log::logTrace("[TimeComponent] Time in time zone '%s' is '%s'", this->_timezoneName.c_str(), this->_TZ.dateTime().c_str());
}

void TimeComponent::loop() {
  events();
}

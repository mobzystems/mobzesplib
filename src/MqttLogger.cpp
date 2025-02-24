#include "MqttLogger.h"

// #define LOG

/*
 * MqttLogger class. Saves logged messages in a backlog buffer and publishes
 * them as soon as loop() is called and the Mqtt client is connected.
 */
MqttLogger::MqttLogger(MqttComponent *mqtt, const char *topic, int max_size, Log::LOGLEVEL minLevel) : 
  Logger("MqttLogger", minLevel, [this](const char *message) {
    // Save the message in the backlog UNLESS WE'RE ALREADY SENDING
    if (!_blocked) {
      // Capture the log message
#ifdef LOG
      Serial.println(String(">>> MQTT: ") + message);
#endif
      // Push message onto backlog, including timestamp
      _backlog.push(Log::getTimeStamp() + message);

      // Remove excess entries from backlog
      while ((int)_backlog.size() > _max_size)
        _backlog.pop();
    }
  }),
  _mqtt(mqtt),
  _topic(String(topic)),
  _max_size(max_size)
{}

/*
* Send the back-log to the Mqtt broker
*/
void MqttLogger::loop() {
  // If there is a backlog, send it here:
  if (!_backlog.empty()) {
    // Are we connected?
    if (_mqtt->mqttClient()->connected()) {
      // Block re-entry
      _blocked = true;
#ifdef LOG
      Serial.println("--- Start of backlog");
#endif
      while (!_backlog.empty()) {
        auto msg = _backlog.front();
#ifdef LOG
        Serial.println(msg);
#endif
        _mqtt->mqttClient()->publish(_topic.c_str(), msg.c_str());
        _backlog.pop();
      }
#ifdef LOG
      Serial.println("--- End of backlog");
#endif
      // Allow re-entry
      _blocked = false;
    }
  }
}

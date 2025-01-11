#include "MqttLogger.h"

/*
 * MqttLogger class. Saves logged messages in a backlog buffer and publishes
 * them as soon as loop() is called and the Mqtt client is connected.
 */
MqttLogger::MqttLogger(MqttComponent *mqtt, const char *topic, int max_size, Log::LOGLEVEL minLevel) : 
  Logger("MqttLogger", minLevel, [this](const char *message) {
    // Save the message in the backlog UNLESS WE'RE ALREADY SENDING
    if (!_blocked) {
    // Capture the log message
    Serial.println(String(">>> MQTT: ") + message);
    _backlog.push(String(message));

    while (!_backlog.size() > _max_size)
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
      Serial.println("--- Start of backlog");
      while (!_backlog.empty()) {
        auto msg = _backlog.front();
        Serial.println(msg);
        _mqtt->mqttClient()->publish(_topic.c_str(), msg.c_str());
        _backlog.pop();
      }
      Serial.println("--- End of backlog");
      // Allow re-entry
      _blocked = false;
    }
  }
}

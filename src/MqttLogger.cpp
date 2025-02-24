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
      _backlog.push(message);

      // Remove excess entries from backlog
      while ((int)_backlog.size() > _max_size)
        _backlog.pop();
    }
  }),
  _mqtt(mqtt),
  _topic(String(topic)),
  _max_size(max_size)
{}

/**
 * Send the back-log to the Mqtt broker
 * NOT ANYMORE: If we have a continuous connection, send the message without a timestamp. The timestamp will be added by the logger.
 * If we had an MQTT connection interruption, DO send thecurrent timestamp before the message. The log message
 * already contains its own time stamp, so it will eventually look like "<publish-TS> [<Log-TS>] Message", i.e. the time stamp
 * of the PUBLISHING of the message is included between square brackets
 */
void MqttLogger::loop() {
  // static bool missedBacklog = false;

  // If there is a backlog, send it here:
  if (!_backlog.empty()) {
    // Are we connected?
    if (_mqtt->mqttClient()->connected()) {
      // Block re-entry
      _blocked = true;
#ifdef LOG
      Serial.println("--- Start of backlog");
#endif
      // String timeStamp = Log::getTimeStamp();
      // timeStamp.trim();
      // timeStamp = String("[") + timeStamp + String("] ");

      while (!_backlog.empty()) {
        auto message = _backlog.front();
#ifdef LOG
        Serial.println(msg);
#endif
        // // Log with extra time stamp if we have missed backlog entries
        // if (missedBacklog) {
        //   _mqtt->mqttClient()->publish(_topic.c_str(), (timeStamp + message).c_str());
        // } else {
          _mqtt->mqttClient()->publish(_topic.c_str(), message.c_str());
        // }
        _backlog.pop();
      }
#ifdef LOG
      Serial.println("--- End of backlog");
#endif
      // Allow re-entry
      _blocked = false;
      // // Solved missing backlog
      // missedBacklog = false;
    }
    // else {
    //   // If not connected, signal we have a missed backlog. This will cause timestamps to be added the next time we
    //   // have an MQTT connection
    //   missedBacklog = true;
    // }
  }
}

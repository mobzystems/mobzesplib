#include <Arduino.h>
#include "Logging.h"
#include "MqttComponent.h"

#include <queue>

/*
 * MqttLogger class. Saves logged messages in a backlog buffer and publishes
 * them as soon as loop() is called and the Mqtt client is connected.
 */
class MqttLogger: public Logger {
private:
  MqttComponent *_mqtt;
  String _topic;
  int _max_size;
  
  std::queue<String> _backlog = std::queue<String>();
  bool _blocked = false;

public:
  MqttLogger(MqttComponent *mqtt, const char *topic, int max_size, Log::LOGLEVEL minLevel = Log::LOGLEVEL::None);
  void loop();
};
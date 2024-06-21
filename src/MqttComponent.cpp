#include "MqttComponent.h"
#include "logging.h"

/***
 * Create a new MqttComponent
 */
MqttComponent::MqttComponent(
  Client *client, 
  const char *broker, 
  uint16_t portNumber, 
  const char *username, 
  const char *password, 
  void (*subscribe)(PubSubClient *), 
  void (*receive)(char *topic, byte *payload, unsigned int length),
  unsigned long intervalMs
) :
  Component("Mqtt"),
  _username(username),
  _password(password),
  _mqttClient(broker, portNumber, receive, *client),
  _subscribe(subscribe),
  _intervalMs(intervalMs)
{}

/***
 * (re)Connect to the MQTT broker
 */
void MqttComponent::reconnect()
{
  // Try to connect
  if (!this->_mqttClient.connected())
  {
    Log::logDebug("[%s] Attempting MQTT connection...", name());
    // Create a random client ID
    String clientId = "MqttComponent-" + String(random(0xffff), HEX);

    // Attempt to connect
    // Log::logTrace("MQTT %s/%s", mqttUsername.c_str(), mqttPassword.c_str());
    if (this->_mqttClient.connect(clientId.c_str(), this->_username.c_str(), this->_password.c_str()))
    {
      Log::logDebug("[%s] MQTT with id '%s' connected.", name(), clientId.c_str());
      if (this->_subscribe != NULL) {
        Log::logTrace("[%s] Subscribing...", name());
        (*(this->_subscribe))(&this->_mqttClient);
      }
    }
    else
    {
      Log::logError("[%s] MQTT connection failed, state = %d", name(), this->_mqttClient.state());
      // Log::logTrace("[%s] Waiting 5 seconds", name());
      // // Wait 5 seconds before retrying
      // delay(5000); // TODO
    }
  }
}

// setup() the component: connect
void MqttComponent::setup()
{
  this->reconnect();
  this->_lastCheckTime = millis();
}

// loop() for Mqtt
void MqttComponent::loop()
{
  if (millis() > this->_lastCheckTime + this->_intervalMs)
  {
    Log::logDebug("[%s] Checking connection...", name());
    if (!this->_mqttClient.connected()) {
      Log::logWarning("[%s] Connection lost, reconnecting...", name());
      this->reconnect();
    } else
      Log::logDebug("[%s] Connected.", name());

    this->_lastCheckTime = millis();
  }

  this->_mqttClient.loop();
}
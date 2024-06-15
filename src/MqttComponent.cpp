#include "MqttComponent.h"
#include "logging.h"

/***
 * Create a new MqttComponent
 */
MqttComponent::MqttComponent(
  Client &client, 
  const char *broker, 
  uint16_t portNumber, 
  const char *username, 
  const char *password, 
  void (*subscribe)(PubSubClient *) =  NULL, 
  void (*receive)(char *topic, byte *payload, unsigned int length) = NULL
) :
  Component("Mqtt Component"),
  _username(username),
  _password(password),
  _subscribe(subscribe),
  _mqttClient(broker, portNumber, receive, client)
{}

/***
 * (re)Connect to the MQTT broker
 */
void MqttComponent::reconnect()
{
  // Loop until we're reconnected
  while (!this->_mqttClient.connected())
  {
    Log::logDebug("[MqttComponent] Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "MqttComponent-" + String(random(0xffff), HEX);

    // Attempt to connect
    // Log::logTrace("MQTT %s/%s", mqttUsername.c_str(), mqttPassword.c_str());
    if (this->_mqttClient.connect(clientId.c_str(), this->_username.c_str(), this->_password.c_str()))
    {
      Log::logDebug("[MqttComponent] MQTT with id '%s' connected.", clientId.c_str());
      if (this->_subscribe != NULL) {
        Log::logTrace("[MqttComponent] Subscribing...");
        (*(this->_subscribe))(&this->_mqttClient);
      }
    }
    else
    {
      Log::logError("[MqttComponent] MQTT connection failed, state = %d", this->_mqttClient.state());
      Log::logTrace("[MqttComponent] Waiting 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000); // TODO
    }
  }
}

// setup() the component: connect
void MqttComponent::setup()
{
  this->reconnect();
}

// loop() for Mqtt
void MqttComponent::loop()
{
  if (!this->_mqttClient.connected())
    this->reconnect();

  this->_mqttClient.loop();
}
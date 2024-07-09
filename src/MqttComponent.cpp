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
  const char *clientId,
  void (*subscribe)(PubSubClient *), 
  void (*receive)(const char *topic, const byte *payload, unsigned int length),
  unsigned long intervalMs,
  const char *willTopic, 
  const char *willMessage,
  bool willRetain, 
  uint8_t willQos
) :
  Component("Mqtt"),
  _username(username),
  _password(password),
  _clientId(clientId),
  _mqttClient(broker, portNumber, receive, *client),
  _subscribe(subscribe),
  _intervalMs(intervalMs),
  _willTopic(willTopic),
  _willMessage(willMessage),
  _willRetain(willRetain),
  _willQos(willQos)
{}

/***
 * (re)Connect to the MQTT broker
 */
void MqttComponent::reconnect()
{
  // Try to connect
  if (!this->_mqttClient.connected())
  {
    // Use the provided client ID, replacing # with a random four-digit hex number
    // String clientId = "MqttComponent-" + String(random(0xffff), HEX);
    String clientId = String(_clientId);
    while (clientId.indexOf('#') >= 0) {
      clientId.replace("#", String(random(0x10000 - 0x1000) + 0x1000, HEX));
    }

    Log::logDebug("[%s] Attempting MQTT connection with client ID '%s' (state is %d)...", this->name(), clientId.c_str(), this->mqttClient()->state());
    // Attempt to connect
    // Log::logTrace("MQTT %s/%s", mqttUsername.c_str(), mqttPassword.c_str());
    if (this->_mqttClient.connect(
      clientId.c_str(),
      this->_username.c_str(), this->_password.c_str(),
      this->_willTopic.c_str(), this->_willQos, this->_willRetain, this->_willMessage.c_str()
    ))
    {
      Log::logInformation("[%s] MQTT connected with client ID '%s'.", this->name(), clientId.c_str());
      if (this->_subscribe != NULL) {
        Log::logTrace("[%s] Subscribing...", name());
        (*(this->_subscribe))(&this->_mqttClient);
      }
    }
    else
    {
      Log::logError("[%s] MQTT connection failed, state = %d", this->name(), this->_mqttClient.state());
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
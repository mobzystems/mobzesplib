#include <Arduino.h>
#include <PubSubClient.h>
#include "Specific_ESP_Wifi.h"

#include "components.h"

class MqttComponent: public Component {
  private:
    String _username;
    String _password;
    PubSubClient _mqttClient;
    void (*_subscribe)(PubSubClient *);
    void (*_receive)(char *topic, byte *payload, unsigned int length);

    void reconnect();

  public:
    MqttComponent(Client &client, const char *broker, uint16_t portNumber, const char *username, const char *password, void (*subscribe)(PubSubClient *), void (*receive)(char *topic, byte *payload, unsigned int length));
    PubSubClient *mqttClient() { return &this->_mqttClient; }

    void setup();
    void loop();
};

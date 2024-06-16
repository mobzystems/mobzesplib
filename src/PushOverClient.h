#ifndef __PUSHOVERCLIENT_H__
#define __PUSHOVERCLIENT_H__

#include <Arduino.h>
#include <WiFiClient.h>

class PushOverClient {
    private:
      WiFiClient &_client;
      String _api_key;

    public:
      PushOverClient(WiFiClient &client, const char *api_key);
      void SendNotification(const char *to, const char *title, const char *message);
};

#endif
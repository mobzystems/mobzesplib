#include "PushOverClient.h"

#include <ESP_HttpClient.h>

#include "logging.h"

PushOverClient::PushOverClient(WiFiClient &client, const char *api_key) :
  _client(client),
  _api_key(api_key)
{}

String json_encode(const char *s)
{
  String t = s;
  t.replace("\\", "\\\\");
  t.replace("\"", "\\\"");
  t.replace("\r", "\\r");
  t.replace("\n", "\\n");

  return t;
}

void PushOverClient::SendNotification(const char *to, const char *title, const char *message)
{
  Log::logInformation("[PushOverClient] Sending notification to '%s'", to);

  HTTPClient http;
    
  // The URL cannot be https because then things get complicated fast :-/
  // This URL works!
  http.begin(this->_client, "http://api.pushover.net/1/messages.json");
  
  // If you need Node-RED/server authentication, insert user and password below
  //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");
  
  // Form encoded version
  // http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  // // Data to send with HTTP POST
  // String httpRequestData = String("token=") + this->_api_key + "&user=" + to + "&title=" + title + "&message=" + message;

  // JSON version
  http.addHeader("Content-Type", "application/json");
  String httpRequestData = String("{\"token\": \"") + this->_api_key + "\", \"user\": \"" + to + "\", \"title\": \"" + json_encode(title) + "\", \"message\": \"" + json_encode(message) + "\"}";

  // Send HTTP POST request
  Log::logInformation("[PushOverClient] Sending data '%s'", httpRequestData.c_str());
  int httpResponseCode = http.POST(httpRequestData);

  Log::logDebug("[PushOverClient] HTTP Response code: %d", httpResponseCode);
    
  // Free resources
  http.end();
}
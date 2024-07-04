#if defined(ESP8266)
  #include <ESP8266WebServer.h>
  #define WEBSERVER ESP8266WebServer 
#elif defined(ESP32)
  #include <WebServer.h>
  #define WEBSERVER WebServer 
#endif
#include "SignalLedTask.h"

/**
 * Use a LED to signal a numeric value.
 * 
 * app: The application to use
 * ledPin: the GPIO number of the LED
 * cycleLength: the length of a single cycle
 * maxValue: number of cycles
 */
void addSignalLedTask(MqttApplication *app, Milliseconds cycleLength, uint maxValue)
{
  int ledPin = atoi(app->config("signal-led-pin", "-1"));
  if (ledPin < 0) {
    Log::logInformation("[SignalLed] No pin configured");
    return;
  }

  uint32_t rgb = strtol(app->config("signal-led-rgb", "0"), NULL, 16);
  
  bool isInverted = atoi(app->config("signal-led-inverted", "0")) != 0;

  if (rgb) {
#ifdef ESP8266
    Log::logError("[SignalLed] RGB LED not supported on ESP8266");
    return;
#else
    Log::logInformation("[SignalLed] Adding RGB signal LED on pin %d with color %06X", ledPin, rgb);
#endif
  } else
    Log::logInformation("[SignalLed] Adding signal LED on pin %d", ledPin);
  
  if (rgb == 0)
    pinMode((uint8_t)ledPin, OUTPUT);

  // _app->addTask("task", 500, []() {
  //   static int count = 0;
  //   bool on;

  //   count++;
  //   if (!WiFi.isConnected()) {
  //     // No Wifi? Then every second
  //     on = count % 2 < 1;
  //   } else if (!_app->mqtt()->mqttClient()->connected()) {
  //     // WiFi, but no MQTT? Then every 2 seconds
  //     on = count % 4 < 2;
  //   } else {
  //     // WiFi and MQTT: every 6 (3 seconds)
  //     on = count % 6 < 3;
  //   }
  //   digitalWrite(33, on ? HIGH : LOW);
  // });

  // Set up a task with half of the cycle length. The first half of the cycle
  // is signal, the other half is "off". Together they form a "pulse"
  app->addTask("task", cycleLength / 2, [ledPin, app, maxValue, rgb, isInverted]() {
    // Max. number of pulses (2 x cycle)
    static uint max = maxValue;
    // "Cycle count". 0..2*max-1
    static uint count = 0;

    bool on;
    
    if (count % 2 == 1) {
      // Odd counts are always off
      on  = false;
    } else {
      // Count the evens: 0..max-1
      int n = count / 2;

      if (!WiFi.isConnected()) {
        // No Wifi? 3 cycles ON
        on = n < 3;
      } else if (!app->mqtt()->mqttClient()->connected()) {
        // WiFi, but no MQTT? 2 cycles ON
        on = n < 2;
      } else {
        // WiFi and MQTT: one cycle ON
        on = n < 1;
      }
    }

    // If inverted, "on" maps to LOW instead of HIGH
    if (isInverted)
      on = !on;

    if (rgb == 0)
      digitalWrite((uint8_t)ledPin, on ? HIGH : LOW);
#ifdef ESP8266
#else
    else if (!on) {
      // Log::logInformation("Neopixel off");
      neopixelWrite((uint8_t)ledPin, 0, 0, 0);
    } else {
      // #ff0000 is red, #00ff00 is green, #0000ff is blue
      // Log::logInformation("Neopixel %d %d %d", (rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
      neopixelWrite((uint8_t)ledPin, (rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
    }
#endif

    // Cycle the pulse
    if (count++ > 2 * max) {
      count = 0;
    }
  });
}
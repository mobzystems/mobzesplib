#include "SignalLedTask.h"

/**
 * Use a LED to signal a numeric value.
 * 
 * app: The application to use
 * ledPin: the GPIO number of the LED
 * cycleLength: the length of a single cycle
 * maxValue: number of cycles
 */
void addSignalLedTask(MqttApplication *app, uint8_t ledPin, Milliseconds cycleLength, uint maxValue)
{
  pinMode(ledPin, OUTPUT);
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
  app->addTask("task", cycleLength / 2, [ledPin, app, maxValue]() {
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

    digitalWrite(ledPin, on ? LOW : HIGH);

    // Cycle the pulse
    if (count++ > 2 * max) {
      count = 0;
    }
  });
}
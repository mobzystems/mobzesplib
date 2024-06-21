#ifndef __BLUETOOTHSERIALCOMPONENT_H__
#define __BLUETOOTHSERIALCOMPONENT_H__

#include <Arduino.h>
#include "components.h"
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

class BluetoothSerialComponent: public Component {
  private:
    const char *_hostname;
    void (*_onDataAvailable)(int byte);
    // Bluetooth (Classic, not LE)
    BluetoothSerial _serialBT;

  public:
    BluetoothSerialComponent(const char *hostname, void (*onDataAvailable)(int));

    BluetoothSerial *serialBT(void) { return &this->_serialBT; }
    const char *hostname() { return this->_hostname; }

    void setup();
    void loop();
};

#endif
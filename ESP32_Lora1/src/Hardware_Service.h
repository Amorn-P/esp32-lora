/**
 * Hardware_Service.h - GPIO, Relay, Dipswitch Management
 */

#ifndef HARDWARE_SERVICE_H
#define HARDWARE_SERVICE_H

#include <Arduino.h>
#include "Hardware_Map.h"

class HardwareService {
public:
    static void init();
    static uint8_t getBoardID();
    static void setRelay(uint8_t index, bool state);
    static void allRelaysOff();
    static uint8_t getRelayStatus();
};

#endif

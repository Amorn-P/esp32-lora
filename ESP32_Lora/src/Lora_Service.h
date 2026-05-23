/**
 * Lora_Service.h - LoRa Radio Engine (SX1278 via RadioLib)
 */

#ifndef LORA_SERVICE_H
#define LORA_SERVICE_H

#include <Arduino.h>
#include <RadioLib.h>
#include "Lora_Protocol.h"
#include "Hardware_Map.h"
#include "User_config.h"

class LoraService {
public:
    static bool init();
    static bool sendPacket(LuckyPacket &packet);
    static bool receivePacket(LuckyPacket &packet);
    static void startReceive();
    static uint16_t calculateChecksum(LuckyPacket *p);
    static bool verifyChecksum(LuckyPacket *p);
private:
    static SX1278 radio;
};

#endif

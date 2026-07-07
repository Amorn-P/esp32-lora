/**
 * Lora_Service.cpp - LoRa Radio Engine Implementation
 */

#include "Lora_Service.h"

SX1278 LoraService::radio = new Module(SS_PIN, DIO0_PIN, RST_PIN);

bool LoraService::init() {
    Serial.print(F("[LoRa] Init SX1278 ... "));
    int state = radio.begin(LORA_FREQ, LORA_BW, LORA_SF, LORA_CR,
                            LORA_SYNC_WORD, LORA_TX_POWER);
    if (state == RADIOLIB_ERR_NONE) {
        Serial.println(F("OK"));
        radio.startReceive();
        return true;
    }
    Serial.printf("FAIL (code %d)\n", state);
    return false;
}

bool LoraService::sendPacket(LuckyPacket &packet) {
    packet.checksum = calculateChecksum(&packet);
    int state = radio.transmit((uint8_t*)&packet, sizeof(LuckyPacket));
    radio.startReceive();
    return (state == RADIOLIB_ERR_NONE);
}

bool LoraService::receivePacket(LuckyPacket &packet) {
    if (!(radio.getIRQFlags() & RADIOLIB_SX127X_CLEAR_IRQ_FLAG_RX_DONE)) {
        return false;
    }
    int state = radio.readData((uint8_t*)&packet, sizeof(LuckyPacket));
    radio.startReceive();
    if (state != RADIOLIB_ERR_NONE) return false;
    return verifyChecksum(&packet);
}

void LoraService::startReceive() {
    radio.startReceive();
}

uint16_t LoraService::calculateChecksum(LuckyPacket *p) {
    uint16_t sum = 0;
    uint8_t *ptr = (uint8_t*)p;
    for (size_t i = 0; i < sizeof(LuckyPacket) - 2; i++) {
        sum += ptr[i];
    }
    return sum;
}

bool LoraService::verifyChecksum(LuckyPacket *p) {
    uint16_t received = p->checksum;
    p->checksum = 0;
    uint16_t calculated = calculateChecksum(p);
    p->checksum = received;
    return (received == calculated);
}

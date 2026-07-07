/**
 * Hardware_Service.cpp - GPIO, Relay, Dipswitch Implementation
 */

#include "Hardware_Service.h"
#include <Wire.h>

// RELAY_PINS definition (moved from .h to avoid include-order issues)
const uint8_t RELAY_PINS[TOTAL_RELAYS] = {
    RELAY_1, RELAY_2, RELAY_3, RELAY_4, RELAY_5, RELAY_6, RELAY_7
};

void HardwareService::init() {
    pinMode(DIP_BIT0, INPUT_PULLUP);
    pinMode(DIP_BIT1, INPUT_PULLUP);
    pinMode(DIP_BIT2, INPUT_PULLUP);
    pinMode(DIP_BIT3, INPUT_PULLUP);

    for (int i = 0; i < TOTAL_RELAYS; i++) {
        pinMode(RELAY_PINS[i], OUTPUT);
        digitalWrite(RELAY_PINS[i], (RELAY_ON == HIGH) ? LOW : HIGH);
    }

    Wire.begin(SDA_PIN, SCL_PIN);
}

uint8_t HardwareService::getBoardID() {
    uint8_t id = 0;
    if (digitalRead(DIP_BIT0) == LOW) id |= (1 << 0);
    if (digitalRead(DIP_BIT1) == LOW) id |= (1 << 1);
    if (digitalRead(DIP_BIT2) == LOW) id |= (1 << 2);
    if (digitalRead(DIP_BIT3) == LOW) id |= (1 << 3);
    return id;
}

void HardwareService::setRelay(uint8_t index, bool state) {
    if (index < TOTAL_RELAYS) {
        digitalWrite(RELAY_PINS[index], state ? RELAY_ON : (RELAY_ON == HIGH ? LOW : HIGH));
    }
}

void HardwareService::allRelaysOff() {
    for (int i = 0; i < TOTAL_RELAYS; i++) {
        digitalWrite(RELAY_PINS[i], (RELAY_ON == HIGH) ? LOW : HIGH);
    }
}

uint8_t HardwareService::getRelayStatus() {
    uint8_t status = 0;
    for (int i = 0; i < TOTAL_RELAYS; i++) {
        if (digitalRead(RELAY_PINS[i]) == HIGH) status |= (1 << i);
    }
    return status;
}

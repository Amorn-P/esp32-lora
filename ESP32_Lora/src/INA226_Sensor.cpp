/**
 * INA226_Sensor.cpp - Implementation
 * Only compiled for slave (B2/B3 pump boards with solar sensor)
 */

#if BOARD_TYPE != 0

#include "INA226_Sensor.h"
#include <Wire.h>

INA226          INA226Sensor::ina(INA226_ADDR, &Wire);
bool            INA226Sensor::solarOK = true;
uint8_t         INA226Sensor::highCount = 0;
uint8_t         INA226Sensor::lowCount = 0;
float           INA226Sensor::current_mA = 0;
unsigned long   INA226Sensor::lastRead = 0;

bool INA226Sensor::init() {
    if (!ina.begin()) {
        Serial.println("[INA226] Init FAILED");
        solarOK = true;
        return false;
    }
    ina.setMaxCurrentShunt(2.0, 0.1);
    Serial.println("[INA226] Init OK");
    return true;
}

void INA226Sensor::update() {
    if (millis() - lastRead < 5000) return;
    lastRead = millis();

    current_mA = ina.getCurrent() * 1000.0;

    if (current_mA >= SOLAR_THRESHOLD_MA) {
        highCount++;
        lowCount = 0;
    } else {
        lowCount++;
        highCount = 0;
    }

    if (!solarOK && highCount >= SOLAR_DEBOUNCE_COUNT) {
        solarOK = true;
        Serial.printf("[INA226] Solar OK (%.1f mA)\n", current_mA);
    }

    if (solarOK && lowCount >= SOLAR_DEBOUNCE_COUNT) {
        solarOK = false;
        Serial.printf("[INA226] Solar LOW (%.1f mA) — pausing pump\n", current_mA);
    }
}

bool INA226Sensor::isSolarOK() {
    return solarOK;
}

float INA226Sensor::getCurrent_mA() {
    return current_mA;
}

#endif // BOARD_TYPE != 0

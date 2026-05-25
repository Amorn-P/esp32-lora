/**
 * INA226_Sensor.cpp - Implementation
 * 
 * v2.0: Vbus-based sun detection + DS18B20 enclosure temp + fan control
 * Only compiled for slave (B2/B3 pump boards with solar sensor)
 */

#if BOARD_TYPE != 0

#include "INA226_Sensor.h"
#include <Wire.h>

INA226          INA226Sensor::ina(INA226_ADDR, &Wire);
OneWire         INA226Sensor::oneWire(DS18B20_PIN);
DallasTemperature INA226Sensor::ds18b20(&oneWire);
bool            INA226Sensor::ds18b20OK = false;

bool            INA226Sensor::solarOK = true;
uint8_t         INA226Sensor::highCount = 0;
uint8_t         INA226Sensor::lowCount = 0;
float           INA226Sensor::current_mA = 0;
float           INA226Sensor::busVoltage_V = 0;
float           INA226Sensor::temperature_C = 0;
unsigned long   INA226Sensor::lastRead = 0;
bool            INA226Sensor::fanOn = false;

// ============================================================
// INIT
// ============================================================
bool INA226Sensor::init() {
    // 1. INA226
    if (!ina.begin()) {
        Serial.println("[INA226] Init FAILED — solarOK=true (safe mode)");
        solarOK = true;
    } else {
        ina.setMaxCurrentShunt(2.0, 0.1);
        Serial.println("[INA226] Init OK");
    }

    // 2. DS18B20
    ds18b20.begin();
    uint8_t nDevices = ds18b20.getDeviceCount();
    if (nDevices == 0) {
        Serial.println("[DS18B20] No sensor found — fan disabled");
        ds18b20OK = false;
    } else {
        ds18b20.setResolution(10); // 0.25°C, 188ms conversion
        ds18b20OK = true;
        Serial.printf("[DS18B20] Found %d device(s), res=10-bit\n", nDevices);
    }

    // 3. Fan GPIO
    pinMode(FAN_PIN, OUTPUT);
    digitalWrite(FAN_PIN, LOW);   // Fan OFF at boot

    return true;
}

// ============================================================
// UPDATE — called every 5 seconds from Scheduler::run()
// ============================================================
void INA226Sensor::update() {
    if (millis() - lastRead < 5000) return;
    lastRead = millis();

    // ---- Read INA226 ----
    busVoltage_V = ina.getBusVoltage();
    current_mA   = ina.getCurrent() * 1000.0;

    // ---- Sun detection: Vbus >= 12.0V (with 1V hysteresis) ----
    bool sunNow;
    if (solarOK) {
        sunNow = (busVoltage_V >= (V_SUN_THRESHOLD - V_SUN_HYSTERESIS)); // stay on down to 11V
    } else {
        sunNow = (busVoltage_V >= V_SUN_THRESHOLD); // need 12V to turn on
    }

    if (sunNow) {
        highCount++;
        lowCount = 0;
    } else {
        lowCount++;
        highCount = 0;
    }

    if (!solarOK && highCount >= SOLAR_DEBOUNCE_COUNT) {
        solarOK = true;
        Serial.printf("[Solar] SUN (Vbus=%.1fV, I=%.0fmA)\n", busVoltage_V, current_mA);
    }

    if (solarOK && lowCount >= SOLAR_DEBOUNCE_COUNT) {
        solarOK = false;
        Serial.printf("[Solar] NIGHT (Vbus=%.1fV) — pausing pump\n", busVoltage_V);
    }

    // ---- Read DS18B20 (if present) ----
    if (ds18b20OK) {
        ds18b20.requestTemperatures();
        temperature_C = ds18b20.getTempCByIndex(0);
        // DS18B20 returns -127 on read error — clamp to last valid
        if (temperature_C < -50.0f) return;
    }

    // ---- Fan control ----
    updateFan();
}

// ============================================================
// FAN CONTROL — solar gated, temperature threshold with hysteresis
// ============================================================
void INA226Sensor::updateFan() {
    if (!solarOK) {
        // No sun — fan OFF regardless of temp (don't drain battery)
        if (fanOn) {
            digitalWrite(FAN_PIN, LOW);
            fanOn = false;
            Serial.println("[Fan] OFF (no sun)");
        }
        return;
    }

    if (!ds18b20OK) {
        // No temp sensor — keep fan off (fail safe)
        return;
    }

    // Hysteresis: ON at 40°C, OFF at 38°C
    if (!fanOn && temperature_C >= FAN_TEMP_ON) {
        digitalWrite(FAN_PIN, HIGH);
        fanOn = true;
        Serial.printf("[Fan] ON (%.1f°C)\n", temperature_C);
    } else if (fanOn && temperature_C <= FAN_TEMP_OFF) {
        digitalWrite(FAN_PIN, LOW);
        fanOn = false;
        Serial.printf("[Fan] OFF (%.1f°C)\n", temperature_C);
    }
}

// ============================================================
// ACCESSORS
// ============================================================
bool INA226Sensor::isSolarOK() {
    return solarOK;
}

bool INA226Sensor::isFanOn() {
    return fanOn;
}

float INA226Sensor::getCurrent_mA() {
    return current_mA;
}

float INA226Sensor::getVoltage_V() {
    return busVoltage_V;
}

float INA226Sensor::getTemperature_C() {
    return temperature_C;
}

#endif // BOARD_TYPE != 0

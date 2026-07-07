/**
 * INA226_Sensor.h - Solar controller for B2/B3 pump boards
 * 
 * v2.0: Vbus-based sun detection (not current), DS18B20 temp, fan control
 * Only available on slave (BOARD_TYPE != 0)
 */

#ifndef INA226_SENSOR_H
#define INA226_SENSOR_H

#include <Arduino.h>

#if BOARD_TYPE != 0

#include <INA226.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Hardware_Map.h"

#define INA226_ADDR           0x40

// Sun detection thresholds
#define V_SUN_THRESHOLD       12.0   // Vbus >= 12V = sun available
#define V_SUN_HYSTERESIS      1.0    // Drop below 11V to declare night
#define SOLAR_DEBOUNCE_COUNT  3      // Readings to confirm state change

// Fan control thresholds
#define FAN_TEMP_ON           40.0   // °C — turn fan ON
#define FAN_TEMP_OFF          38.0   // °C — turn fan OFF (2°C hysteresis)

class INA226Sensor {
public:
    static bool init();
    static void update();
    static bool isSolarOK();
    static bool isFanOn();
    static float getCurrent_mA();
    static float getVoltage_V();
    static float getTemperature_C();

private:
    static INA226 ina;
    static OneWire oneWire;
    static DallasTemperature ds18b20;
    static bool ds18b20OK;

    static bool solarOK;
    static uint8_t highCount;
    static uint8_t lowCount;
    static float current_mA;
    static float busVoltage_V;
    static float temperature_C;
    static unsigned long lastRead;

    static bool fanOn;
    static void updateFan();
};

#else
// Master stub — no INA226
class INA226Sensor {
public:
    static bool init() { return false; }
    static void update() {}
    static bool isSolarOK() { return true; }
    static bool isFanOn() { return false; }
    static float getCurrent_mA() { return 0; }
    static float getVoltage_V() { return 0; }
    static float getTemperature_C() { return 0; }
};
#endif // BOARD_TYPE != 0

#endif // INA226_SENSOR_H

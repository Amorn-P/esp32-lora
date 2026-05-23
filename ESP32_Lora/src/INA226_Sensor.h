/**
 * INA226_Sensor.h - Solar current sensor for B2/B3 pump boards
 * Only available on slave (BOARD_TYPE != 0)
 */

#ifndef INA226_SENSOR_H
#define INA226_SENSOR_H

#include <Arduino.h>

#if BOARD_TYPE != 0

#include <INA226.h>

#define INA226_ADDR           0x40
#define SOLAR_THRESHOLD_MA    50
#define SOLAR_DEBOUNCE_COUNT  3

class INA226Sensor {
public:
    static bool init();
    static void update();
    static bool isSolarOK();
    static float getCurrent_mA();

private:
    static INA226 ina;
    static bool solarOK;
    static uint8_t highCount;
    static uint8_t lowCount;
    static float current_mA;
    static unsigned long lastRead;
};

#else
// Master stub — no INA226
class INA226Sensor {
public:
    static bool init() { return false; }
    static void update() {}
    static bool isSolarOK() { return true; }
    static float getCurrent_mA() { return 0; }
};
#endif // BOARD_TYPE != 0

#endif // INA226_SENSOR_H

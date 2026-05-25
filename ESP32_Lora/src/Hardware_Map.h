/**
 * Hardware_Map.h - Pin Definitions (All Boards)
 */

#ifndef HARDWARE_MAP_H
#define HARDWARE_MAP_H

#include <Arduino.h>

// ============================================================
// RELAY COUNT
// ============================================================
#define TOTAL_RELAYS             7

// ============================================================
// LORA SX1278 (SPI)
// ============================================================
#define SCK_PIN                  18
#define MISO_PIN                 19
#define MOSI_PIN                 23
#define SS_PIN                   5
#define RST_PIN                  4
#define DIO0_PIN                 2

// ============================================================
// SOLAR CONTROLLER (B2/B3 only)
// ============================================================
#define FAN_PIN                 16   // AO3400 MOSFET gate
#define DS18B20_PIN             17   // OneWire data (4.7kΩ pull-up to 3.3V)

// ============================================================
// I2C (DS3231 + LCD2004)
// ============================================================
#define SDA_PIN                  21
#define SCL_PIN                  22

// ============================================================
// DIPSWITCH 4-BIT ID
// ============================================================
#define DIP_BIT0                 36
#define DIP_BIT1                 39
#define DIP_BIT2                 34
#define DIP_BIT3                 35

// ============================================================
// RELAYS (GPIO12 skipped for boot safety)
// ============================================================
#define RELAY_1                  13
#define RELAY_2                  14
#define RELAY_3                  27
#define RELAY_4                  26
#define RELAY_5                  25
#define RELAY_6                  33
#define RELAY_7                  32

// ============================================================
// RELAY ACTIVE LEVEL
// Set to HIGH if relay triggers on HIGH, LOW if relay triggers on LOW
// Most 8-ch optocoupler modules with JD-VCC jumper ON = active LOW
// ============================================================
#define RELAY_ON                LOW

#endif // HARDWARE_MAP_H

#ifndef CONFIG_H
#define CONFIG_H

#include "User_config.h"
#include "system_state.h"
#include "freertos/semphr.h"

// --- Mutex Declaration for Shared Resources ---
extern SemaphoreHandle_t rtcMutex;
extern SemaphoreHandle_t lcdMutex;

// Motor state constants
#define MOTOR_STOP 0
#define MOTOR_FORWARD 1
#define MOTOR_REVERSE -1

// LCD Settings
#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 20
#define LCD_ROWS 4

// --- LEDC PWM Configuration for Motor Control ---
#define PWM_FREQUENCY           5000    // 5 kHz PWM
#define PWM_RESOLUTION          8       // 8-bit resolution (0-255)
#define PWM_RAMP_STEP           5       // Increment per loop iteration for smooth ramp
#define PWM_MAX_DUTY            255

// LEDC Channels
#define PAN_CHANNEL_1           0
#define PAN_CHANNEL_2           1
#define TILT_CHANNEL_1          2
#define TILT_CHANNEL_2          3

// --- Safety Task Configuration ---
#define WIND_SENSOR_SLAVE_ID    1       // Modbus slave ID of wind sensor
#define WIND_SPEED_REGISTER     0x0000  // Modbus holding register for wind speed
#define MODBUS_BAUD             9600    // RS485 baud rate
#define MODBUS_TIMEOUT_MS       200     // Modbus response timeout

// --- Astro Task Configuration ---
// Geographic location defaults (override via User_config.h if not present)
#ifndef SITE_LATITUDE
#define SITE_LATITUDE           14.0    // Default latitude (Thailand region)
#endif
#ifndef SITE_LONGITUDE
#define SITE_LONGITUDE          101.0   // Default longitude (Thailand region)
#endif

// --- UI Task Configuration ---
#define UI_UPDATE_INTERVAL_MS   1000    // LCD refresh rate

#endif

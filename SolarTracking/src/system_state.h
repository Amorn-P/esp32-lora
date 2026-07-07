#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include <Arduino.h>
#include "freertos/semphr.h"

// --- Wind Safety Constants ---
#define STOW_WIND_SPEED_KPH       40.0f   // Wind speed threshold to stow panels flat
#define DEAD_BAND_DEGREES         2.0f    // Hysteresis dead-band for motor control
#define SMOOTHING_WINDOW_SIZE     10      // Rolling average buffer size for sensor noise

// --- Global System State Struct ---
// All tasks MUST acquire stateMutex before reading or writing any field.
typedef struct {
    // --- Current Position (populated by sensor_task) ---
    float current_elevation;    // Current tilt/pitch angle in degrees (0 = flat/horizontal, 90 = vertical)
    float current_azimuth;      // Current heading/pan angle in degrees (0-360, 0 = North)

    // --- Target Position (populated by astro_task) ---
    float target_elevation;     // Computed solar elevation in degrees
    float target_azimuth;       // Computed solar azimuth in degrees

    // --- Wind Sensor Data (populated by safety_task) ---
    float wind_speed_kph;       // Latest wind speed reading in km/h
    bool  wind_sensor_fault;    // True when Modbus sensor is unplugged/timed out
    bool  wind_alarm;           // True when wind_speed_kph > STOW_WIND_SPEED_KPH

    // --- RTC Health ---
    bool  rtc_valid;             // True when DS3231 is responding and time is plausible
} SystemState;

// --- Global State Instance (defined in main.cpp) ---
extern SystemState sysState;

// --- Mutex for sysState access (defined in main.cpp) ---
extern SemaphoreHandle_t stateMutex;

#endif // SYSTEM_STATE_H

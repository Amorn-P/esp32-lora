#ifndef USER_CONFIG_H
#define USER_CONFIG_H

// ======================================================
// 1. PRIMARY CREDENTIALS (WiFi, Blynk)
// ======================================================
#define WIFI_SSID                "Gilbert_2.4G"
#define WIFI_PASSWORD            "3MRTKUHF"

/* Blynk Configuration (Legacy) */
#define BLYNK_TEMPLATE_ID        "TMPLxxxxxx"
#define BLYNK_TEMPLATE_NAME      "SolarTracker"
#define BLYNK_AUTH_TOKEN         "Your_Auth_Token"
#define BLYNK_SERVER             "43.229.135.169"
#define BLYNK_PORT               8080

// ======================================================
// 2. PIN DEFINITIONS
// ======================================================

// Motor Control (H-Bridge/Relay)
#define PAN_MOTOR_PIN_1          25
#define PAN_MOTOR_PIN_2          26
#define TILT_MOTOR_PIN_1         27
#define TILT_MOTOR_PIN_2         14

// LDR Sensor Pin Definitions (ADC1)
#define LDR_PAN_LEFT_PIN         34
#define LDR_PAN_RIGHT_PIN        35
#define LDR_TILT_UP_PIN          32
#define LDR_TILT_DOWN_PIN        33

// I2C for LCD2004
#define I2C_SDA_PIN              21
#define I2C_SCL_PIN              22

// RS485 Pins (MAX3485 module, 3.3V logic)
#define RS485_RX_PIN            16
#define RS485_TX_PIN            17
#define RS485_EN_PIN            4        // DE+RE combined, HIGH=TX LOW=RX

// RTC DS1302 Pins (DEPRECATED - Using DS3231 on I2C pins 21/22 now)
// #define RTC_RST_PIN              23
// #define RTC_DAT_PIN              18
// #define RTC_CLK_PIN              19

// ======================================================
// 3. TRACKING PARAMETERS
// ======================================================
#define PAN_THRESHOLD            10
#define TILT_THRESHOLD           10
// Default offsets (if calibration is skipped)
#define INITIAL_PAN_BIAS         0
#define INITIAL_TILT_BIAS        0

#define MOVE_BURST_MS            50
#define DAYTIME_CHECK_INTERVAL   2000
#define NIGHT_CHECK_INTERVAL     300000
#define HOMING_DURATION_MS       10000

#define DAYTIME_START_HOUR       7
#define DAYTIME_END_HOUR         20
#define CLOUD_THRESHOLD_LDR      3000

// ======================================================
// 3.5 MAGNETIC DECLINATION (compass correction per location)
// ======================================================
// Magnetic declination values for different deployment sites.
// Uncomment ONE location before deploying.
// Declination changes ~0.1° per year; update annually.

// --- Bangkok (Default) ---
#define MAGNETIC_DECLINATION  -1.0f

// --- Uthai Thani ---
// #define MAGNETIC_DECLINATION  -1.1f

// --- Chiang Mai ---
// #define MAGNETIC_DECLINATION  -0.8f

// --- Songkhla ---
// #define MAGNETIC_DECLINATION  -0.4f

// ======================================================
// 4. SYSTEM CONSTANTS
// ======================================================
const long GMT_OFFSET_SEC        = 25200; // UTC+7
const int DAYLIGHT_OFFSET_SEC    = 0;

#endif

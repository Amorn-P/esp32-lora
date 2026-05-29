#ifndef USER_CONFIG_H
#define USER_CONFIG_H

// ======================================================
// CherryOne ModbusHealth — Commercial Production v2.0
// ======================================================

// ======================================================
// 1. PRIMARY CREDENTIALS (WiFi, Blynk, Telegram)
// ======================================================

/* WiFi Manager Settings */
#define DEFAULT_THING_NAME       "CherryOne"
#define DEFAULT_AP_PASSWORD      "11111111"
#define DEFAULT_DEVICE_NAME      "CherryOne_01"

/* Fallback WiFi Credentials */
#define FALLBACK_WIFI_SSID       "Gilbert_2.4G"
#define FALLBACK_WIFI_PASSWORD   "3MRTKUHF"

/* Blynk Configuration */
#define DEFAULT_BLYNK_TOKEN      "OKsgCXShzvZOnCvclPJNHRrii1VVntl_"
#define DEFAULT_BLYNK_SERVER     "43.229.135.169"
#define DEFAULT_BLYNK_PORT       8080

/* Telegram Configuration */
#define DEFAULT_TELEGRAM_TOKEN   "7670562126:AAGLY9GcaHFpTdv-BLf8hctbni_82_NowkU"
#define DEFAULT_TELEGRAM_GROUP   "-4641052312"

// ======================================================
// 2. SYSTEM DEFAULTS & LOGIC
// ======================================================
#define DEFAULT_CONFIG_VERSION   "2.1.0"
#define DEFAULT_TIME_SOURCE      1       // 0=RTC, 1=NTP
#define DEFAULT_RESET_HOUR       3       // Daily reset at 3 AM
#define DEFAULT_RESET_MIN        0

// ======================================================
// 2.1 DEBUG CONFIGURATION
// ======================================================
#define SERIAL_DEBUG_ENABLED     1       // 1=debug serial, 0=production silent
#define SIMULATION_MODE          0       // 1=test logic only, 0=real RS485

#if SERIAL_DEBUG_ENABLED
  #define DEBUG_PRINT(x)         Serial.print(x)
  #define DEBUG_PRINTLN(x)       Serial.println(x)
  #define DEBUG_PRINTF(x, ...)   Serial.printf(x, ##__VA_ARGS__)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(x, ...)
#endif

// ======================================================
// 2.2 RS485 PHYSICAL LAYER CONFIGURATION
// ======================================================
// These are critical for reliable Modbus RTU in farm environments.
// See docs/RS485_HARDWARE_GUIDE.md for wiring recommendations.

#define RS485_BAUD               9600    // Must match ALL slave dipswitch settings
#define RS485_RX_PIN             16      // RXD2
#define RS485_TX_PIN             17      // TXD2

// Inter-frame delay (3.5 chars at 9600 baud ≈ 4ms)
// Used between Modbus messages to prevent frame overlap
#define RS485_FRAME_GAP_US       5000

// Max consecutive failures before bus recovery (flush/reinit Serial2)
#define RS485_MAX_FAILURES       10

// ======================================================
// 3. HARDWARE PIN DEFINITIONS
// ======================================================
#define SERIAL_BAUD              115200

#define RTC_CE_PIN               15      // DS1302 CE
#define RTC_IO_PIN               4       // DS1302 I/O
#define RTC_CLK_PIN              5       // DS1302 SCLK

// STATUS_PIN (25) and LED_Internet (26) defined in 11_Wf_Main.h / main.cpp

// ======================================================
// 4. PUMP STATUS INDICATOR & BUZZER (v2.1.0)
// ======================================================
// Local hardware indicators for Pump (SV1 / Board 0 Relay 0)
// These provide audible+visual alerts on the main ESP32 board
// so farm operators can see/hear pump state without Blynk app.

#define PUMP_LED_PIN             32      // 3mm LED: ON=pump running, OFF=pump idle
#define BUZZER_RELAY_PIN         33      // Relay module IN pin (active-LOW optocoupler)

// Buzzer pattern timing (seconds)
// Total alert: BUZZER_ROUNDS × (BUZZER_ON_SEC + BUZZER_OFF_SEC) = 3×15 = 45s
#define BUZZER_ON_SEC            10      // Relay ON duration per round
#define BUZZER_OFF_SEC           5       // Relay OFF duration per round
#define BUZZER_ROUNDS            3       // Total ON/OFF cycles per event

// Active logic: most 1-ch 12VDC relay modules use active-LOW optocoupler
// Set true: relay ON when IN pin is LOW  (common optocoupler module)
// Set false: relay ON when IN pin is HIGH (direct transistor drive)
#define BUZZER_ACTIVE_LOW        true

#endif

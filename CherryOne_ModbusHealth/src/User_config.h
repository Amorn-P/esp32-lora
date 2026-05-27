#ifndef USER_CONFIG_H
#define USER_CONFIG_H

// ======================================================
// 1. PRIMARY CREDENTIALS (WiFi, Blynk, Telegram)
// ======================================================
/* WiFi Manager Settings */
#define DEFAULT_THING_NAME       "CherryOne"  // Name of the WiFi Access Point
#define DEFAULT_AP_PASSWORD      "11111111"        // Password for the WiFi Access Point (8 chars)
#define DEFAULT_DEVICE_NAME      "CherryOne_01"    // Display name in Telegram/Web

/* Fallback WiFi Credentials (used when web portal has no saved config) */
#define FALLBACK_WIFI_SSID       "Gilbert_2.4G"
#define FALLBACK_WIFI_PASSWORD   "3MRTKUHF"

/* Blynk Configuration */
#define DEFAULT_BLYNK_TOKEN      "OKsgCXShzvZOnCvclPJNHRrii1VVntl_"  //Your_Blynk_Token_Here
#define DEFAULT_BLYNK_SERVER     "43.229.135.169"                    // Provided Blynk Server IP
#define DEFAULT_BLYNK_PORT       8080

/* Telegram Configuration */
#define DEFAULT_TELEGRAM_TOKEN   "7670562126:AAGLY9GcaHFpTdv-BLf8hctbni_82_NowkU"  //Your_Bot_Token_Here
#define DEFAULT_TELEGRAM_GROUP   "-4641052312"                                      //Your_Group_ID_Here

// ======================================================
// 2. SYSTEM DEFAULTS & LOGIC
// ======================================================
#define DEFAULT_CONFIG_VERSION   "1.2.0"           // Change this to force a config reset
#define DEFAULT_TIME_SOURCE      1                 // 0 = RTC, 1 = NTP (Default)
#define DEFAULT_RESET_HOUR       3                 // Daily reset at 3 AM
#define DEFAULT_RESET_MIN        0

// ======================================================
// 2.1 DEBUG CONFIGURATION
// ======================================================
#define SERIAL_DEBUG_ENABLED     1                 // Set to 1 to enable, 0 to disable
#define SIMULATION_MODE          0                 // 1: Test logic/Blynk only, 0: Actual RS485 Hardware

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
// 3. HARDWARE PIN DEFINITIONS
// ======================================================
#define SERIAL_BAUD              115200
#define RS485_BAUD               9600
#define RS485_RX_PIN             16                // RXD2
#define RS485_TX_PIN             17                // TXD2

#define RTC_CE_PIN               15                // DS1302 Pins
#define RTC_IO_PIN               4
#define RTC_CLK_PIN              5
#define STATUS_LED_PIN           25                // Indicators

#endif
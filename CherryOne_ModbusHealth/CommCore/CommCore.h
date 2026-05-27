/**
 * CommCore.h - Communication Core Public API
 * 
 * Copy CommCore/ folder to any ESP32 project for instant:
 *   WiFi (IotWebConf), Internet check, Blynk Legacy, Telegram, OTA
 * 
 * Usage:
 *   1. #include "CommCore.h"
 *   2. Call CommCore_begin() in setup()
 *   3. Call CommCore_run() in loop() or Task
 *   4. Define BLYNK_WRITE(Vxx) handlers in your app code
 */

#ifndef COMMCORE_H
#define COMMCORE_H

#include <Arduino.h>
#include "CommCore_Params.h"   // All shared globals
#include "CommCore_WiFi.h"     // WiFi + IotWebConf + internetcheck
#include "CommCore_Web.h"      // Web routes + configSaved + handleRoot
#include "CommCore_Blynk.h"    // Blynk core + V90-V127 system pins
#include "CommCore_Telegram.h" // Bot setup + sendMessage
#include "CommCore_OTA.h"      // OTA firmware update
#include "CommCore_Tasks.h"    // Task1 (IoT loop), Task4 (Blynk)

// ============================================================
// PUBLIC API - call these from your project
// ============================================================

/**
 * Initialize communication core. Call ONCE in setup().
 * Registers WiFi handlers, loads EEPROM, starts IotWebConf.
 */
void CommCore_begin();

/**
 * Run communication loop. Call in loop() or dedicated task.
 * Handles: internet check, Blynk connection, web server.
 */
void CommCore_run();

/**
 * Send Telegram message. Thread-safe.
 * Returns true if queued/sent successfully.
 */
bool CommCore_sendTelegram(const char* msg);

/**
 * Get WiFi connection status
 */
bool CommCore_isWiFiConnected();

/**
 * Get internet connection status
 */
bool CommCore_isInternetConnected();

/**
 * Get Blynk connection status
 */
bool CommCore_isBlynkConnected();

#endif // COMMCORE_H

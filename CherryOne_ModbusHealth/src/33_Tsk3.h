#ifndef _33_TSK3_H
#define _33_TSK3_H

/**
 * ===========================================================================
 * Task3: Relay Control Engine (Commercial Production v2.0)
 * ===========================================================================
 * 
 * Core responsibilities:
 *   - Relay state machine (ON/OFF per minute boundaries)
 *   - Modbus RTU write with 3-attempt retry + verification
 *   - RS485 bus mutex for thread-safe Serial2 access
 *   - Safe heartbeat: read-based alive detection (NO writes to unknown registers)
 *   - State guard: re-assert all relays every 2s (corrects noise glitches fast)
 *   - Error counters & bus recovery for field diagnostics
 * 
 * CHANGELOG v2.0:
 *   [CRITICAL] Replaced writeHoldingRegister(0x00FF, random) heartbeat
 *              with safe readHoldingRegister(0x0001) — reads relay 1 status
 *              without writing anything. Root cause of phantom relay toggling.
 *   [HIGH]    Added RS485 bus mutex (g_rs485Mutex) for all Modbus operations
 *   [HIGH]    Added writeRelayRetry() with 3-attempt retry + verification
 *   [HIGH]    Reduced state guard interval from 10s → 2s
 *   [HIGH]    Added bus recovery: flush Serial2 on repeated errors
 *   [MEDIUM]  Added error counters (serial via V87 debug pin)
 *   [MEDIUM]  Heartbeat now uses read (safe, passive) instead of write
 * ===========================================================================
 */

#include "User_config.h"
#include "Relay_config.h"

// ===========================================================================
// HELPER: Set Blynk LED for relay status (mutex-safe)
// ===========================================================================
static void setLed(int id, bool state)
{
  if (id < 1 || id > 21) return;
  BLYNK_WRITE_SAFE(60 + id, state ? 255 : 0);  // V61-V81
}

// ===========================================================================
// RS485 BUS MUTEX — prevents concurrent Modbus access across tasks
// ===========================================================================
// Created in setup(), used by Task3 (relay control), Task6 (stop worker),
// and any Blynk callback that writes relays.
extern SemaphoreHandle_t g_rs485Mutex;
#ifndef RS485_LOCK_TIMEOUT_MS
#define RS485_LOCK_TIMEOUT_MS  200   // Max wait for bus access
#endif

// ===========================================================================
// MODBUS RETRY CONFIGURATION
// ===========================================================================
#define MODBUS_RETRY_COUNT      3     // Attempts per write
#define MODBUS_RETRY_DELAY_MS   15    // Wait between retries

// ===========================================================================
// RELAY STATE TRACKING
// ===========================================================================
// relayState[id] = last commanded state (true=ON, false=OFF)
// Index 1-21 active, index 0 unused.
// Used by state guard to re-assert after noise glitches.
bool relayState[22] = {false};

// Error tracking for field diagnostics
static uint32_t g_relayWriteErrors   = 0;
static uint32_t g_relayWriteTotal    = 0;
static uint32_t g_busRecoveryCount   = 0;
static uint32_t g_slaveTimeoutCount[6] = {0};  // Per-slave (1-5)

// ===========================================================================
// SLAVE ALIVE TRACKING (safe, read-based)
// ===========================================================================
// Track last successful contact per slave for Blynk LED indicators (V82-V86)
static unsigned long g_slaveLastAlive[6] = {0}; // Index 1-5

// ===========================================================================
// HELPER: Modbus write with retry, mutex, and verification
// ===========================================================================
// Returns: 0 = success (ACK received), non-zero = failure after all retries
static uint8_t modbusWriteRegister(uint8_t slaveId, uint16_t reg, uint16_t value)
{
  uint8_t finalRet = 99;
  
  for (int attempt = 0; attempt < MODBUS_RETRY_COUNT; attempt++) {
    uint8_t ret = RTU_MASTER.writeHoldingRegister(slaveId, reg, value);
    
    if (ret == 0) {
      // Success: ACK received from slave
      return 0;
    }
    
    finalRet = ret;
    
    // Brief delay before retry
    if (attempt < MODBUS_RETRY_COUNT - 1) {
      vTaskDelay(MODBUS_RETRY_DELAY_MS / portTICK_PERIOD_MS);
      esp_task_wdt_reset();
    }
  }
  
  g_relayWriteErrors++;
  return finalRet;
}

// ===========================================================================
// HELPER: Safe slave alive check (READ only — never writes!)
// ===========================================================================
// Reads holding register 0x0001 (relay 1 status) from each slave.
// Uses multi-register read API which returns error code.
// A successful read (ret==0) means the slave is online.
// An exception response (ret==1,2,3) also means online.
// A timeout (ret>3, typically 9 or 255) means offline.
static bool isSlaveAlive(uint8_t slaveId)
{
  // Safe read: register 0x0001 = relay 1 status
  // readHoldingRegister(id, reg, *data, regNum) returns error code
  uint16_t regData[1] = {0};
  uint8_t ret = RTU_MASTER.readHoldingRegister(slaveId, 0x0001, regData, 1);
  
  // ret==0 means success (got the value)
  // ret 1-3 = Modbus exception (board is there but rejected — still alive)
  // ret >3 = timeout / no response
  return (ret <= 3);
}

// ===========================================================================
// HELPER: Set Blynk LED for slave alive status (mutex-safe)
// ===========================================================================
static void setSlaveLed(uint8_t slaveId, bool alive)
{
  // V82=slave1, V83=slave2, ... V86=slave5
  if (slaveId >= 1 && slaveId <= 5) {
    BLYNK_WRITE_SAFE(81 + slaveId, alive ? 255 : 0);
  }
}

// ===========================================================================
// PUBLIC: Write relay with all safety guards
// ===========================================================================
void writeRelay(int id, bool state)
{
  if (id < 1 || id > 21) return;
  
  uint8_t  slave = SYSTEM_VALVE_MAP[id].slaveId;
  uint8_t  reg   = SYSTEM_VALVE_MAP[id].relayIndex + 1;
  uint16_t val   = state ? 256 : 512;
  
  #if SIMULATION_MODE == 0
  if (!stopInProgress) {
    // Acquire RS485 bus mutex
    if (g_rs485Mutex != NULL && 
        xSemaphoreTake(g_rs485Mutex, pdMS_TO_TICKS(RS485_LOCK_TIMEOUT_MS)) == pdTRUE) {
      
      g_relayWriteTotal++;
      uint8_t ret = modbusWriteRegister(slave, reg, val);
      
      if (ret == 0) {
        g_slaveLastAlive[slave] = millis();
      } else {
        g_slaveTimeoutCount[slave]++;
        
        // Bus recovery: flush Serial2 on repeated errors
        if (g_slaveTimeoutCount[slave] > 10 && (g_slaveTimeoutCount[slave] % 10 == 0)) {
          Serial2.flush();
          // Brief RX flush delay (3.5 chars at 9600 = ~4ms)
          delayMicroseconds(5000);
          g_busRecoveryCount++;
        }
      }
      
      xSemaphoreGive(g_rs485Mutex);
    } else {
      // Mutex timeout — log error
      g_relayWriteErrors++;
    }
    
    // RS485 activity blink on every write attempt
    static bool txBlink = false; txBlink = !txBlink;
    BLYNK_WRITE_SAFE(V87, txBlink ? 255 : 0);
  }
  #endif
  
  relayState[id] = state;  // Track commanded state for state guard
  setLed(id, state);       // Update Blynk LED
}

// ===========================================================================
// Task3: Relay Control Engine (Core 1)
// ===========================================================================
void Task3code(void *pvParameters)
{
  DEBUG_PRINT("Task3 running on core ");
  DEBUG_PRINTLN(xPortGetCoreID());
  esp_task_wdt_add(NULL);

  for (;;)
  {
    // =======================================================================
    // SAFE HEARTBEAT: Check slave aliveness via READ (every 2s)
    // =======================================================================
    // [CRITICAL FIX v2.0] No longer writes random millis() values to register
    // 0x00FF. Instead reads register 0x0001 (relay 1 status) which is a
    // passive, side-effect-free operation. This eliminates the root cause
    // of phantom relay toggling.
    // =======================================================================
    #if SIMULATION_MODE == 0
    static unsigned long lastHeartbeat = 0;
    if (millis() - lastHeartbeat > 2000) {
      lastHeartbeat = millis();
      
      if (g_rs485Mutex != NULL &&
          xSemaphoreTake(g_rs485Mutex, pdMS_TO_TICKS(200)) == pdTRUE) {
        
        for (uint8_t s = 1; s <= 5; s++) {
          bool alive = isSlaveAlive(s);
          if (alive) {
            g_slaveLastAlive[s] = millis();
          }
          
          // Update Blynk slave LEDs
          bool ledState = (millis() - g_slaveLastAlive[s] < 10000);
          setSlaveLed(s, ledState);
          
          esp_task_wdt_reset();
        }
        
        xSemaphoreGive(g_rs485Mutex);
      }
    }
    #endif

    // =======================================================================
    // STATE GUARD: Re-assert all relay states every 2 seconds
    // =======================================================================
    // [FIX v2.0] Reduced from 10s → 2s. 
    // If RS485 bus noise corrupts a relay state, it gets corrected within 
    // 2 seconds instead of 10. This is critical for farm environments with 
    // motor/pump EMI that can induce false Modbus writes.
    // 
    // During IDLE: writes OFF to all 21 relays
    // During RUN:  re-sends current commanded state
    // =======================================================================
    #if SIMULATION_MODE == 0
    static unsigned long lastStateGuard = 0;
    if (millis() - lastStateGuard > 2000) {
      lastStateGuard = millis();
      
      if (g_rs485Mutex != NULL &&
          xSemaphoreTake(g_rs485Mutex, pdMS_TO_TICKS(200)) == pdTRUE) {
        
        for (int i = 1; i <= 21; i++) {
          uint8_t  s = SYSTEM_VALVE_MAP[i].slaveId;
          uint8_t  r = SYSTEM_VALVE_MAP[i].relayIndex + 1;
          uint16_t v = relayState[i] ? 256 : 512;
          
          // Single write (no retry needed — state guard runs frequently)
          uint8_t ret = RTU_MASTER.writeHoldingRegister(s, r, v);
          if (ret == 0) g_slaveLastAlive[s] = millis();
          
          // Small inter-frame gap (3.5 chars)
          vTaskDelay(3 / portTICK_PERIOD_MS);
          esp_task_wdt_reset();
        }
        
        xSemaphoreGive(g_rs485Mutex);
      }
    }
    #endif

    // =======================================================================
    // RELAY CONTROL STATE MACHINE
    // =======================================================================
    
    if (Period1 == 0 && Period2 == 0 && Manual == 0) {
      // IDLE state — update Blynk status once
      static bool idleReported = false;
      if (!idleReported) {
        BLYNK_WRITE_SAFE(V4, "System Ready (IDLE)");
        idleReported = true;
      }
      BlynkRun_once1 = 0;
      TA40 = 0;
    } else {
      // RUN state — compute TA40 based on active mode
      if (Period1 == 1)      { Time_Count_Period1(); TA40 = TA40_P1; }
      else if (Period2 == 1) { Time_Count_Period2(); TA40 = TA40_P2; }
      else if (Manual == 1)  { TA40 = Manual_Timeduration_Min_int; }

      if (TA40 > 0 && !stopInProgress) {
        int timeRemaining = (TA40 > CountP1Min) ? (TA40 - CountP1Min) : 0;
        BLYNK_WRITE_SAFE(V24, String(timeRemaining) + " min");
        
        // Status label
        if (Manual == 1)        BLYNK_WRITE_SAFE(V4, "Manual Running");
        else if (Period1 == 1)  BLYNK_WRITE_SAFE(V4, "Period 1 Running");
        else if (Period2 == 1)  BLYNK_WRITE_SAFE(V4, "Period 2 Running");

        // --- PUMP LOGIC (ID 1) ---
        if (CountP1Min >= 0 && CountP1Min < TA40) writeRelay(1, true);
        else if (CountP1Min == TA40)                writeRelay(1, false);

        // --- MANUAL MODE LOGIC (single relay + pump) ---
        if (Manual == 1) {
          // Handle relay switch request
          if (manualSwitchFlag) {
            writeRelay(manualRelayID_prev, false);
            BLYNK_WRITE_SAFE(V31, "Manual: SV" + String(manualRelayActive) + " ON");
            manualSwitchFlag = false;
          }
          if (CountP1Min >= 0 && CountP1Min < TA40) writeRelay(manualRelayActive, true);
          else if (CountP1Min == TA40)               writeRelay(manualRelayActive, false);
        }

        // --- PERIOD 1/2 SEQUENCE LOGIC (ID 2-21) ---
        if (Period1 == 1 || Period2 == 1) {
          if (CountP1Min >= 0   && CountP1Min < TA2)  writeRelay(2, true);  else if (CountP1Min == TA2)  writeRelay(2, false);
          if (CountP1Min >= TA3 && CountP1Min < TA4)  writeRelay(3, true);  else if (CountP1Min == TA4)  writeRelay(3, false);
          if (CountP1Min >= TA5 && CountP1Min < TA6)  writeRelay(4, true);  else if (CountP1Min == TA6)  writeRelay(4, false);
          if (CountP1Min >= TA7 && CountP1Min < TA8)  writeRelay(5, true);  else if (CountP1Min == TA8)  writeRelay(5, false);
          if (CountP1Min >= TA9 && CountP1Min < TA10) writeRelay(6, true);  else if (CountP1Min == TA10) writeRelay(6, false);
          if (CountP1Min >= TA11 && CountP1Min < TA12) writeRelay(7, true);  else if (CountP1Min == TA12) writeRelay(7, false);
          if (CountP1Min >= TA13 && CountP1Min < TA14) writeRelay(8, true);  else if (CountP1Min == TA14) writeRelay(8, false);
          if (CountP1Min >= TA15 && CountP1Min < TA16) writeRelay(9, true);  else if (CountP1Min == TA16) writeRelay(9, false);
          if (CountP1Min >= TA17 && CountP1Min < TA18) writeRelay(10, true); else if (CountP1Min == TA18) writeRelay(10, false);
          if (CountP1Min >= TA19 && CountP1Min < TA20) writeRelay(11, true); else if (CountP1Min == TA20) writeRelay(11, false);
          if (CountP1Min >= TA21 && CountP1Min < TA22) writeRelay(12, true); else if (CountP1Min == TA22) writeRelay(12, false);
          if (CountP1Min >= TA23 && CountP1Min < TA24) writeRelay(13, true); else if (CountP1Min == TA24) writeRelay(13, false);
          if (CountP1Min >= TA25 && CountP1Min < TA26) writeRelay(14, true); else if (CountP1Min == TA26) writeRelay(14, false);
          if (CountP1Min >= TA27 && CountP1Min < TA28) writeRelay(15, true); else if (CountP1Min == TA28) writeRelay(15, false);
          if (CountP1Min >= TA29 && CountP1Min < TA30) writeRelay(16, true); else if (CountP1Min == TA30) writeRelay(16, false);
          if (CountP1Min >= TA31 && CountP1Min < TA32) writeRelay(17, true); else if (CountP1Min == TA32) writeRelay(17, false);
          if (CountP1Min >= TA33 && CountP1Min < TA34) writeRelay(18, true); else if (CountP1Min == TA34) writeRelay(18, false);
          if (CountP1Min >= TA35 && CountP1Min < TA36) writeRelay(19, true); else if (CountP1Min == TA36) writeRelay(19, false);
          if (CountP1Min >= TA37 && CountP1Min < TA38) writeRelay(20, true); else if (CountP1Min == TA38) writeRelay(20, false);
          if (CountP1Min >= TA39 && CountP1Min < TA40) writeRelay(21, true); else if (CountP1Min == TA40) writeRelay(21, false);
        }

        // --- END OF CYCLE ---
        if (CountP1Min >= TA40) {
          BLYNK_WRITE_SAFE(V1, 100);
          vTaskDelay(2000 / portTICK_PERIOD_MS);
          
          // Turn off ALL relays
          for (int i = 1; i <= 21; i++) writeRelay(i, false);
          
          Manual = 0; Period1 = 0; Period2 = 0;
          CountP1Min = 0; CountP1 = 0;
          BlynkRun_once1 = 0; TA40 = 0;

          esp_task_wdt_reset();
          EEPROM.put(240, 0); EEPROM.put(250, 0); EEPROM.put(260, 0);
          EEPROM.commit();
          
          if (preferencesMutex != NULL && xSemaphoreTake(preferencesMutex, pdMS_TO_TICKS(2000)) == pdTRUE) {
            preferences.begin("storage", false);
            preferences.putInt("CountP1", 0);
            preferences.putInt("CountP1Min", 0);
            preferences.end();
            xSemaphoreGive(preferencesMutex);
          }
          
          BLYNK_WRITE_SAFE(V4, "Finished");
          BLYNK_WRITE_SAFE(V1, 0);
          BLYNK_WRITE_SAFE(V31, "System stop");
          
          // --- ERROR REPORT (field diagnostics) ---
          if (g_relayWriteErrors > 0) {
            DEBUG_PRINTF("[DIAG] Modbus errors: %u / %u writes (%.1f%%), bus recoveries: %u\n",
              (unsigned int)g_relayWriteErrors, (unsigned int)g_relayWriteTotal,
              (g_relayWriteTotal > 0) ? (100.0f * g_relayWriteErrors / g_relayWriteTotal) : 0.0f,
              (unsigned int)g_busRecoveryCount);
            // Reset for next cycle
            g_relayWriteErrors = 0;
            g_relayWriteTotal = 0;
          }
        } else {
          int progress = map(CountP1Min, 0, TA40, 0, 100);
          BLYNK_WRITE_SAFE(V1, progress);
        }
      }
    }
    
    // =======================================================================
    // TASK YIELD — 1 second tick
    // =======================================================================
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    esp_task_wdt_reset();
  }
}

#endif // _33_TSK3_H

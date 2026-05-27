#ifndef _33_TSK3_H
#define _33_TSK3_H

#include "User_config.h"
#include "Relay_config.h"

// [HEALTH] Track last successful Modbus write per slave (1-5)
unsigned long slaveLastWrite[6] = {0, 0, 0, 0, 0, 0};

// [STATE GUARD] Track last commanded relay state to defend against RS485 glitches
// Some relay boards can false-trigger from bus noise or power glitches.
// Periodic re-assertion corrects any glitched relay within 10 seconds.
bool relayState[22] = {false};  // Index 1-21, index 0 unused

/**
   ==================================================
   Task3: Pump Control & Solenoid Valve Management
   ==================================================
*/

void setLed(int id, bool state) {
    if (id < 1 || id > 21) return;
    BLYNK_WRITE_SAFE(60 + id, state ? 255 : 0);  // V61-V81, mutex-safe
}

void writeRelay(int id, bool state) {
    if (id < 1 || id > 21) return;
    uint8_t slave = SYSTEM_VALVE_MAP[id].slaveId;
    uint8_t reg = SYSTEM_VALVE_MAP[id].relayIndex + 1;
    // [FIX] Standard Modbus coil ON/OFF commands are often 0xFF00 (ON) and 0x0000 (OFF) when using WriteSingleCoil (0x05)
    // Based on diagnostic tests, 256 for ON and 512 for OFF works with writeHoldingRegister (0x06).
    uint16_t val = state ? 256 : 512;
    
    #if SIMULATION_MODE == 0
    if (!stopInProgress) {
        uint8_t ret = RTU_MASTER.writeHoldingRegister(slave, reg, val);
        if (ret == 0) slaveLastWrite[slave] = millis();  // Track alive
        
        // RS485 activity blink on every Modbus write
        static bool txBlink = false; txBlink = !txBlink;
        BLYNK_WRITE_SAFE(V87, txBlink ? 255 : 0);
    }
    #endif
    relayState[id] = state;  // [STATE GUARD] Track commanded state
    setLed(id, state);
}

void Task3code(void *pvParameters)
{
  DEBUG_PRINT("Task3 running on core ");
  DEBUG_PRINTLN(xPortGetCoreID());
  esp_task_wdt_add(NULL);

  for (;;)
  {
    // [HEALTH] Heartbeat + alive tracking every 2s using writeHoldingRegister retval
    // ret==0 → ACK received → board alive. ret!=0 → timeout → board dead.
    // V82-V86: LED ON=alive, OFF=dead/disconnected (updates in IDLE and RUN modes)
    #if SIMULATION_MODE == 0
    static unsigned long lastHeartbeat = 0;
    if (millis() - lastHeartbeat > 2000) {
      lastHeartbeat = millis();
      for (uint8_t s = 1; s <= 5; s++) {
        // [FIX] A simple readHoldingRegister gets false positives from noise on empty lines.
        // Instead, we ping an invalid register (0x00FF). 
        // If the board exists, it throws a Modbus Exception (Code 1, 2, or 3).
        // If the board is dead/unplugged, it Times Out (Code > 3, usually 255 or 9).
        uint8_t ret = RTU_MASTER.writeHoldingRegister(s, 0x00FF, millis() & 0xFFFF);
        
        // Treat 0 (Success) or 1/2/3 (Exception) as ALIVE.
        // Treat Timeout (e.g. 9 or 255) as DEAD.
        if (ret <= 3) {
           slaveLastWrite[s] = millis();
        }
        
        bool alive = (millis() - slaveLastWrite[s] < 10000);
        BLYNK_WRITE_SAFE(81 + s, alive ? 255 : 0);
        esp_task_wdt_reset();
      }
    }
    #endif

    // [STATE GUARD] Re-assert all relay states every 10s to correct RS485 glitches
    // During IDLE: writes OFF to all 21 relays (defense against false triggers)
    // During RUN: re-sends current commanded state (defense against bus noise)
    #if SIMULATION_MODE == 0
    static unsigned long lastStateGuard = 0;
    if (millis() - lastStateGuard > 10000) {
      lastStateGuard = millis();
      for (int i = 1; i <= 21; i++) {
        uint8_t s = SYSTEM_VALVE_MAP[i].slaveId;
        uint8_t r = SYSTEM_VALVE_MAP[i].relayIndex + 1;
        uint16_t v = relayState[i] ? 256 : 512;
        RTU_MASTER.writeHoldingRegister(s, r, v);
        vTaskDelay(5 / portTICK_PERIOD_MS);
        esp_task_wdt_reset();
      }
    }
    #endif

    if (Period1 == 0 && Period2 == 0 && Manual == 0) {
      BLYNK_WRITE_SAFE(V4, "System Ready (IDLE)");
      BlynkRun_once1 = 0; 
      TA40 = 0;
    } else {
      // Set TA40 based on mode
      if (Period1 == 1) { Time_Count_Period1(); TA40 = TA40_P1; }
      else if (Period2 == 1) { Time_Count_Period2(); TA40 = TA40_P2; }
      else if (Manual == 1) { TA40 = Manual_Timeduration_Min_int; }

      if (TA40 > 0 && !stopInProgress) {
        int timeRemaining = (TA40 > CountP1Min) ? (TA40 - CountP1Min) : 0;
        BLYNK_WRITE_SAFE(V24, String(timeRemaining) + " min");
        
        if (Manual == 1) BLYNK_WRITE_SAFE(V4, "Manual Running");
        else if (Period1 == 1) BLYNK_WRITE_SAFE(V4, "Period 1 Running");
        else if (Period2 == 1) BLYNK_WRITE_SAFE(V4, "Period 2 Running");

        // --- PUMP LOGIC (ID 1) ---
        if (CountP1Min >= 0 && CountP1Min < TA40) writeRelay(1, true);
        else if (CountP1Min == TA40) writeRelay(1, false);

        // --- MANUAL MODE LOGIC (single relay + pump) ---
        if (Manual == 1) {
            // Handle relay switch request
            if (manualSwitchFlag) {
                writeRelay(manualRelayID_prev, false);  // Turn off old
                BLYNK_WRITE_SAFE(V31, "Manual: SV" + String(manualRelayActive) + " ON");
                manualSwitchFlag = false;
            }
            if (CountP1Min >= 0 && CountP1Min < TA40) writeRelay(manualRelayActive, true);
            else if (CountP1Min == TA40) writeRelay(manualRelayActive, false);
        }
        
        // --- PERIOD 1/2 SEQUENCE LOGIC (ID 2-21) ---
        if (Period1 == 1 || Period2 == 1) {
            if (CountP1Min >= 0 && CountP1Min < TA2) writeRelay(2, true);
            else if (CountP1Min == TA2) writeRelay(2, false);
            
            if (CountP1Min >= TA3 && CountP1Min < TA4) writeRelay(3, true); else if (CountP1Min == TA4) writeRelay(3, false);
            if (CountP1Min >= TA5 && CountP1Min < TA6) writeRelay(4, true); else if (CountP1Min == TA6) writeRelay(4, false);
            if (CountP1Min >= TA7 && CountP1Min < TA8) writeRelay(5, true); else if (CountP1Min == TA8) writeRelay(5, false);
            if (CountP1Min >= TA9 && CountP1Min < TA10) writeRelay(6, true); else if (CountP1Min == TA10) writeRelay(6, false);
            if (CountP1Min >= TA11 && CountP1Min < TA12) writeRelay(7, true); else if (CountP1Min == TA12) writeRelay(7, false);
            if (CountP1Min >= TA13 && CountP1Min < TA14) writeRelay(8, true); else if (CountP1Min == TA14) writeRelay(8, false);
            if (CountP1Min >= TA15 && CountP1Min < TA16) writeRelay(9, true); else if (CountP1Min == TA16) writeRelay(9, false);
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
            
            // [FIX] Turn off ALL relays (including any added during Manual)
            for (int i = 1; i <= 21; i++) writeRelay(i, false);
            
            Manual = 0; Period1 = 0; Period2 = 0;
            CountP1Min = 0; CountP1 = 0;
            BlynkRun_once1 = 0; TA40 = 0;

            esp_task_wdt_reset();  // [FIX] Watchdog before EEPROM commit
            EEPROM.put(240, 0); EEPROM.put(250, 0); EEPROM.put(260, 0);
            EEPROM.commit();
            
            // [FIX] Use mutex for preferences
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
        } else {
            int progress = map(CountP1Min, 0, TA40, 0, 100);
            BLYNK_WRITE_SAFE(V1, progress);
        }
      }
    }
    
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    esp_task_wdt_reset();
  }
}

#endif

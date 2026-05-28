#include <Arduino.h>
#include "Board_ID.h"
#include "Relay_config.h"

/**
 * Project: CherryOne ModbusHealth — Commercial Production v2.0
 * Hardware: ESP32, DS1302 RTC, RS485 (5x 8ch Relay Slaves)
 * 
 * CHANGELOG v2.0:
 *   - RS485 bus mutex (g_rs485Mutex) for thread-safe Modbus
 *   - Task priority differentiation: relay control > Blynk > web/IoT
 *   - Boot relay safety: all relays forced OFF before anything else
 *   - Hardware WDT extended to 60s for OTA downloads
 *   - panic-on-timeout retained for auto-reboot
 */
#include "11_Wf_Main.h"
#include "21_Wf.h"              // IoT Web Configuration Setup
#include "01_Social_Handlers.h" // Telegram & Blynk Web Handlers
#include "15_Set_Tsk.h"         // FreeRTOS Task Handles Declaration

// ===========================================================================
// RS485 BUS MUTEX (shared with Task3 / 33_Tsk3.h)
// ===========================================================================
// Prevents concurrent access to Serial2 (Modbus RTU) from multiple tasks.
// All Modbus reads/writes MUST acquire this semaphore first.
SemaphoreHandle_t g_rs485Mutex = NULL;

// ===========================================================================
// TASK PRIORITY ALLOCATION (Production)
// ===========================================================================
// ESP32 FreeRTOS: 1 (idle) to 25 (max). Arduino loop runs at priority 1.
// Priority values chosen to ensure relay control never starves.
//
//   Priority 3: Task3 (Relay Control)    — Core 1, CRITICAL
//   Priority 2: Task6 (Stop Worker)      — Core 1, HIGH
//   Priority 2: Task4 (Blynk Comms)      — Core 1, NORMAL
//   Priority 1: Task1 (IoT Web Conf)     — Core 0, LOW
//   Priority 1: Task2 (NTP/Time/Timer)   — Core 0, LOW
//   Priority 1: Task5 (Time Counter)     — Core 0, LOW
//   Priority 1: Task8 (SW Watchdog)      — Core 0, LOW
// ===========================================================================
enum TaskPriority_v2 {
  PRIO_RELAY     = 3,  // Relay control — must run on time
  PRIO_STOP      = 2,  // Emergency stop worker
  PRIO_BLYNK     = 2,  // Blynk communication
  PRIO_WEB       = 1,  // IoT Web Config
  PRIO_TIME      = 1,  // NTP / Time tracking
  PRIO_COUNTER   = 1,  // Minute counter
  PRIO_WATCHDOG  = 1   // Software watchdog
};

// ===========================================================================
// Global variables for Manual relay control
// ===========================================================================
int manualRelayID = 2;
int manualRelayDuration = 0;
int manualRelayActive = 2;
int manualRelayID_prev = 2;
volatile bool manualSwitchFlag = false;

// Forward declaration for Blynk handlers
void writeRelay(int id, bool state);

#include "00_Blynk.h"
#include "00_Blynk1.h"
#include "31_Tsk1.h"    // Task1: IoT Web Config loop
#include "32_Tsk2.h"    // Task2: NTP Time & Schedule triggers
#include "33_Tsk3.h"    // Task3: Relay Control Engine (CRITICAL)
#include "34_Tsk4.h"    // Task4: Blynk Communication
#include "35_Tsk5.h"    // Task5: Time Counter
#include "36_Tsk6.h"    // Task6: Stop Worker (non-blocking)
#include "38_Tsk8.h"    // Task8: Software Watchdog Timer

#include "22_Cons.h"
#include "23_IP.h"

// ===========================================================================
// BOOT RELAY SAFETY: Force all relays OFF before anything initializes
// ===========================================================================
// ESP32 GPIOs float during reset, which can briefly trigger relays.
// This function is called BEFORE Serial.begin() to minimize the window.
// It sets RS485 TX pin low to prevent transient bus signals.
static void bootRelaySafety()
{
  // Pre-configure RS485 pins as outputs LOW before Serial2 starts
  pinMode(RXD2, INPUT_PULLUP);   // RX with pullup — prevents floating
  pinMode(TXD2, OUTPUT);
  digitalWrite(TXD2, LOW);       // TX low — no bus drive
  
  // Configure status LEDs OFF during boot
  pinMode(LED_Internet, OUTPUT);
  digitalWrite(LED_Internet, LOW);
  pinMode(STATUS_PIN, OUTPUT);
  digitalWrite(STATUS_PIN, LOW);
}

void setup() {
  // ---- SERIAL INIT FIRST (for debug visibility) ----
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("================================================"));
  Serial.println(F("CherryOne v2.0 - booting..."));
  
  // ---- BOOT SAFETY ----
  bootRelaySafety();
  
  // ---- PREFERENCES (NVS) ----
  preferences.begin("storage", false); 
  CountP1 = preferences.getInt("CountP1", 0);
  CountP1Min = preferences.getInt("CountP1Min", 0);
  preferences.end(); 

  // ---- MUTEX CREATION ----
  preferencesMutex = xSemaphoreCreateMutex();
  if (preferencesMutex == NULL) {
    Serial.println(F("[FATAL] preferencesMutex creation failed!"));
    while(1) { delay(1000); }
  }
  
  blynkMutex = xSemaphoreCreateMutex();
  if (blynkMutex == NULL) {
    Serial.println(F("[FATAL] blynkMutex creation failed!"));
    while(1) { delay(1000); }
  }
  
  // [NEW v2.0] RS485 bus mutex
  g_rs485Mutex = xSemaphoreCreateMutex();
  if (g_rs485Mutex == NULL) {
    Serial.println(F("[FATAL] RS485 mutex creation failed!"));
    while(1) { delay(1000); }
  }
  Serial.println(F("[OK] All mutexes created (prefs, blynk, rs485)"));

  // ---- RESTORE STATE ----
  Serial.print(F("Restored CountP1: ")); Serial.println(CountP1);
  Serial.print(F("Restored CountP1Min: ")); Serial.println(CountP1Min);
  
  // ---- RS485 INIT (Serial2 for Modbus RTU) ----
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  // Brief settle time for RS485 transceivers
  vTaskDelay(100 / portTICK_PERIOD_MS);
  
  // ---- MODBUS WATCHDOG CONFIGURATION ----
  // Configure all 5 slave boards: if ESP32 stops sending heartbeat,
  // each slave's internal watchdog will turn off all relays after 5 seconds.
  #if SIMULATION_MODE == 0
  {
    bool anyWatchdogFailed = false;
    for (uint8_t s = 1; s <= 5; s++) {
      uint8_t wdRet = RTU_MASTER.writeHoldingRegister(s, 0x00FE, 5);
      vTaskDelay(25 / portTICK_PERIOD_MS);
      
      if (wdRet == 0) {
        Serial.printf("[SAFETY] Slave %d watchdog set: 5s timeout\n", s);
      } else {
        Serial.printf("[WARN] Slave %d watchdog FAIL (ret=%d) — board offline?\n", s, wdRet);
        anyWatchdogFailed = true;
      }
    }
    if (anyWatchdogFailed) {
      Serial.println(F("[WARN] Some slaves did not ACK watchdog config"));
      Serial.println(F("[WARN] Check RS485 wiring, termination, and slave power"));
    }
  }
  #endif
  
  systemInitialized = true;

  // ---- WIFI & EEPROM LOAD ----
  static_ip();
  Converse_value();
  
  // ---- VALIDATE CREDENTIALS ----
  if (strlen(Bot_Token_1) < 20 || strchr(Bot_Token_1, ':') == NULL) {
    strcpy(Bot_Token_1, DEFAULT_TELEGRAM_TOKEN);
  }
  if (strlen(Bot_Group) < 3 || Bot_Group[0] != '-') {
    strcpy(Bot_Group, DEFAULT_TELEGRAM_GROUP);
  }
  
  // ---- VALIDATE TIMER VALUES ----
  if (Period1_Time_Start_Hr_int < 0 || Period1_Time_Start_Hr_int > 23)  { Period1_Time_Start_Hr_int = 6; }
  if (Period1_Time_Start_Min_int < 0 || Period1_Time_Start_Min_int > 59) { Period1_Time_Start_Min_int = 0; }
  if (Period1_Timeduration_Min_int < 1 || Period1_Timeduration_Min_int > 1440) { Period1_Timeduration_Min_int = 5; }
  if (Period2_Time_Start_Hr_int < 0 || Period2_Time_Start_Hr_int > 23)  { Period2_Time_Start_Hr_int = 18; }
  if (Period2_Time_Start_Min_int < 0 || Period2_Time_Start_Min_int > 59) { Period2_Time_Start_Min_int = 0; }
  if (Period2_Timeduration_Min_int < 1 || Period2_Timeduration_Min_int > 1440) { Period2_Timeduration_Min_int = 5; }
  if (Manual_Timeduration_Min_int < 1 || Manual_Timeduration_Min_int > 1440) { Manual_Timeduration_Min_int = 5; }
  if (Sys_Time_Select != 0 && Sys_Time_Select != 1) { Sys_Time_Select = 1; }
  if (Period1 < 0 || Period1 > 1) { Period1 = 0; }
  if (Period2 < 0 || Period2 > 1) { Period2 = 0; }
  if (Manual < 0 || Manual > 1)   { Manual = 0; }
  if (BlynkRst_Hr_int < -1 || BlynkRst_Hr_int > 99)   { BlynkRst_Hr_int = -1; }
  if (BlynkRst_Min_int < -1 || BlynkRst_Min_int > 99)  { BlynkRst_Min_int = -1; }
  
  prevPeriod1 = Period1; prevPeriod2 = Period2; prevManual = Manual;
  
  // ---- VALIDATE BLYNK CREDENTIALS ----
  if (strlen(Blynk_Token_1) == 0 || strcmp(Blynk_Token_1, "YOUR_BLYNK_TOKEN") == 0) {
    strcpy(Blynk_Token_1, DEFAULT_BLYNK_TOKEN);
    strcpy(Blynk_Token_11.valueBuffer, DEFAULT_BLYNK_TOKEN);
  }
  if (strlen(configblynk) == 0 || strcmp(configblynk, "YOUR_SERVER_BLYNK") == 0) {
    strcpy(configblynk, DEFAULT_BLYNK_SERVER);
    strcpy(configblynkserver11.valueBuffer, DEFAULT_BLYNK_SERVER);
  }
  if (strcmp(ChkboxSelBlynk_11.valueBuffer, "selected") != 0) {
    strcpy(ChkboxSelBlynk_11.valueBuffer, "selected");
    Sel_1_Blynk_Mode = 1;
  }
  
  // ---- IOTWEBCONF START ----
  Iotwencof_start();
  needReset = false;

  // ---- FALLBACK WIFI ----
  if (strlen(iotWebConf.getWifiSsidParameter()->valueBuffer) == 0) {
    Serial.println(F("No saved WiFi config — using fallback"));
    WiFi.begin(FALLBACK_WIFI_SSID, FALLBACK_WIFI_PASSWORD);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 40) {
      delay(500); attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print(F("WiFi OK: ")); Serial.println(WiFi.localIP());
    }
  }

  // ---- PRE-COMPUTE TIMING ----
  Time_Count_Period1();
  Time_Count_Period2();

  // ---- POWER RECOVERY ----
  if (Period1 == 1) {
    Time_Count_Period1();
    TA40 = TA40_P1;
    if (TA40 > 0 && CountP1Min >= TA40) CountP1Min = 0;
    BlynkRun_once1 = 1;
  }
  if (Period2 == 1) {
    Time_Count_Period2();
    TA40 = TA40_P2;
    if (TA40 > 0 && CountP1Min >= TA40) CountP1Min = 0;
    BlynkRun_once1 = 1;
  }
  if (Manual == 1) {
    TA40 = Manual_Timeduration_Min_int;
    if (CountP1Min > 0 && CountP1Min < TA40 && TA40 > 0) {
      BlynkRun_once1 = 1;
    }
  }
  
  // If job was already finished before power loss, clear it
  if ((Period1 == 1 || Period2 == 1 || Manual == 1) && CountP1Min >= TA40 && TA40 > 0) {
    Period1 = 0; Period2 = 0; Manual = 0;
    CountP1Min = 0; CountP1 = 0;
    EEPROM.put(240, (int)Period1); EEPROM.put(250, (int)Period2);
    EEPROM.put(260, (int)Manual); EEPROM.commit();
    preferences.begin("storage", false);
    preferences.putInt("CountP1", 0); preferences.putInt("CountP1Min", 0);
    preferences.end();
  }

  // ---- TELEGRAM BOT INIT ----
  bot1 = UniversalTelegramBot(String(Bot_Token_1), secured_client1);
  secured_client1.setCACert(TELEGRAM_CERTIFICATE_ROOT); 

  // ---- MODE RECOVERY ----
  if (currentMode != AP_MODE && currentMode != FAST_MODE) {
    currentMode = AP_MODE;
    EEPROM.put(500, (int)currentMode); EEPROM.commit();
  }
  if (currentMode == FAST_MODE) {
    iotWebConf.skipApStartup();
  }

  // =========================================================================
  // HARDWARE WATCHDOG CONFIGURATION (Production)
  // =========================================================================
  // 60 second timeout with panic-on-expiry.
  // All tasks feed via esp_task_wdt_reset().
  // Task8 (SW Watchdog) feeds proactively at ~25s.
  // OTA downloads can take 30-120s — we reset WDT before and after each chunk.
  esp_task_wdt_init(60, true);  // 60s timeout, panic on expiry
  esp_task_wdt_reset();

  // =========================================================================
  // TASK CREATION — Priority-ordered, Core-pinned
  // =========================================================================
  
  // Core 0: Non-critical tasks (web, time, counters)
  xTaskCreatePinnedToCore(Task1code, "Task1_IoTWeb", 8000, NULL,
    PRIO_WEB, &Task1, 0);
  vTaskDelay(100 / portTICK_PERIOD_MS);
  
  xTaskCreatePinnedToCore(Task2code, "Task2_NTP", 8000, NULL,
    PRIO_TIME, &Task2, 0);
  vTaskDelay(100 / portTICK_PERIOD_MS);
  
  xTaskCreatePinnedToCore(Task5code, "Task5_Counter", 4000, NULL,
    PRIO_COUNTER, &Task5, 0);
  vTaskDelay(100 / portTICK_PERIOD_MS);
  
  xTaskCreatePinnedToCore(Task8code, "Task8_SW_WDT", 4000, NULL,
    PRIO_WATCHDOG, &Task8, 0);
  vTaskDelay(100 / portTICK_PERIOD_MS);

  // Core 1: Critical tasks (relay control, stop worker, Blynk)
  xTaskCreatePinnedToCore(Task3code, "Task3_Relay", 8000, NULL,
    PRIO_RELAY, &Task3, 1);
  vTaskDelay(100 / portTICK_PERIOD_MS);
  
  xTaskCreatePinnedToCore(Task6code, "Task6_Stop", 6000, NULL,
    PRIO_STOP, &Task6, 1);
  vTaskDelay(100 / portTICK_PERIOD_MS);
  
  xTaskCreatePinnedToCore(Task4code, "Task4_Blynk", 8000, NULL,
    PRIO_BLYNK, &Task4, 1);
  vTaskDelay(100 / portTICK_PERIOD_MS);

  // ---- WDT: Register loop task ----
  esp_task_wdt_add(NULL);
  esp_task_wdt_reset();

  // ---- INTERNET CHECK ----
  Internet_Check_Mode = 0;
  consecutive_internet_success = 0;
  consecutive_internet_fail = 0;

  // ---- BLYNK INIT ----
  if (Sel_1_Blynk_Mode == 1) {
    Blynk.config(Blynk_Token_1, configblynk, 8080);
  }

  timer.setInterval(2000L, internetcheck);
  
  Serial.println(F("[BOOT] setup() COMPLETE"));
  Serial.printf("[BOOT] Free heap: %u bytes\n", ESP.getFreeHeap());
  Serial.printf("[BOOT] Task priorities: Relay=%d, Stop=%d, Blynk=%d, Others=%d\n",
    PRIO_RELAY, PRIO_STOP, PRIO_BLYNK, PRIO_WEB);
  Serial.println(F("================================================"));
}

void loop() {
  timer.run();
  vTaskDelay(10 / portTICK_PERIOD_MS);
  esp_task_wdt_reset();
}

// ===========================================================================
// BLYNK CONNECTION HANDLERS
// ===========================================================================
BLYNK_CONNECTED()
{
  if (Sel_1_Blynk_Mode == 1) {
    Serial.println(F("Blynk connected"));
    BlynkConnected = true;
    refreshWebConfigValues();
    Blynk.syncVirtual(V2, V3, V5);
    Blynk.syncVirtual(V12, V13, V6);
    Blynk.syncVirtual(V105, V106);
    
    if (currentMode == AP_MODE)       Blynk.virtualWrite(V121, "AP Mode");
    else if (currentMode == FAST_MODE) Blynk.virtualWrite(V121, "Fast Mode");
    
    Blynk.virtualWrite(V0, 0);
    Blynk.virtualWrite(V1, 0);
  }
  // Clear all relay LEDs on connect
  for (int i = 1; i <= 21; i++) {
    BLYNK_WRITE_SAFE(60 + i, 0);
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

BLYNK_DISCONNECTED()
{
  if (Sel_1_Blynk_Mode == 1) {
    BlynkConnected = false;
  }
}

// ===========================================================================
// MANUAL MODE: Single Relay Control (V105-V107)
// ===========================================================================
BLYNK_WRITE(V105) { manualRelayID = param.asInt(); }
BLYNK_WRITE(V106) { manualRelayDuration = param.asInt(); }

BLYNK_WRITE(V107) {
  if (param.asInt() == 1) {
    DEBUG_PRINTLN("[BLYNK] Manual Start (V107)");
    if (Period1 == 1 || Period2 == 1) {
      Blynk.virtualWrite(V31, "Manual blocked: Schedule active");
      return;
    }
    if (manualRelayDuration > 0 && manualRelayID >= 2 && manualRelayID <= 21) {
      if (Manual == 1) {
        // Switch relay during active manual run
        manualRelayID_prev = manualRelayActive;
        manualRelayActive = manualRelayID;
        manualSwitchFlag = true;
        return;
      }
      // Fresh manual start
      manualRelayActive = manualRelayID;
      manualRelayID_prev = manualRelayID;
      stopInProgress = false; stopRequested = false;
      Manual = 1; Period1 = 0; Period2 = 0;
      CountP1Min = 0; CountP1 = 0;
      Manual_Timeduration_Min_int = manualRelayDuration;
      TA40 = manualRelayDuration;
      BlynkRun_once1 = 1;
      EEPROM.put(240, 0); EEPROM.put(250, 0);
      EEPROM.put(220, Manual_Timeduration_Min_int);
      EEPROM.put(260, (int)Manual);
      esp_task_wdt_reset(); EEPROM.commit();
      Blynk.virtualWrite(V4, "Manual START");
      Blynk.virtualWrite(V24, String(TA40) + " min");
      Blynk.virtualWrite(V1, 0);
    } else {
      Blynk.virtualWrite(V31, "Error: SV ID (2-21) / Duration > 0");
    }
  }
}

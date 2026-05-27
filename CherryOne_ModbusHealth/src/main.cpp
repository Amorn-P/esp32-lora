#include <Arduino.h>
#include "Board_ID.h"
#include "Relay_config.h"

/**
 * Project: Auto_Pump_Control
 * Hardware: ESP32, DS1302 RTC, RS485 (Relay Slaves)
 */
#include "11_Wf_Main.h"
#include "21_Wf.h"      // IoT Web Configuration Setup
#include "01_Social_Handlers.h" // Telegram & Blynk Web Handlers
#include "15_Set_Tsk.h" // FreeRTOS Task Handles Declaration ( include  Task files)

// Global variables used by Tasks (declared before task includes)
int manualRelayID = 2;
int manualRelayDuration = 0;
int manualRelayActive = 2;  // Actually ON right now
int manualRelayID_prev = 2;
volatile bool manualSwitchFlag = false;

#include "00_Blynk.h"
#include "00_Blynk1.h"
#include "31_Tsk1.h"    //Task1 
#include "32_Tsk2.h"    //Task2
#include "33_Tsk3.h"    //Task3
#include "34_Tsk4.h"    //Task4
#include "35_Tsk5.h"    //Task5
#include "36_Tsk6.h"    //Task6 - Stop Worker (non-blocking)
#include "38_Tsk8.h"    //Task8 - Software Watchdog Timer & Stack Monitor

#include "22_Cons.h"
#include "23_IP.h"

void setup() {
  pinMode(LED_Internet, OUTPUT);
  digitalWrite(LED_Internet, LOW);

  // RTC Pins for DS1302: CE=15, IO=4, CLK=5
  Serial.begin(115200);
  Serial.println();
  Serial.println("************************************************");
  Serial.println("* CHECK SERIAL MONITOR SPEED: SET TO 9600      *");
  Serial.println("************************************************");
  Serial.println("================================================");
  Serial.println("ESP32 System Startup");
  Serial.println("================================================");
  preferences.begin("storage", false); 
  CountP1 = preferences.getInt("CountP1", 0);
  CountP1Min = preferences.getInt("CountP1Min", 0);
  preferences.end(); 

  preferencesMutex = xSemaphoreCreateMutex();
  if (preferencesMutex == NULL) {
    Serial.println("[ERROR] Failed to create preferencesMutex!");
    while(1) { delay(1000); }
  }
  Serial.println("[OK] preferencesMutex created");

  blynkMutex = xSemaphoreCreateMutex();
  if (blynkMutex == NULL) {
    Serial.println("[ERROR] Failed to create blynkMutex! System halted.");
    while(1) { delay(1000); }
  }
  Serial.println("[OK] blynkMutex created");

  Serial.print("Restored CountP1: ");
  Serial.println(CountP1);
  Serial.print("Restored CountP1Min: ");
  Serial.println(CountP1Min);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  
  // [SAFETY] Configure Modbus relay watchdog timeout = 5s on all slaves
  // If ESP32 dies, relays auto-shutoff after 5s of no heartbeat
  #if SIMULATION_MODE == 0
  for (uint8_t s = 1; s <= 5; s++) {
    uint8_t wdRet = RTU_MASTER.writeHoldingRegister(s, 0x00FE, 5);
    if (wdRet == 0) {
      Serial.printf("[SAFETY] Slave %d watchdog set (5s timeout)\n", s);
    } else {
      Serial.printf("[SAFETY] Slave %d watchdog FAIL (ret=%d) — board may be offline\n", s, wdRet);
    }
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
  #endif
  
  systemInitialized = true;

  // [FIX] NO EEPROM.begin() - Core 1.0.6 auto-inits. Calling begin() before IotWebConf corrupts NVS
  static_ip();
  Serial.println("[BOOT] static_ip() done - handlers registered");
  Converse_value();
  Serial.println("[BOOT] Converse_value() done - EEPROM loaded");
  
  // [FIX] Validate Telegram credentials - replace garbage with defaults
  if (strlen(Bot_Token_1) < 20 || strchr(Bot_Token_1, ':') == NULL) {
    Serial.println("[FIX] Bot_Token_1 corrupted - using DEFAULT_TELEGRAM_TOKEN");
    strcpy(Bot_Token_1, DEFAULT_TELEGRAM_TOKEN);
  }
  if (strlen(Bot_Group) < 3 || Bot_Group[0] != '-') {
    Serial.println("[FIX] Bot_Group corrupted - using DEFAULT_TELEGRAM_GROUP");
    strcpy(Bot_Group, DEFAULT_TELEGRAM_GROUP);
  }
  
  // [FIX] Validate EEPROM timer values - replace -1 garbage with defaults
  if (Period1_Time_Start_Hr_int < 0 || Period1_Time_Start_Hr_int > 23) {
    Serial.println("[FIX] P1 Start Hr corrupted - using 6"); Period1_Time_Start_Hr_int = 6; }
  if (Period1_Time_Start_Min_int < 0 || Period1_Time_Start_Min_int > 59) {
    Serial.println("[FIX] P1 Start Min corrupted - using 0"); Period1_Time_Start_Min_int = 0; }
  if (Period1_Timeduration_Min_int < 1 || Period1_Timeduration_Min_int > 1440) {
    Serial.println("[FIX] P1 Duration corrupted - using 5"); Period1_Timeduration_Min_int = 5; }
  if (Period2_Time_Start_Hr_int < 0 || Period2_Time_Start_Hr_int > 23) {
    Serial.println("[FIX] P2 Start Hr corrupted - using 18"); Period2_Time_Start_Hr_int = 18; }
  if (Period2_Time_Start_Min_int < 0 || Period2_Time_Start_Min_int > 59) {
    Serial.println("[FIX] P2 Start Min corrupted - using 0"); Period2_Time_Start_Min_int = 0; }
  if (Period2_Timeduration_Min_int < 1 || Period2_Timeduration_Min_int > 1440) {
    Serial.println("[FIX] P2 Duration corrupted - using 5"); Period2_Timeduration_Min_int = 5; }
  if (Manual_Timeduration_Min_int < 1 || Manual_Timeduration_Min_int > 1440) {
    Serial.println("[FIX] Manual Duration corrupted - using 5"); Manual_Timeduration_Min_int = 5; }
  if (Sys_Time_Select != 0 && Sys_Time_Select != 1) {
    Serial.println("[FIX] Time Source corrupted - using 1 (NTP)"); Sys_Time_Select = 1; }
  if (Period1 < 0 || Period1 > 1) {
    Serial.println("[FIX] Period1 status corrupted - using 0"); Period1 = 0; }
  if (Period2 < 0 || Period2 > 1) {
    Serial.println("[FIX] Period2 status corrupted - using 0"); Period2 = 0; }
  if (Manual < 0 || Manual > 1) {
    Serial.println("[FIX] Manual status corrupted - using 0"); Manual = 0; }
  if (BlynkRst_Hr_int < -1 || BlynkRst_Hr_int > 99) {
    Serial.println("[FIX] Reset Hr corrupted - using -1 (disabled)"); BlynkRst_Hr_int = -1; }
  if (BlynkRst_Min_int < -1 || BlynkRst_Min_int > 99) {
    Serial.println("[FIX] Reset Min corrupted - using -1 (disabled)"); BlynkRst_Min_int = -1; }
  
  // Sync prevPeriod variables after validation
  prevPeriod1 = Period1;
  prevPeriod2 = Period2;
  prevManual = Manual;
  
  // [FIX] Use User_config.h defaults when IotWebConf params are empty/placeholder
  if (strlen(Blynk_Token_1) == 0 || strcmp(Blynk_Token_1, "YOUR_BLYNK_TOKEN") == 0) {
    Serial.println("[FIX] Blynk token empty - using DEFAULT_BLYNK_TOKEN");
    strcpy(Blynk_Token_1, DEFAULT_BLYNK_TOKEN);
    strcpy(Blynk_Token_11.valueBuffer, DEFAULT_BLYNK_TOKEN);
  }
  if (strlen(configblynk) == 0 || strcmp(configblynk, "YOUR_SERVER_BLYNK") == 0) {
    Serial.println("[FIX] Blynk server empty - using DEFAULT_BLYNK_SERVER");
    strcpy(configblynk, DEFAULT_BLYNK_SERVER);
    strcpy(configblynkserver11.valueBuffer, DEFAULT_BLYNK_SERVER);
  }
  if (strcmp(ChkboxSelBlynk_11.valueBuffer, "selected") != 0) {
    Serial.println("[FIX] Blynk not enabled - enabling");
    strcpy(ChkboxSelBlynk_11.valueBuffer, "selected");
    Sel_1_Blynk_Mode = 1;
  }
  
  Iotwencof_start();
  Serial.println("[BOOT] Iotwencof_start() done - AP should be up");
  needReset = false;  // [FIX] Prevent config reset from triggering immediate reboot after EEPROM wipe

  // ===========================================
  // [FIX] Fallback WiFi connection
  // Try hardcoded credentials when IotWebConf has no saved config
  // ===========================================
  if (strlen(iotWebConf.getWifiSsidParameter()->valueBuffer) == 0) {
    Serial.println("========================================");
    Serial.println("⚠️ No saved WiFi config. Using fallback credentials...");
    Serial.print("📡 SSID: "); Serial.println(FALLBACK_WIFI_SSID);
    Serial.println("========================================");

    WiFi.begin(FALLBACK_WIFI_SSID, FALLBACK_WIFI_PASSWORD);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 40) {
      delay(500);
      attempts++;
      Serial.print(".");
      if (attempts % 10 == 0) {
        Serial.print(" ["); Serial.print(attempts / 2); Serial.println("s]");
      }
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("✅ WiFi CONNECTED via fallback!");
      Serial.print("📡 IP: "); Serial.println(WiFi.localIP());
    } else {
      Serial.print("❌ WiFi FAILED. Status: "); Serial.println(WiFi.status());
      Serial.println("👉 AP remains active at ESP32_CherryOne for config");
    }
  } else {
    Serial.println("✅ IotWebConf has saved WiFi config - will connect via normal flow");
  }

  // Always initialize total durations at startup to prevent "Instant-Finish" bug
  Time_Count_Period1();
  Time_Count_Period2();

  // Power Recovery Handling
  if (Period1 == 1) {
    Time_Count_Period1(); // Ensure timeline markers (TA2, TA3...) match P1
    TA40 = TA40_P1;
    if (TA40 > 0 && CountP1Min >= TA40) CountP1Min = 0; 
    BlynkRun_once1 = 1; // Mark as running to prevent Task2 re-trigger
  }
  
  if (Period2 == 1) {
    Time_Count_Period2(); // Ensure timeline markers (TA2, TA3...) match P2
    TA40 = TA40_P2;
    if (TA40 > 0 && CountP1Min >= TA40) CountP1Min = 0;
    BlynkRun_once1 = 1; // Mark as running to prevent Task2 re-trigger
  }

  if (Manual == 1) {
    TA40 = Manual_Timeduration_Min_int; // For single relay manual mode, TA40 is the duration of the single relay
    if (CountP1Min > 0 && CountP1Min < TA40 && TA40 > 0) {
      Serial.println("================================================");
      Serial.println("Manual RESUME: Power recovered");
      Serial.print("Resuming from minute: ");
      Serial.print(CountP1Min);
      Serial.print(" / ");
      Serial.println(TA40);
      Serial.println("================================================");
      BlynkRun_once1 = 1;  // flag  Task2  TA40 
    }
  }
  
  if ((Period1 == 1 || Period2 == 1 || Manual == 1) && CountP1Min >= TA40 && TA40 > 0) {
    Serial.println("================================================");
    Serial.println("Job finished before power loss -> Clearing status");
    Serial.println("================================================");
    Period1 = 0;
    Period2 = 0;
    Manual = 0;
    CountP1Min = 0;
    CountP1 = 0;
    EEPROM.put(240, (int)Period1);
    EEPROM.put(250, (int)Period2);
    EEPROM.put(260, (int)Manual);
    EEPROM.commit();
    preferences.begin("storage", false);
    preferences.putInt("CountP1", 0);
    preferences.putInt("CountP1Min", 0);
    preferences.end();
  }

  bot1 = UniversalTelegramBot(String(Bot_Token_1), secured_client1);
  secured_client1.setCACert(TELEGRAM_CERTIFICATE_ROOT); 

  if (currentMode != AP_MODE && currentMode != FAST_MODE) {
    currentMode = AP_MODE; 
    EEPROM.put(500, (int)currentMode);
    EEPROM.commit();
  }

  if (currentMode == FAST_MODE) {
    iotWebConf.skipApStartup();
  }

  esp_task_wdt_init(30, true); 

  xTaskCreatePinnedToCore(
    Task1code,    /* Task function. */
    "Task1",      /* name of task. */
    8000,        /* Stack size of task */
    NULL,         /* parameter of the task */
    1,            /* priority of the task */
    &Task1,       /* Task handle to keep track of created task */
    0);           /* pin task to core 0 or 1  */
  vTaskDelay(200 / portTICK_PERIOD_MS);
  //------------------------------------------------------------------------
  //Task2 : Date-Time  Core :0
  xTaskCreatePinnedToCore(
    Task2code,    /* Task function. */
    "Task2",      /* name of task. */
    8000,        /* Stack size of task */
    NULL,         /* parameter of the task */
    1,            /* priority of the task */
    &Task2,       /* Task handle to keep track of created task */
    0);           /* pin task to core 0 or 1  */
  vTaskDelay(200 / portTICK_PERIOD_MS);
  xTaskCreatePinnedToCore(
    Task3code,    /* Task function. */
    "Task3",      /* name of task. */
    8000,        /* Stack size of task */
    NULL,         /* parameter of the task */
    1,            /* priority of the task */
    &Task3,       /* Task handle to keep track of created task */
    1);           /* pin task to core 1 */
  vTaskDelay(200 / portTICK_PERIOD_MS);

  xTaskCreatePinnedToCore(
    Task4code,    /* Task function. */
    "Task4",      /* name of task. */
    8000,        /* Stack size of task */
    NULL,         /* parameter of the task */
    1,            /* priority of the task */
    &Task4,       /* Task handle to keep track of created task */
    1);           /* pin task to core 0 or 1  */
  vTaskDelay(200 / portTICK_PERIOD_MS);
  xTaskCreatePinnedToCore(
    Task5code,    /* Task function. */
    "Task5",      /* name of task. */
    4000,        /* Stack size of task */
    NULL,         /* parameter of the task */
    1,            /* priority of task */
    &Task5,       /* Task handle to keep track of created task */
    0);           /* pin task to core 0 or 1  */
  vTaskDelay(200 / portTICK_PERIOD_MS);
  xTaskCreatePinnedToCore(
    Task6code,
    "Task6",
    6000,
    NULL,
    1,            /* priority  Task4 ( 0  worker ) */
    &Task6,
    1);
  vTaskDelay(200 / portTICK_PERIOD_MS);
  xTaskCreatePinnedToCore(
    Task8code,    /* Task function. */
    "Task8",      /* name of task. */
    4000,        /* Stack size of task ( 10,000 bytes) */
    NULL,         /* parameter of the task */
    1,            /* priority of the task ( Task  -  Round-Robin scheduling) */
    &Task8,       /* Task handle to keep track of created task */
    0);           /* pin task to core 0 (Core  Task1, Task2) */
  vTaskDelay(200 / portTICK_PERIOD_MS);

  esp_task_wdt_add(NULL); 
  esp_task_wdt_reset(); 

  Internet_Check_Mode = 0;
  consecutive_internet_success = 0;
  consecutive_internet_fail = 0;

  if (Sel_1_Blynk_Mode == 1) {
    Blynk.config(Blynk_Token_1, configblynk, 8080);
  }

  timer.setInterval(2000L, internetcheck);
  Serial.println("[BOOT] setup() COMPLETE - entering loop");
}

void loop() {
  timer.run();

  // --- RS485 TRANSPARENT BRIDGE ---
  // [FIX] Disabled because it steals Modbus ACK responses from Serial2 during the Health Heartbeat.
  /*
  if (Manual == 0 && Period1 == 0 && Period2 == 0) {
    while (Serial.available()) { Serial2.write(Serial.read()); }
    while (Serial2.available()) { Serial.write(Serial2.read()); }
  }
  */

  vTaskDelay(10 / portTICK_PERIOD_MS); // Reduced delay for better serial responsiveness
  esp_task_wdt_reset();
}


BLYNK_CONNECTED()
{
  if (Sel_1_Blynk_Mode == 1) {
    Serial.println("Blynk connected - ready for real-time communication");
    BlynkConnected = true;
    refreshWebConfigValues();
    Blynk.syncVirtual(V2, V3, V5);
    Blynk.syncVirtual(V12, V13, V6);
    Blynk.syncVirtual(V105, V106);
    Serial.println("System parameters synced from Blynk Cloud");

    if (currentMode == AP_MODE) {
      Blynk.virtualWrite(V121, "AP Mode");
    } else if (currentMode == FAST_MODE) {
      Blynk.virtualWrite(V121, "Fast Mode");
    } else {
      currentMode = AP_MODE;
      EEPROM.put(500, (int)currentMode);
      EEPROM.commit();
      Blynk.virtualWrite(V121, "AP Mode");
      Serial.println("⚠️ Invalid mode, defaulting to AP Mode");
    }
    Blynk.virtualWrite(V0, 0);
    Blynk.virtualWrite(V1, 0);   // Progress bar reset
  }
  // [FIX] BLYNK_WRITE_SAFE — WidgetLED.off() calls Blynk.virtualWrite() without mutex
  for (int i = 1; i <= 21; i++) {
    BLYNK_WRITE_SAFE(60 + i, 0);
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

BLYNK_DISCONNECTED()
{
  if (Sel_1_Blynk_Mode == 1) {
    Serial.println("Blynk disconnected");
    BlynkConnected = false;
  }
}

// New Manual Single Relay Logic (Using V105-V107 due to 7-bit limit)
BLYNK_WRITE(V105) { // Select Relay Number (1-21)
  manualRelayID = param.asInt();
}

BLYNK_WRITE(V106) { // Set Duration (Minutes)
  manualRelayDuration = param.asInt();
}

BLYNK_WRITE(V107) { // Start Manual Single
 
   if (param.asInt() == 1) {
    DEBUG_PRINTLN("[BLYNK] Manual Start Button (V107) Pressed");
    // Block Manual if Sch1 or Sch2 is running
    if (Period1 == 1 || Period2 == 1) {
      Serial.println("[BLYNK] Manual rejected - Schedule running");
      Blynk.virtualWrite(V31, "Manual blocked: Schedule active");
      return;
    }
    if (manualRelayDuration > 0 && manualRelayID >= 2 && manualRelayID <= 21) {
      
      if (Manual == 1) {
        // Save the relay that's actually ON, then switch to new one
        manualRelayID_prev = manualRelayActive;
        manualRelayActive = manualRelayID;
        manualSwitchFlag = true;
        Serial.printf("Manual SWITCH: SV%d -> SV%d\n", manualRelayID_prev, manualRelayID);
        return;
      }
      
      // Fresh start
      manualRelayActive = manualRelayID;
      manualRelayID_prev = manualRelayID;
      Serial.printf("Manual Start: SV%d for %d min (Pump SV1 Active)\n", manualRelayID, manualRelayDuration);
      stopInProgress = false; 
      stopRequested = false;
      Manual = 1; 
      Period1 = 0; 
      Period2 = 0; 
      CountP1Min = 0;
      CountP1 = 0;

      Manual_Timeduration_Min_int = manualRelayDuration;
      TA40 = manualRelayDuration; 
      BlynkRun_once1 = 1;
      EEPROM.put(240, (int)0);
      EEPROM.put(250, (int)0);
      EEPROM.put(220, Manual_Timeduration_Min_int);
      EEPROM.put(260, (int)Manual);
      esp_task_wdt_reset();
      EEPROM.commit();
      Blynk.virtualWrite(V4, "Manual START");
      Blynk.virtualWrite(V24, String(TA40) + " min");
      Blynk.virtualWrite(V1, 0); 
      Blynk.virtualWrite(V31, "Manual Single: SV" + String(manualRelayID) + " started");
      DEBUG_PRINTLN("[BLYNK] Manual Mode Started Successfully");
    } else {
      Serial.println("[DEBUG] Manual Start Rejected:");
      Serial.printf(" - Duration: %d (must be > 0)\n", manualRelayDuration);
      Serial.printf(" - Relay ID: %d (must be 2-21)\n", manualRelayID);
      Blynk.virtualWrite(V31, "Manual Error: SV ID (2-21) / Duration");
      DEBUG_PRINTLN("[BLYNK ERROR] Manual Start Rejected: Invalid ID or Duration");
    }
  }
}

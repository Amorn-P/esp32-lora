/*
  BLYNK VIRTUAL PINS
  V0: Stop (Hold 5s)
  V1: Run Status (Indicator)
  V2: P1 Start Hour
  V3: P1 Start Min
  V4: Status Label
  V5: P1 SV Duration
  V6: P2 SV Duration
  V7: P1 Total Display
  V8: P2 Total Display
  V12: P2 Start Hour
  V13: P2 Start Min
  V15: Time Source (0:RTC, 1:NTP)
  V16: Time Source Label
  V21: RTC Sync Label
  V24: Remaining Time
  V31: Notification Log
  V118-121: Bot Config & WiFi Mode
*/

#include <stdint.h>

// RS485 bus mutex (created in main.cpp, shared with Task3)
extern SemaphoreHandle_t g_rs485Mutex;
#define RS485_LOCK_TIMEOUT_MS 500  // Longer timeout for stop operations

extern WidgetLED led_sv1;  extern WidgetLED led_sv2;  extern WidgetLED led_sv3;
extern WidgetLED led_sv4;  extern WidgetLED led_sv5;  extern WidgetLED led_sv6;
extern WidgetLED led_sv7;  extern WidgetLED led_sv8;  extern WidgetLED led_sv9;
extern WidgetLED led_sv10; extern WidgetLED led_sv11; extern WidgetLED led_sv12;
extern WidgetLED led_sv13; extern WidgetLED led_sv14; extern WidgetLED led_sv15;
extern WidgetLED led_sv16; extern WidgetLED led_sv17; extern WidgetLED led_sv18;
extern WidgetLED led_sv19; extern WidgetLED led_sv20; extern WidgetLED led_sv21;

// stopRequested  13_Param.h (global)
extern volatile bool stopRequested;

// ==========================================================
// STOP VERIFY MODE
// ==========================================================
// 0 =  (write-only)  RS485/Modbus  Blynk 
// 1 = read+verify ( readHoldingRegister   SV17-SV21)
#define STOP_VERIFY_READBACK 0

// ==========================================================
// STOP + VERIFY (Modbus RTU)
// ==========================================================
// " Stop " :
// - / 
// -  RS485  OFF   ()  512 
// // :
// -  DFRobot_RTU  readHoldingRegister(slave, reg)
// signature 

struct RelayPoint {
  uint8_t slave;
  uint16_t reg;
  const char* name;
};

static const RelayPoint RELAYS_ALL[] = {
  {1, 1, "SV1"}, // Pump
  {2, 1, "SV2"}, {2, 2, "SV3"}, {2, 3, "SV4"}, {2, 4, "SV5"}, // Board 1
  {3, 1, "SV6"}, {3, 2, "SV7"}, {3, 3, "SV8"}, {3, 4, "SV9"}, {3, 5, "SV10"}, {3, 6, "SV11"}, // Board 2
  {4, 1, "SV12"}, {4, 2, "SV13"}, {4, 3, "SV14"}, {4, 4, "SV15"}, // Board 3
  {5, 1, "SV16"}, {5, 2, "SV17"}, {5, 3, "SV18"}, {5, 4, "SV19"}, {5, 5, "SV20"}, {5, 6, "SV21"} // Board 4
};

void forceAllRelaysOffOnce()
{
  // Acquire RS485 mutex for the entire stop sequence
  bool haveLock = (g_rs485Mutex != NULL && 
                   xSemaphoreTake(g_rs485Mutex, pdMS_TO_TICKS(RS485_LOCK_TIMEOUT_MS)) == pdTRUE);
  
  for (size_t i = 0; i < (sizeof(RELAYS_ALL) / sizeof(RELAYS_ALL[0])); i++) {
    // Retry up to 3 times per relay during stop (critical operation)
    for (int attempt = 0; attempt < 3; attempt++) {
      uint8_t ret = RTU_MASTER.writeHoldingRegister(RELAYS_ALL[i].slave, RELAYS_ALL[i].reg, 512);
      if (ret == 0) break;
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    vTaskDelay(20 / portTICK_PERIOD_MS);
    esp_task_wdt_reset();
  }
  
  if (haveLock) xSemaphoreGive(g_rs485Mutex);
}

void verifyRelaysOffAndAutoFix(bool reportToBlynk)
{
#if STOP_VERIFY_READBACK == 0
  const int ROUNDS = 3;
  for (int r = 1; r <= ROUNDS; r++) {
    for (size_t i = 0; i < (sizeof(RELAYS_ALL) / sizeof(RELAYS_ALL[0])); i++) {
      for (int a = 0; a < 2; a++) {
        uint8_t rret = RTU_MASTER.writeHoldingRegister(RELAYS_ALL[i].slave, RELAYS_ALL[i].reg, 512);
        if (rret == 0) break;
        vTaskDelay(10 / portTICK_PERIOD_MS);
      }
      vTaskDelay(20 / portTICK_PERIOD_MS);
      esp_task_wdt_reset();
    }
    if (reportToBlynk) {
      Blynk.virtualWrite(V31, "STOP OFF ROUND " + String(r) + "/" + String(ROUNDS));
      vTaskDelay(30 / portTICK_PERIOD_MS);
    }
  }
  if (reportToBlynk) {
    Blynk.virtualWrite(V31, "Cleared and Stopped");
  }
  return;
#else
  const int MAX_ATTEMPTS = 5;
  int totalRetried = 0;
  int finalStillOn = 0;
  int finalReadFail = 0;

  for (int attempt = 1; attempt <= MAX_ATTEMPTS; attempt++) {
    int readFail = 0;
    int retried = 0;
    int stillOn = 0;

    for (size_t i = 0; i < (sizeof(RELAYS_ALL) / sizeof(RELAYS_ALL[0])); i++) {
      int val = RTU_MASTER.readHoldingRegister(RELAYS_ALL[i].slave, RELAYS_ALL[i].reg);
      vTaskDelay(20 / portTICK_PERIOD_MS);
      esp_task_wdt_reset();

      if (val < 0) {
        readFail++;
        RTU_MASTER.writeHoldingRegister(RELAYS_ALL[i].slave, RELAYS_ALL[i].reg, 512);
        vTaskDelay(20 / portTICK_PERIOD_MS);
        continue;
      }

      if (val == 256) {
        retried++;
        RTU_MASTER.writeHoldingRegister(RELAYS_ALL[i].slave, RELAYS_ALL[i].reg, 512);
        vTaskDelay(20 / portTICK_PERIOD_MS);
      }
    }

    for (size_t i = 0; i < (sizeof(RELAYS_ALL) / sizeof(RELAYS_ALL[0])); i++) {
      int val = RTU_MASTER.readHoldingRegister(RELAYS_ALL[i].slave, RELAYS_ALL[i].reg);
      vTaskDelay(20 / portTICK_PERIOD_MS);
      esp_task_wdt_reset();

      if (val < 0) {
        readFail++;
        continue;
      }
      if (val == 256) {
        stillOn++;
      }
    }

    totalRetried += retried;
    finalStillOn = stillOn;
    finalReadFail = readFail;

    if (reportToBlynk) {
      String progress = "STOP VERIFY [" + String(attempt) + "/" + String(MAX_ATTEMPTS) + "]"
                        " retry=" + String(retried) +
                        " stillOn=" + String(stillOn) +
                        " readFail=" + String(readFail);
      Blynk.virtualWrite(V31, progress);
      vTaskDelay(50 / portTICK_PERIOD_MS);
      esp_task_wdt_reset();
    }

    if (stillOn == 0 && readFail == 0) {
      break;
    }

    vTaskDelay(150 / portTICK_PERIOD_MS);
    esp_task_wdt_reset();
  }

  if (reportToBlynk) {
    if (finalStillOn == 0 && finalReadFail == 0) {
      Blynk.virtualWrite(V31, "Cleared and Stopped");
    } else {
      Blynk.virtualWrite(
        V31,
        "STOP WARN: not fully cleared | stillOn=" + String(finalStillOn) +
        " | readFail=" + String(finalReadFail)
      );
    }
  }
#endif
}

void stopSystemAndVerifyAll(bool reportToBlynk)
{
  stopInProgress = true;
  vTaskDelay(100 / portTICK_PERIOD_MS);
  esp_task_wdt_reset();

  forceAllRelaysOffOnce();
  esp_task_wdt_reset();

  verifyRelaysOffAndAutoFix(reportToBlynk);
  esp_task_wdt_reset();

  Manual = 0;
  Period1 = 0;
  Period2 = 0;
  CountP1Min = 0;
  CountP1 = 0;
  TA40 = 0;
  BlynkRun_once1 = 0;

  {
    int zero = 0;
    EEPROM.put(240, zero);
    EEPROM.put(250, zero);
    EEPROM.put(260, zero);
    esp_task_wdt_reset();
    EEPROM.commit();
  }

  if (preferencesMutex != NULL && xSemaphoreTake(preferencesMutex, pdMS_TO_TICKS(2000)) == pdTRUE) {
    preferences.begin("storage", false);
    preferences.putInt("CountP1", 0);
    preferences.putInt("CountP1Min", 0);
    preferences.end();
    xSemaphoreGive(preferencesMutex);
  }

  if (reportToBlynk) {
    Blynk.virtualWrite(V4, "STOPPING...");
    vTaskDelay(20 / portTICK_PERIOD_MS);
    esp_task_wdt_reset();
    
    Blynk.virtualWrite(V24, "0 min");
    vTaskDelay(20 / portTICK_PERIOD_MS);
    esp_task_wdt_reset();
    
    Blynk.virtualWrite(V1, 0);
    vTaskDelay(20 / portTICK_PERIOD_MS);
    esp_task_wdt_reset();
  }

  for (int i = 1; i <= 21; i++) {
    BLYNK_WRITE_SAFE(60 + i, 0);
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
  esp_task_wdt_reset();

  stopInProgress = false;
}

BLYNK_WRITE(V0)
{
  int pinValue = param.asInt();
  if (pinValue == 1) {
    stopInProgress = true;
    stopRequested = true;
    Blynk.virtualWrite(V31, "STOP REQ: received");
  }
}

BLYNK_WRITE(V2)
{
  Period1_Time_Start_Hr_int = param.asInt();
  Blynk.virtualWrite(V31, "P1 Hour: " + String(Period1_Time_Start_Hr_int) + " saving...");
  EEPROM.put(150, Period1_Time_Start_Hr_int);
  esp_task_wdt_reset();
  EEPROM.commit();
}

BLYNK_WRITE(V3)
{
  Period1_Time_Start_Min_int = param.asInt();
  Blynk.virtualWrite(V31, "P1 Min: " + String(Period1_Time_Start_Min_int) + " saving...");
  EEPROM.put(160, Period1_Time_Start_Min_int);
  esp_task_wdt_reset();
  EEPROM.commit();
}

BLYNK_WRITE(V5)
{
  Period1_Timeduration_Min_int = param.asInt();
  Blynk.virtualWrite(V31, "P1 Dur: " + String(Period1_Timeduration_Min_int) + " saving...");
  EEPROM.put(200, Period1_Timeduration_Min_int);
  esp_task_wdt_reset();
  EEPROM.commit();
  Time_Count_Period1();
  Blynk.virtualWrite(V7, "Total: " + String(hours1) + " hr " + String(minutes1) + " min (" + String(TA40_P1) + " min)");
}

BLYNK_WRITE(V6)
{
  Period2_Timeduration_Min_int = param.asInt();
  EEPROM.put(210, Period2_Timeduration_Min_int);
  esp_task_wdt_reset();
  EEPROM.commit();
  Blynk.virtualWrite(V31, "P2 Dur: " + String(Period2_Timeduration_Min_int) + " saved!");
  Time_Count_Period2();
  Blynk.virtualWrite(V8, "Total: " + String(hours2) + " hr " + String(minutes2) + " min (" + String(TA40_P2) + " min)");
}

BLYNK_WRITE(V10)
{
  int pinValue = param.asInt();
  if (pinValue == 1) {
    // Sync  NTP  RTC Module
    rtc.setTime(THour, Tmin, Tsec + 1);
    rtc.setDate(Day, Month, Year_1);
    
    // RTC 
    t = rtc.getTime();
    
    // Sync  RTC  V21
    Blynk.virtualWrite(V21, "RTC Sync Time :    " + String(t.hour) + ":" + String(t.min) + ":" + String(t.sec + 1));
  }
}

// ===============================================
// V12 - Period2 Start Hour (Input)
// Period2 ()  Blynk  ESP32
// ===============================================
BLYNK_WRITE(V12)
{
  // Period2  Blynk
  Period2_Time_Start_Hr_int = param.asInt();
  
  // EEPROM (Address 170)
  EEPROM.put(170, Period2_Time_Start_Hr_int);
  esp_task_wdt_reset();  // [FIX]  WDT  EEPROM.commit()
  EEPROM.commit();
  Period2_Time_Start_Hr_int = EEPROM.get(170, Period2_Time_Start_Hr_int);
  
  // V31
  Blynk.virtualWrite(V31, "Period2 Hour = " + String(Period2_Time_Start_Hr_int) + " --> saved!");
}

// ===============================================
// V13 - Period2 Start Minute (Input)
// Period2 ()  Blynk  ESP32
// ===============================================
BLYNK_WRITE(V13)
{
  // Period2  Blynk
  Period2_Time_Start_Min_int = param.asInt();
  
  // EEPROM (Address 180)
  EEPROM.put(180, Period2_Time_Start_Min_int);
  esp_task_wdt_reset();  // [FIX]  WDT  EEPROM.commit()
  EEPROM.commit();
  Period2_Time_Start_Min_int = EEPROM.get(180, Period2_Time_Start_Min_int);
  
  // V31
  Blynk.virtualWrite(V31, "Period2 Min = " + String(Period2_Time_Start_Min_int) + " --> saved!");
}

// ===============================================
// V15 - Time Source Selection (Switch)
// RTC  NTP
// 0 = RTC Module, 1 = NTP Server
// V16
// ===============================================
BLYNK_WRITE(V15)
{
  int pinValue = param.asInt();
  if (pinValue == 1) {
    // NTP Server
    Sys_Time_Select = 1;
    EEPROM.put(230, Sys_Time_Select);
    EEPROM.commit();
    Blynk.virtualWrite(V16, "  NTP Server");
  } else {
    // RTC Module
    Sys_Time_Select = 0;
    EEPROM.put(230, Sys_Time_Select);
    EEPROM.commit();
    Blynk.virtualWrite(V16, "  RTC Module");
  }
}



// ===============================================
// V118 - Bot Token (Input)
// Bot Token  Blynk  ESP32
// ===============================================
BLYNK_WRITE(V118)
{
  Blynk_Bot_Token = param.asString();           // Blynk   String
  Blynk_Bot_Token.toCharArray(Bot_Token_1, 60); // String  Charecter  Bot_Token_1
  EEPROM.put(0, Bot_Token_1);                   // Bot_Token_1  Charecter  Address 0
  esp_task_wdt_reset();                         // watchdog  EEPROM.commit() 
  EEPROM.commit();                              // EEPROM.get(0, Bot_Token_1);                   // Bot_Token_1  Charecter  Address 0 
  Serial.print("Bot_Token_1 :");
  Serial.println(Bot_Token_1);
}

// ===============================================
// V119 - Bot ID Group (Input)
// Bot ID Group  Blynk  ESP32
// ===============================================
BLYNK_WRITE(V119)
{
  Blynk_Bot_Group = param.asString();         // Blynk   String
  Blynk_Bot_Group.toCharArray(Bot_Group, 20); // String  Charecter  Bot_Group
  EEPROM.put(70, Bot_Group);                  // Bot_Group  Charecter  Address 70
  esp_task_wdt_reset();                       // watchdog  EEPROM.commit() 
  EEPROM.commit();                            // Bot_Group
  EEPROM.get(70, Bot_Group);                  // Bot_Group  Charecter  Address 70 
  Serial.print("Bot_Group :");
  Serial.println(Bot_Group);
}

// ===============================================
// V120 - Mode Selection (Button)
// AP  Fast Mode direct WiFi  Blynk
// ===============================================
BLYNK_WRITE(V120)
{
  int pinValue = param.asInt();
  if (pinValue == 1)
  {
    // currentMode = (currentMode == AP_MODE) ? FAST_MODE : AP_MODE;

    // EEPROM
    EEPROM.put(500, currentMode);
    esp_task_wdt_reset(); // watchdog  EEPROM.commit() 
    EEPROM.commit();

    // LED
    digitalWrite(STATUS_PIN, currentMode == FAST_MODE ? HIGH : LOW);

    // Serial.print(": ");
    Serial.println(currentMode == AP_MODE ? "AP Mode" : "Fast Mode");
    Blynk.virtualWrite(V121, currentMode == AP_MODE ? "AP Mode" : "Fast Mode");

    // 1  ()
    Blynk.syncVirtual(V121);

    esp_task_wdt_reset();  // [FIX]  WDT  delay
    vTaskDelay(2000 / portTICK_PERIOD_MS);  // [FIX]  delay() blocking → vTaskDelay (non-blocking)
    // ESP
    ESP.restart();
  }
  if (pinValue == 0)
  {
  }
}

// ===============================================
// WEB CONFIG REFRESH FUNCTION
// ===============================================

// ===============================================
// refreshWebConfigValues()
// Config  Blynk Virtual Pins
// ===============================================
void refreshWebConfigValues()
{
  // Local Mode  ( Local Mode )
  if (Sel_1_Mode_Local_Or_Blynk == 1)
  {
    Serial.println("⚠️ refreshWebConfigValues() ignored - Local Mode active");
    return; // Local Mode
  }

  // ===============================================
  // WiFi Settings (V90-V91)
  // ===============================================
  Blynk.virtualWrite(V90, String(iotWebConf.getWifiSsidParameter()->valueBuffer));
  Blynk.virtualWrite(V91, String(iotWebConf.getWifiPasswordParameter()->valueBuffer));

  // ===============================================
  // Static IP Configuration (V92-V97)
  // ===============================================
  // Static IP Checkbox Status
  Blynk.virtualWrite(V92, (Sel_1_SelIP_Sys == 1) ? "Enabled" : "Disabled");

  // Static IP Settings
  Blynk.virtualWrite(V93, String(ipAddressValue));
  Blynk.virtualWrite(V94, String(gatewayValue));
  Blynk.virtualWrite(V95, String(netmaskValue));
  Blynk.virtualWrite(V96, String(primaryDNSValue));
  Blynk.virtualWrite(V97, String(secondaryDNSValue));

  // ===============================================
  // Blynk Configuration (V98-V100)
  // ===============================================
  // Blynk Mode Checkbox Status
  Blynk.virtualWrite(V98, (Sel_1_Blynk_Mode == 1) ? "Enabled" : "Disabled");

  // Blynk Settings
  Blynk.virtualWrite(V99, String(Blynk_Token_1));
  Blynk.virtualWrite(V100, String(configblynk));

  // ===============================================
  // System Status (V101-V102)
  // ===============================================
  // Apply & Restart Button Status
  Blynk.virtualWrite(V101, "Ready to Apply & Restart");

  // Current IP Address -  IP Address  (Dynamic  Static)
  Blynk.virtualWrite(V102, WiFi.localIP().toString());
}
//------------------------------------------------------

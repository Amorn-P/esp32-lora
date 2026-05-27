================================================================================
CommCore - Reusable ESP32 Communication Framework
================================================================================
Version: 1.0.0 (extracted from CherryOne_Copy v1.2.0)

PURPOSE
  Copy CommCore/ into any ESP32 PlatformIO project for instant:
  - WiFi Manager (IotWebConf AP + STA, DHCP + Static IP)
  - Internet connectivity check
  - Blynk Legacy (port 8080, no SSL)
  - Telegram Bot notifications
  - OTA firmware update
  - Web config portal

FILE STRUCTURE
  CommCore/
    CommCore.h              Public API (include this only)
    CommCore_Params.h       All shared globals, externs, includes
    CommCore_WiFi.h         IotWebConf setup, connectWifi, internetcheck, static_ip
    CommCore_Web.h          Web routes, handleRoot, configSaved, WiFi scan
    CommCore_Blynk.h        Blynk system V-pins (V90-V127), connect/disconnect
    CommCore_Telegram.h     Bot setup, TLS cert, sendMessage wrapper
    CommCore_OTA.h          OTA update via Blynk V123-V124
    CommCore_Tasks.h        Task1 (IoT loop), Task4 (Blynk run)
  
  Project-specific (your app):
    src/
      User_config.h         Credentials (WiFi, Blynk, Telegram, AP name)
      Pins_config.h         GPIO pins
      12_List_Wf.h          IotWebConf parameter definitions
      01_Social_Handlers.h  Telegram/Reset web handlers
      00_Blynk_App.h        Project Blynk V-pins (V0-V81)
      App_Tasks.h           Your FreeRTOS tasks (Task2, Task3, Task5-8)
      main.cpp              Setup + loop

QUICK START (new project)
  1. Copy CommCore/ folder to your project
  2. Copy src/User_config.h, src/12_List_Wf.h, src/01_Social_Handlers.h
  3. Create src/00_Blynk_App.h for your V-pins
  4. In main.cpp:
     #include "CommCore/CommCore.h"
     #include "src/12_List_Wf.h"
     #include "src/01_Social_Handlers.h"
     #include "src/00_Blynk_App.h"
     
     void setup() {
       Serial.begin(115200);
       pinMode(LED_INTERNET, OUTPUT);
       static_ip();                // Register WiFi handlers
       Converse_value();            // Load EEPROM
       Iotwencof_start();           // Start IotWebConf
       needReset = false;
       CommCore_telegramSetup();    // Init Telegram
       
       // Create CommCore tasks
       xTaskCreatePinnedToCore(Task1_CommCore, "T1", 8000, NULL, 1, &CommTask1, 0);
       xTaskCreatePinnedToCore(Task4_CommCore, "T4", 8000, NULL, 1, &CommTask4, 1);
       
       // Create your app tasks...
       
       if (Sel_1_Blynk_Mode == 1) {
         Blynk.config(Blynk_Token_1, configblynk, 8080);
       }
       timer.setInterval(2000L, internetcheck);
     }
     
     void loop() {
       timer.run();
       vTaskDelay(10 / portTICK_PERIOD_MS);
       esp_task_wdt_reset();
     }

V-PIN ALLOCATION
  System (CommCore):        V90-V127  (WiFi, Blynk, Telegram, OTA, Reset)
  Available for projects:   V0-V89    (Your app's widgets)
  LED indicators:           V61-V81   (21 WidgetLEDs, reusable)
  Reserved:                 V102, V121, V122

CRITICAL RULES
  1. NEVER call EEPROM.begin() before IotWebConf.init()
     (causes NVS corruption on Core 1.0.6)
  2. ALWAYS use BLYNK_WRITE_SAFE() for cross-core Blynk writes
  3. ALWAYS use preferencesMutex for Preferences access
  4. Blynk Legacy server 43.229.135.169:8080 does NOT use SSL
  5. configSaved() MUST have 60s boot guard + 3s delay
  6. Platform MUST be espressif32@3.5.0 (Core 1.0.6)
  7. Use DHCP (Static IP DNS unreliable on some networks)

WEAKNESSES ADDRESSED (compared to original CherryOne_Copy)
  [FIXED] 13_Param.h was 448-line monolith -> CommCore_Params.h + project params
  [FIXED] WiFi fallback duplicated -> single source in CommCore_WiFi.h
  [FIXED] Blynk pins mixed -> CommCore_Blynk.h (system) + 00_Blynk_App.h (project)
  [FIXED] connectWifi/static_ip scattered -> CommCore_WiFi.h unified
  [FIXED] OTA code duplicated -> CommCore_OTA.h standalone
  [FIXED] Task1/Task4 definitions mixed -> CommCore_Tasks.h
  [FIXED] Telegram setup scattered -> CommCore_Telegram.h
  [FIXED] Global variables unorganized -> CommCore_Params.h with clear externs
================================================================================

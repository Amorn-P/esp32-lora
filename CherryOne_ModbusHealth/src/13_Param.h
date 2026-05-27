/**
   ===============================================
   13_Param.h - Global Variables & Parameters
   ===============================================
   
    Global  Parameters 
    
   
   Features:
   - System Control Variables
   - WiFi & Network Configuration
   - Blynk Configuration
   - Telegram Bot Configuration
   - RTC & NTP Time Configuration
   - Pump Control Variables (Period1, Period2, Manual)
   - Modbus RTU Configuration
   - Preferences (Non-volatile Storage)
   
   ===============================================
**/

#include "14_Bot_massage.h"

// ==========================================================
// SYSTEM CONTROL VARIABLES
// ==========================================================
int Notify1 = 0;           // // ==========================================================
// PREFERENCES LIBRARY (Non-volatile Storage)
// ==========================================================
// (Flash)
// /ESP32 Restart  Watchdog Timer
// : 
#include <Preferences.h>
Preferences preferences;  // Preferences

// ==========================================================
// TIMER LIBRARY
// ==========================================================
#include <SimpleTimer.h>
SimpleTimer timer;

// ==========================================================
// MODBUS RTU MASTER CONFIGURATION
// ==========================================================
// Pump SV  Modbus RTU Protocol
#include "DFRobot_RTU.h"  // MASTER Only
#define RXD2 16           // GPIO16 - Serial2 RX pin
#define TXD2 17           // GPIO17 - Serial2 TX pin

DFRobot_RTU RTU_MASTER(/*s =*/&Serial2);  // Modbus RTU Master instance

// ==========================================================
// WATCHDOG TIMER & RTC CONFIGURATION
// ==========================================================
#include "esp_task_wdt.h"  // ESP32 Watchdog Timer
#include <DS1302.h>        // DS1302 RTC Module Library

// RTC Module Pin Configuration
// DS1302(CE, IO, SCLK) = DS1302(15, 4, 5)
DS1302 rtc(15, 4, 5);  // GPIO15=CE, GPIO4=IO, GPIO5=SCLK
Time t;                 // Time structure  RTC

// ==========================================================
// RTC TIME SYNCHRONIZATION CONTROL
// ==========================================================
// DS1302 RTC  ESP32  NTP Server 
// 1 
int rtcUpdated = 0;  // (1=, 0=)

// update  Blynk ( Sync )
int TimeSync_OK1 = 0;  // Sync  1
int TimeSync_OK2 = 0;  // Sync  2
int TimeSync_OK3 = 0;  // Sync  3

// ==========================================================
// FREERTOS MUTEX -  Race Condition
// ==========================================================
// [FIX] Mutex  Preferences.h (Task3  Task5 )
// setup()  Task  xSemaphoreCreateMutex()
SemaphoreHandle_t preferencesMutex = NULL;

// [FIX] Mutex  Blynk.virtualWrite()  Core 0 (Task2)
// Blynk.run()  Task4 Core 1 -  buffer corrupt  Core
SemaphoreHandle_t blynkMutex = NULL;

// Helper macro: Blynk write thread-safe (timeout 100ms)
// Blynk.virtualWrite()  Task4
#define BLYNK_WRITE_SAFE(pin, val) \
  do { \
    if (blynkMutex != NULL && xSemaphoreTake(blynkMutex, pdMS_TO_TICKS(100)) == pdTRUE) { \
      Blynk.virtualWrite(pin, val); \
      xSemaphoreGive(blynkMutex); \
    } \
  } while(0)

// ==========================================================
// PUMP CONTROL VARIABLES
// ==========================================================
// [FIX] volatile:  compiler  RAM 
// register caching  Core  SMP (Core0/Core1)
volatile int Period1 = 0;  // Period1 (1=, 0=)
volatile int Period2 = 0;  // Period2 (1=, 0=)
volatile int Manual = 0;   // Manual (1=, 0=)

// ==========================================================
// PUMP TIMING CONTROL ( Preferences.h)
// ==========================================================
// [FIX] volatile: CountP1Min  Task5 (Core0)  Task3 (Core1)
volatile int CountP1 = 0;
volatile int CountP1Min = 0;  // ( Preferences)

// ==========================================================
// PERIOD STATUS TRACKING
// ==========================================================
// Period/Manual
// Telegram notification 
int prevPeriod1 = -1;  // Period1
int prevPeriod2 = -1;  // Period2
int prevManual  = -1;  // Manual



// ==========================================================
// TIME CALCULATION VARIABLES
// ==========================================================
// [FIX] volatile: TA40  Task2/Blynk callbacks  Task3  Core
volatile int TA1, TA2, TA3, TA4, TA5, TA6, TA7, TA8, TA9, TA10,
    TA11, TA12, TA13, TA14, TA15, TA16, TA17, TA18, TA19, TA20,
    TA21, TA22, TA23, TA24, TA25, TA26, TA27, TA28, TA29, TA30,
    TA31, TA32, TA33, TA34, TA35, TA36, TA37, TA38, TA39, TA40;
// [FIX] Separate variables to store totals for each mode
volatile int TA40_P1, TA40_P2, TA40_Man;

// ==========================================================
// PERIOD1 TIME CALCULATION
// ==========================================================
// Period1
int totalMinutes1;
int hours1;         // 60 
int minutes1;       // % 60 

// ==========================================================
// PERIOD2 TIME CALCULATION
// ==========================================================
// Period2
int totalMinutes2;
int hours2;         // 60 
int minutes2;       // % 60 

// ==========================================================
// MANUAL TIME CALCULATION
// ==========================================================
// Manual Mode
int totalMinutes3;
int hours3;         // 60 
int minutes3;       // % 60 

// ==========================================================
// SYSTEM STATUS VARIABLES
// ==========================================================
bool isCleared = false;
int Sys_Time_Select = 0;    // (0=RTC, 1=NTP) [FIX]  = RTC

// ==========================================================
// PERIOD1 CONFIGURATION ( Blynk)
// ==========================================================
int Period1_Time_Start_Hr_int  = 0;   // Period1 () [FIX]  = 0
int Period1_Time_Start_Min_int = 0;   // Period1 () [FIX]  = 0
int Period1_Timeduration_Min_int = 0; // SV  Period1 () [FIX]  = 0

// ==========================================================
// PERIOD2 CONFIGURATION ( Blynk)
// ==========================================================
int Period2_Time_Start_Hr_int  = 0;   // Period2 () [FIX]  = 0
int Period2_Time_Start_Min_int = 0;   // Period2 () [FIX]  = 0
int Period2_Timeduration_Min_int = 0; // SV  Period2 () [FIX]  = 0

// ==========================================================
// MANUAL CONFIGURATION ( Blynk)
// ==========================================================
int Manual_Timeduration_Min_int = 0;  // SV  Manual Mode () [FIX]  = 0

// ==========================================================
// WIFI AP/DIRECT MODE CONFIGURATION
// ==========================================================
#include <Bounce2.h>

// Button Configuration
#define BUTTON_0 0  // GPIO0 - Mode switching button
Bounce debouncer0 = Bounce();
// Button LCD MODE - DISABLED
//#define BUTTON_1 12  // GPIO12 - LCD Page switching button
//Bounce debouncer1 = Bounce();

bool systemInitialized = false; 
const int AP_MODE = 0;     // Access Point Mode ()
const int FAST_MODE = 1;   // Fast Mode ( WiFi )

// Current System Mode
int currentMode = AP_MODE;  // AP mode
// ==========================================================

// ==========================================================
// STOP CONTROL (Global)
// ==========================================================
// flag  Task3  ON  Stop
// -  Stop  true
// -  Stop /  false
volatile bool stopInProgress = false;


// ==========================================================
// INTERNET CONNECTION MANAGEMENT
// ==========================================================

// Internet Connection Status
int Internet = 0;                // [FIX]  = 0 ()
int Internet_Reset = 0;          // Telegram  (1=, 0=)

// Dynamic Internet Check System
int Internet_Check_Mode = 0;     // (0=Fast 2s, 1=Normal 6s, 2=Slow 10s)
int consecutive_internet_success = 0;
int consecutive_internet_fail = 0;
// ==========================================================

// ==========================================================
// LED STATUS INDICATORS
// ==========================================================
#define LED_Internet 26  // GPIO26  - LED 
//#define LED_FA_Mode 12  // GPIO12 - LED  AP/FAST DIRECT
// ==========================================================


// ==========================================================
// WIFI NETWORK SCANNING
// ==========================================================
int numberOfNetworks;  // WiFi 
bool wifiScanInProgress = false;  // WiFi 
bool wifiScanComplete = false;    // WiFi 

// ==========================================================
// IP CONFIGURATION MANAGEMENT
// ==========================================================
String Sel_SelIP_Sys;  // IP Mode
int Sel_1_SelIP_Sys;   // IP Mode (1=Static, 0=Dynamic)

// ==========================================================
// SYSTEM RESET MANAGEMENT
// ==========================================================
bool needReset = false;  // ESP32  Apply  Web Config
                         // extern  00_Blynk.h


// ==========================================================
// STATIC IP CONFIGURATION
// ==========================================================
IPAddress ipAddress;      // Static IP Address
IPAddress gateway;        // Gateway Address
IPAddress netmask;        // Subnet Mask
IPAddress primaryDNS;     // Primary DNS Server
IPAddress secondaryDNS;   // Secondary DNS Server

// ==========================================================
// EEPROM LIBRARIES
// ==========================================================
#include <EEPROM.h>  // Address 0-512 || Address >512  IoTWebConf

// ==========================================================
// TELEGRAM BOT CONFIGURATION
// ==========================================================

#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <cstring>  // strcpy  strncpy

#include <WiFiClientSecure.h>
#include <HTTPClient.h>
//#include <Update.h>  // OTA Update firmware by Telegram ( IoT WebConf )

// Telegram Bot Configuration
WiFiClientSecure secured_client1;                    // Secure client  Telegram Bot
UniversalTelegramBot bot1("", secured_client1);      // Telegram Bot instance ()
String message;                                       // Telegram
//String project_name = "Master project";                  // String chat_id;                                       // Chat ID 

// ==========================================================
// WIFI CONFIGURATION VARIABLES
// ==========================================================
// WiFi  Telegram Notify

// Access Point Configuration
String SSID_AP;        // Access Point
String PASSWORD_AP;    // Access Point

// WiFi Network Configuration
String SSID_NAME;      // WiFi
String PASSWORD_NAME;  // WiFi

// Network Information
String IP;             // IP Address
String Sub_M;          // Subnet Mask
String Gate_way;       // Gateway Address



// ==========================================================
// NTP TIME SERVER CONFIGURATION
// ==========================================================
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <TimeLib.h>

// NTP Client Configuration
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "asia.pool.ntp.org", 25200, 60000);  // 25200 = UTC+7 ()
int NTP_connect = 0;  // NTP Server

// Time Variables
int Year;      // ..
int Year_1;    // .. ( ..)
byte last_second, Tsec, Tmin, THour, Day, Month;  // // ==========================================================
// BLYNK LIBRARY CONFIGURATION
// ==========================================================

#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>        // OTA Update
#include <Update.h>            // OTA Update
#include <WiFiClient.h>        // OTA Update

/*
   Blynk Library 
  Blynk Library 0.6.1 ():
  -  7-bit addressing  Virtual Pins
  - 7-bit = 2^7 = 128 values (0-127)
  -  protocol 
*/
// Blynk Library Optimization Settings
// Heartbeat  ( reconnect)
#ifndef BLYNK_HEARTBEAT
#define BLYNK_HEARTBEAT      300  // 100  300 
#endif

// Timeout 
#ifndef BLYNK_TIMEOUT_MS
#define BLYNK_TIMEOUT_MS     10000UL  // 6000  10000ms
#endif

// Rate Limit ( 15  30 /)
#ifndef BLYNK_MSG_LIMIT
#define BLYNK_MSG_LIMIT      30
#endif

// Buffer Size
#ifndef BLYNK_MAX_READBYTES
#define BLYNK_MAX_READBYTES  512  // 256  512
#endif

#ifndef BLYNK_MAX_SENDBYTES
#define BLYNK_MAX_SENDBYTES  256  // 128  256
#endif

// Performance Optimization
#define BLYNK_NO_DEBUG        // Debug  overhead
#define BLYNK_SEND_ATOMIC     // Atomic Send
#define BLYNK_NO_FANCY_LOGO   // Fancy Logo

// SSL Configuration - DISABLED for Blynk Legacy (port 8080 no SSL)
//#define BLYNK_USE_SSL         // SSL
//#define BLYNK_SSL_INSECURE    // SSL  ()

// Force Data Sync
#define BLYNK_FORCE_SYNC      // Widget

// ==========================================================
// BLYNK CONTROL CONFIGURATION
// ==========================================================

String Sel_Blynk_Mode;        // Blynk Mode
int Sel_1_Blynk_Mode;         // Blynk Mode (1=, 0=)
bool BlynkConnected = false;  // Blynk
String URL_Firmware = "";      // URL  OTA Update
                               // extern  00_Blynk.h

// Local/Blynk Mode Selection Configuration
String Sel_Mode_Local_Or_Blynk;  // Mode (Local/Blynk)
int Sel_1_Mode_Local_Or_Blynk;   // Mode (1=Local, 0=Blynk)

// Telegram Bot Configuration ( Blynk)
String Blynk_Bot_Token;  // Bot Token (String format)
char Bot_Token_1[60];    // Bot Token (Character array format)

String Blynk_Bot_Group;  // Bot Group ID (String format)
char Bot_Group[20];      // Bot Group ID (Character array format)

// System Reset Time Configuration ( Blynk)
String BlynkRst_Hr_St1;   // Reset Hour (String format)
int BlynkRst_Hr_int = 99; // Reset Hour (Integer format) [FIX] 99 =  ( garbage  EEPROM )

String BlynkRst_Min_St1;   // Reset Minute (String format)
int BlynkRst_Min_int = 99; // Reset Minute (Integer format) [FIX] 99 =  ( garbage  EEPROM )

// ==========================================================
// BLYNK LED STATUS INDICATORS (V61-V81)
// ==========================================================
// WidgetLED  Pump SV 
// V61-V81 = LED indicators  SV1-SV21
WidgetLED led_sv1(V61);   // LED  SV1
WidgetLED led_sv2(V62);   // LED  SV2
WidgetLED led_sv3(V63);   // LED  SV3
WidgetLED led_sv4(V64);   // LED  SV4
WidgetLED led_sv5(V65);   // LED  SV5
WidgetLED led_sv6(V66);   // LED  SV6
WidgetLED led_sv7(V67);   // LED  SV7
WidgetLED led_sv8(V68);   // LED  SV8
WidgetLED led_sv9(V69);   // LED  SV9
WidgetLED led_sv10(V70);  // LED  SV10
WidgetLED led_sv11(V71);  // LED  SV11
WidgetLED led_sv12(V72);  // LED  SV12
WidgetLED led_sv13(V73);  // LED  SV13
WidgetLED led_sv14(V74);  // LED  SV14
WidgetLED led_sv15(V75);  // LED  SV15
WidgetLED led_sv16(V76);  // LED  SV16
WidgetLED led_sv17(V77);  // LED  SV17
WidgetLED led_sv18(V78);  // LED  SV18
WidgetLED led_sv19(V79);  // LED  SV19
WidgetLED led_sv20(V80);  // LED  SV20
WidgetLED led_sv21(V81);  // LED  SV21

// ==========================================================
// BLYNK RUN CONTROL
// ==========================================================
volatile int BlynkRun_once1 = 0;  // [FIX] volatile:  Task4/Blynk,  Task2

// ==========================================================
// TELEGRAM NOTIFICATION CONTROL
// ==========================================================
// Telegram notification 

// ==========================================================
// STOP WORKER (Task6) CONTROL
// ==========================================================
// non-blocking ( BLYNK_WRITE(V0)  Task6 )
volatile bool stopRequested = false;
// 1  ( Time_run_once )

int Time_run_once1 = 0;  // Telegram notification  Period1
int Time_run_once2 = 0;  // Telegram notification  Period2
int Time_run_once3 = 0;  // Telegram notification  Manual

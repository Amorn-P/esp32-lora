/**
 * CommCore_Params.h - Communication Core Global Parameters
 * 
 * NOT project-specific. WiFi, Blynk, Telegram, NTP, System variables.
 * For project-specific params (relays, pumps, Modbus), use your own params file.
 */

#ifndef COMMCORE_PARAMS_H
#define COMMCORE_PARAMS_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Update.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <Preferences.h>
#include <SimpleTimer.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <TimeLib.h>
#include <esp_task_wdt.h>

#include <IotWebConf.h>
#include <IotWebConfUsing.h>
#include <IotWebConfOptionalGroup.h>
#include <IotWebConfESP32HTTPUpdateServer.h>

#include <BlynkSimpleEsp32.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// ============================================================
// BLYNK OPTIMIZATION (Legacy, port 8080, NO SSL)
// ============================================================
#ifndef BLYNK_HEARTBEAT
#define BLYNK_HEARTBEAT      300
#endif
#ifndef BLYNK_TIMEOUT_MS
#define BLYNK_TIMEOUT_MS     10000UL
#endif
#ifndef BLYNK_MSG_LIMIT
#define BLYNK_MSG_LIMIT      30
#endif
#ifndef BLYNK_MAX_READBYTES
#define BLYNK_MAX_READBYTES  512
#endif
#ifndef BLYNK_MAX_SENDBYTES
#define BLYNK_MAX_SENDBYTES  256
#endif

#define BLYNK_NO_DEBUG
#define BLYNK_SEND_ATOMIC
#define BLYNK_NO_FANCY_LOGO
// SSL DISABLED for Blynk Legacy port 8080
// #define BLYNK_USE_SSL
// #define BLYNK_SSL_INSECURE
#define BLYNK_FORCE_SYNC

// ============================================================
// IOTWEBCONF
// ============================================================
extern DNSServer dnsServer;
extern WebServer server;
extern IotWebConf iotWebConf;
extern iotwebconf::OptionalGroupHtmlFormatProvider optionalGroupHtmlFormatProvider;

// ============================================================
// WIFI / INTERNET
// ============================================================
extern int Internet;
extern int Internet_Reset;
extern int Internet_Check_Mode;
extern int consecutive_internet_success;
extern int consecutive_internet_fail;

extern String SSID_AP, PASSWORD_AP;
extern String SSID_NAME, PASSWORD_NAME;
extern String IP, Sub_M, Gate_way;

extern String Sel_SelIP_Sys;
extern int Sel_1_SelIP_Sys;

extern IPAddress ipAddress, gateway, netmask, primaryDNS, secondaryDNS;

// ============================================================
// BLYNK
// ============================================================
extern String Blynk_Token_1;
extern char configblynk[100];
extern String Sel_Blynk_Mode;
extern int Sel_1_Blynk_Mode;
extern bool BlynkConnected;
extern String URL_Firmware;

// ============================================================
// TELEGRAM
// ============================================================
extern WiFiClientSecure secured_client1;
extern UniversalTelegramBot bot1;
extern String message;
extern char Bot_Token_1[60];
extern char Bot_Group[20];
extern String Blynk_Bot_Token, Blynk_Bot_Group;

// ============================================================
// NTP / RTC
// ============================================================
extern WiFiUDP ntpUDP;
extern NTPClient timeClient;
extern int NTP_connect;
extern int Year, Year_1;
extern byte Tsec, Tmin, THour, Day, Month;

// ============================================================
// SYSTEM
// ============================================================
extern SimpleTimer timer;
extern Preferences preferences;
extern SemaphoreHandle_t blynkMutex;
extern bool needReset;
extern bool systemInitialized;
extern int currentMode;
extern const int AP_MODE, FAST_MODE;

// Thread-safe Blynk write helper
#define BLYNK_WRITE_SAFE(pin, val) \
  do { \
    if (blynkMutex != NULL && xSemaphoreTake(blynkMutex, pdMS_TO_TICKS(100)) == pdTRUE) { \
      Blynk.virtualWrite(pin, val); \
      xSemaphoreGive(blynkMutex); \
    } \
  } while(0)

// ============================================================
// FORWARD DECLARATIONS
// ============================================================
void internetcheck();
void configSaved();
void handleRoot();
void Message_telegram();

#endif // COMMCORE_PARAMS_H

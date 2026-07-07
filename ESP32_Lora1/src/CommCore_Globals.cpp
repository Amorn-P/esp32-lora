/**
 * CommCore_Globals.cpp - Global Variable Definitions (Master Only)
 * Defines all externs from CommCore_Params.h.
 * Does NOT include CommCore.h to avoid multiple-definition of
 * objects instantiated in CommCore .h files (dnsServer, server, etc.)
 */
#if BOARD_TYPE == 0

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <SimpleTimer.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <UniversalTelegramBot.h>

// ============================================================
// WIFI / INTERNET
// ============================================================
int Internet = 0;
int Internet_Reset = 0;
int Internet_Check_Mode = 0;
int consecutive_internet_success = 0;
int consecutive_internet_fail = 0;

String SSID_AP, PASSWORD_AP;
String SSID_NAME, PASSWORD_NAME;
String IP, Sub_M, Gate_way;
String Sel_SelIP_Sys = "Dynamic IP";
int Sel_1_SelIP_Sys = 0;
IPAddress ipAddress, gateway, netmask, primaryDNS, secondaryDNS;

// ============================================================
// BLYNK
// ============================================================
String Sel_Blynk_Mode = "Blynk OFF";
int Sel_1_Blynk_Mode = 0;
bool BlynkConnected = false;
String URL_Firmware;
SemaphoreHandle_t blynkMutex = NULL;
int BlynkRst_Hr_int = 0;
int BlynkRst_Min_int = 0;

// ============================================================
// TELEGRAM
// ============================================================
char Bot_Token_1[60] = {0};
char Bot_Group[20] = {0};
WiFiClientSecure secured_client1;
UniversalTelegramBot bot1("", secured_client1);
String message;
String Blynk_Bot_Token, Blynk_Bot_Group;

// ============================================================
// NTP / RTC
// ============================================================
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7*3600, 60000);
int NTP_connect = 0;
int Year = 0, Year_1 = 0;
byte Tsec = 0, Tmin = 0, THour = 0, Day = 0, Month = 0;

// ============================================================
// SYSTEM
// ============================================================
SimpleTimer timer;
Preferences preferences;
bool needReset = false;
bool systemInitialized = false;
int currentMode = 0;
const int AP_MODE = 0;
const int FAST_MODE = 1;
TaskHandle_t CommTask1 = NULL;
TaskHandle_t CommTask4 = NULL;

// WiFi scan state
int numberOfNetworks = 0;
bool wifiScanInProgress = false;
bool wifiScanComplete = false;

// ============================================================
// IOTWEBCONF IDENTITY (extern in CommCore_WiFi.h)
// ============================================================
const char* DEFAULT_THING_NAME      = "Lucky_Lora_AA";
const char* DEFAULT_AP_PASSWORD     = "12345678";
const char* DEFAULT_CONFIG_VERSION  = "v1.0.0";
const char* FALLBACK_WIFI_SSID      = "";
const char* FALLBACK_WIFI_PASSWORD  = "";

// ============================================================
// MESSAGE_TELEGRAM - Status message formatter
// ============================================================
void Message_telegram() {
    message = "✅ " + String(DEFAULT_THING_NAME) + " Online\n";
    message += "🌐 WiFi: " + SSID_NAME + "\n";
    message += "📡 IP: " + IP + "\n";
    message += "🔑 AP: " + SSID_AP + "\n";
}

#endif // BOARD_TYPE == 0

/**
 * CommCore_WiFi.h - WiFi Connection + Internet Check + IotWebConf Setup
 * 
 * Dependencies: CommCore_Params.h, 12_List_Wf.h (IotWebConf parameters),
 *               User_config.h (credentials)
 */

#ifndef COMMCORE_WIFI_H
#define COMMCORE_WIFI_H

#include "CommCore_Params.h"

// ============================================================
// IOTWEBCONF OBJECTS (instantiated once)
// ============================================================
DNSServer dnsServer;
WebServer server(80);
HTTPUpdateServer httpUpdater;
iotwebconf::OptionalGroupHtmlFormatProvider optionalGroupHtmlFormatProvider;

// Thing name + AP password from User_config.h
extern const char* DEFAULT_THING_NAME;
extern const char* DEFAULT_AP_PASSWORD;
extern const char* DEFAULT_CONFIG_VERSION;

IotWebConf iotWebConf(DEFAULT_THING_NAME, &dnsServer, &server, 
                       DEFAULT_AP_PASSWORD, DEFAULT_CONFIG_VERSION);

// ============================================================
// LED PINS
// ============================================================
#define STATUS_PIN    25
#define LED_INTERNET  26

// ============================================================
// IOTWEBCONF SETUP
// ============================================================
void Iotwencof_start()
{
  // Register parameter groups (from 12_List_Wf.h)
  extern iotwebconf::OptionalParameterGroup Static_IP_Group;
  extern iotwebconf::OptionalParameterGroup Blynk_Group;
  extern iotwebconf::OptionalParameterGroup Telegram_Device_Name_Group;
  
  extern IotWebConfCheckboxParameter Chkbox_SelIP_Sys11;
  extern IotWebConfTextParameter ipAddressParam, gatewayParam, netmaskParam;
  extern IotWebConfTextParameter primaryDNSParam, secondaryDNSParam;
  extern IotWebConfCheckboxParameter ChkboxSelBlynk_11;
  extern IotWebConfTextParameter Blynk_Token_11, configblynkserver11;
  extern IotWebConfTextParameter Telegram_Device_Name_Param;

  Static_IP_Group.addItem(&Chkbox_SelIP_Sys11);
  Static_IP_Group.addItem(&ipAddressParam);
  Static_IP_Group.addItem(&gatewayParam);
  Static_IP_Group.addItem(&netmaskParam);
  Static_IP_Group.addItem(&primaryDNSParam);
  Static_IP_Group.addItem(&secondaryDNSParam);
  Blynk_Group.addItem(&ChkboxSelBlynk_11);
  Blynk_Group.addItem(&Blynk_Token_11);
  Blynk_Group.addItem(&configblynkserver11);
  Telegram_Device_Name_Group.addItem(&Telegram_Device_Name_Param);

  iotWebConf.setStatusPin(STATUS_PIN);
  iotWebConf.setHtmlFormatProvider(&optionalGroupHtmlFormatProvider);
  iotWebConf.addParameterGroup(&Static_IP_Group);
  iotWebConf.addParameterGroup(&Blynk_Group);
  iotWebConf.addParameterGroup(&Telegram_Device_Name_Group);
  iotWebConf.setConfigSavedCallback(&configSaved);
  iotWebConf.getApTimeoutParameter()->visible = true;
  strcpy(iotWebConf.getApTimeoutParameter()->valueBuffer, "0"); // Never timeout

  iotWebConf.setupUpdateServer(
    [](const char *updatePath) { httpUpdater.setup(&server, updatePath); },
    [](const char *userName, char *password) { httpUpdater.updateCredentials(userName, password); }
  );

  iotWebConf.init();

  // Web routes
  server.on("/", handleRoot);
  server.on("/config", [] { iotWebConf.handleConfig(); });
  server.on("/telegram-settings", handleTelegramSettings);
  server.on("/save-telegram", HTTP_POST, handleTelegramSave);
  server.on("/restart", HTTP_POST, handleRestart);
  server.on("/save-reset", HTTP_POST, handleResetSave);
  server.on("/scan-wifi", HTTP_GET, handleScanWifi);
  server.on("/scan-wifi-result", HTTP_GET, handleScanWifiResult);
  server.onNotFound([] { iotWebConf.handleNotFound(); });
}

// ============================================================
// WIFI CONNECTION HANDLER
// ============================================================
void connectWifi(const char* ssid, const char* password)
{
  extern char ipAddressValue[128], gatewayValue[128], netmaskValue[128];
  extern char primaryDNSValue[128], secondaryDNSValue[128];
  extern IotWebConfCheckboxParameter Chkbox_SelIP_Sys11;
  extern IotWebConfTextParameter ipAddressParam, gatewayParam, netmaskParam;
  extern IotWebConfTextParameter primaryDNSParam, secondaryDNSParam;

  strcpy(ipAddressValue, ipAddressParam.valueBuffer);
  strcpy(gatewayValue, gatewayParam.valueBuffer);
  strcpy(netmaskValue, netmaskParam.valueBuffer);
  strcpy(primaryDNSValue, primaryDNSParam.valueBuffer);
  strcpy(secondaryDNSValue, secondaryDNSParam.valueBuffer);

  if (strcmp(Chkbox_SelIP_Sys11.valueBuffer, "selected") == 0) {
    Sel_1_SelIP_Sys = 1; Sel_SelIP_Sys = "Static IP";
  } else {
    Sel_1_SelIP_Sys = 0; Sel_SelIP_Sys = "Dynamic IP";
  }

  if (Sel_1_SelIP_Sys == 1) {
    ipAddress.fromString(String(ipAddressValue));
    netmask.fromString(String(netmaskValue));
    gateway.fromString(String(gatewayValue));
    primaryDNS.fromString(String(primaryDNSValue));
    secondaryDNS.fromString(String(secondaryDNSValue));
    if (!WiFi.config(ipAddress, gateway, netmask, primaryDNS, secondaryDNS)) {
      Serial.println("Static IP failed, using DHCP");
    }
  }

  // Fallback if config empty
  if (strlen(ssid) == 0) {
    extern const char* FALLBACK_WIFI_SSID;
    extern const char* FALLBACK_WIFI_PASSWORD;
    ssid = FALLBACK_WIFI_SSID;
    password = FALLBACK_WIFI_PASSWORD;
  }
  WiFi.begin(ssid, password);
}

bool connectAp(const char* apName, const char* password) {
  return WiFi.softAP(apName, password, 4);
}

// ============================================================
// STATIC IP HANDLER REGISTRATION
// ============================================================
void static_ip() {
  iotWebConf.setApConnectionHandler(&connectAp);
  iotWebConf.setWifiConnectionHandler(&connectWifi);
}

// ============================================================
// INTERNET CHECK
// ============================================================
void internetcheck()
{
  unsigned long current_interval;
  switch (Internet_Check_Mode) {
    case 0:  current_interval = 2000;  break;
    case 1:  current_interval = 6000;  break;
    case 2:  current_interval = 10000; break;
    default: current_interval = 6000;  break;
  }

  static unsigned long last_interval = 0;
  if (last_interval != current_interval) {
    timer.setInterval(current_interval, internetcheck);
    last_interval = current_interval;
  }

  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    const char* host = "www.google.com";
    client.setTimeout(2000);
    esp_task_wdt_reset();
    if (client.connect(host, 80)) {
      Internet = 1;
      digitalWrite(LED_INTERNET, 1);
      client.stop();
      if (BlynkConnected) {
        Blynk.virtualWrite(V102, WiFi.localIP().toString());
      }
      consecutive_internet_success++;
      consecutive_internet_fail = 0;
      if (consecutive_internet_success >= 5 && Internet_Check_Mode == 0) {
        Internet_Check_Mode = 1;
      }
    } else {
      Internet = 0;
      digitalWrite(LED_INTERNET, 0);
      consecutive_internet_fail++;
      consecutive_internet_success = 0;
      if (consecutive_internet_fail >= 2 && Internet_Check_Mode == 1) {
        Internet_Check_Mode = 0;
      }
    }
  } else {
    Internet = 0;
    digitalWrite(LED_INTERNET, 0);
    consecutive_internet_fail++;
    consecutive_internet_success = 0;
    if (Internet_Check_Mode == 1) Internet_Check_Mode = 0;
  }

  // Telegram startup notification
  if (Internet == 1 && Internet_Reset == 0 && Year > 2565) {
    Internet_Reset = 1;
    SSID_NAME = iotWebConf.getWifiSsidParameter()->valueBuffer;
    PASSWORD_NAME = iotWebConf.getWifiPasswordParameter()->valueBuffer;
    IP = WiFi.localIP().toString();
    Sub_M = WiFi.subnetMask().toString();
    Gate_way = WiFi.gatewayIP().toString();
    SSID_AP = iotWebConf.getThingNameParameter()->valueBuffer;
    PASSWORD_AP = iotWebConf.getApPasswordParameter()->valueBuffer;
    Message_telegram();
    bot1.sendMessage(Bot_Group, message, "");
    delay(1000);
  }

  esp_task_wdt_reset();
}

#endif // COMMCORE_WIFI_H

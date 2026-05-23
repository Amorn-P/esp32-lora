/**
 * 12_List_Wf.h - IotWebConf Parameters (Master Only)
 * Matches CherryOne_Copy IotWebConf API (5-arg TextParameter)
 */
#if BOARD_TYPE == 0

#ifndef LIST_WF_H
#define LIST_WF_H

#include "CommCore.h"

// ============================================================
// STATIC IP GROUP
// ============================================================
iotwebconf::OptionalParameterGroup Static_IP_Group =
    iotwebconf::OptionalParameterGroup(" Static_IP", " Static_IP", false);

#define ChkboxSelIPSys 10
char Chkbox_SelIP_Sys[ChkboxSelIPSys];
IotWebConfCheckboxParameter Chkbox_SelIP_Sys11 =
    IotWebConfCheckboxParameter(" Mode Static IP", " Mode Static IP",
                                Chkbox_SelIP_Sys, ChkboxSelIPSys, false);

#define STRING_LEN 128
char ipAddressValue[STRING_LEN];
char gatewayValue[STRING_LEN];
char netmaskValue[STRING_LEN];
char primaryDNSValue[STRING_LEN];
char secondaryDNSValue[STRING_LEN];

IotWebConfTextParameter ipAddressParam =
    IotWebConfTextParameter("IP address", "ipAddress", ipAddressValue, STRING_LEN, "192.168.0.222");
IotWebConfTextParameter gatewayParam =
    IotWebConfTextParameter("Gateway", "gateway", gatewayValue, STRING_LEN, "192.168.0.1");
IotWebConfTextParameter netmaskParam =
    IotWebConfTextParameter("Subnet mask", "netmask", netmaskValue, STRING_LEN, "255.255.255.0");
IotWebConfTextParameter primaryDNSParam =
    IotWebConfTextParameter("primaryDNS", "primaryDNS", primaryDNSValue, STRING_LEN, "8.8.8.8");
IotWebConfTextParameter secondaryDNSParam =
    IotWebConfTextParameter("secondaryDNS", "secondaryDNS", secondaryDNSValue, STRING_LEN, "8.8.4.4");

// ============================================================
// BLYNK GROUP
// ============================================================
iotwebconf::OptionalParameterGroup Blynk_Group =
    iotwebconf::OptionalParameterGroup("ONLINE", "Blynk Connect", false);

#define ChkboxSelBlynk 10
char ChkboxSelBlynk_1[ChkboxSelBlynk];
IotWebConfCheckboxParameter ChkboxSelBlynk_11 =
    IotWebConfCheckboxParameter(" ONLINE ", " ONLINE ", ChkboxSelBlynk_1, ChkboxSelBlynk, true);

// Blynk Token (note: char array, CommCore_Params.h has extern String Blynk_Token_1)
// The internals handle the conversion
#define Blynk_Token 34
char Blynk_Token_1[Blynk_Token];
IotWebConfTextParameter Blynk_Token_11 =
    IotWebConfTextParameter("Blynk Token : ", "Blynk Token : ", Blynk_Token_1, Blynk_Token, "");

#define configblynkserver 100
char configblynk[configblynkserver] = "";
IotWebConfTextParameter configblynkserver11 =
    IotWebConfTextParameter("Blynk Server", "Blynk Server ", configblynk, configblynkserver, "43.229.135.169");

// ============================================================
// DEVICE NAME GROUP (Telegram)
// ============================================================
iotwebconf::OptionalParameterGroup Telegram_Device_Name_Group =
    iotwebconf::OptionalParameterGroup("Device Name (Telegram)", "Device Name (Telegram)", false);

#define Telegram_Device_Name_Len 32
char telegram_device_name[Telegram_Device_Name_Len];
IotWebConfTextParameter Telegram_Device_Name_Param =
    IotWebConfTextParameter("Device Name:", "Device Name:", telegram_device_name, Telegram_Device_Name_Len, "");

#endif // LIST_WF_H
#endif // BOARD_TYPE == 0

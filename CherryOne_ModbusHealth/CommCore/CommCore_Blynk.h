/**
 * CommCore_Blynk.h - Blynk Core (System V-pins V90-V127)
 * 
 * Project-specific V-pins (V0-V81) go in your app's 00_Blynk_App.h
 */

#ifndef COMMCORE_BLYNK_H
#define COMMCORE_BLYNK_H

#include "CommCore_Params.h"

// Forward declares
void performOTAUpdate();
void refreshWebConfigValues();

extern char ipAddressValue[128], gatewayValue[128], netmaskValue[128];
extern char primaryDNSValue[128], secondaryDNSValue[128];
extern char Chkbox_SelIP_Sys[10], ChkboxSelBlynk_1[10];

// ============================================================
// BLYNK CONNECTED / DISCONNECTED
// ============================================================
BLYNK_CONNECTED() {
  if (Sel_1_Blynk_Mode == 1) {
    BlynkConnected = true;
    refreshWebConfigValues();
  }
}

BLYNK_DISCONNECTED() {
  if (Sel_1_Blynk_Mode == 1) {
    BlynkConnected = false;
  }
}

// ============================================================
// WEB CONFIG PINS (V90-V101)
// ============================================================
BLYNK_READ(V90)  { Blynk.virtualWrite(V90, String(iotWebConf.getWifiSsidParameter()->valueBuffer)); }
BLYNK_WRITE(V90) { String v = param.asString(); if(v.length()>0){v.toCharArray(iotWebConf.getWifiSsidParameter()->valueBuffer,32);EEPROM.commit();Blynk.virtualWrite(V90,v);} }

BLYNK_READ(V91)  { Blynk.virtualWrite(V91, String(iotWebConf.getWifiPasswordParameter()->valueBuffer)); }
BLYNK_WRITE(V91) { String v = param.asString(); if(v.length()>0){v.toCharArray(iotWebConf.getWifiPasswordParameter()->valueBuffer,64);EEPROM.commit();Blynk.virtualWrite(V91,v);} }

BLYNK_READ(V92)  { Blynk.virtualWrite(V92, (Sel_1_SelIP_Sys==1)?"Enabled":"Disabled"); }
BLYNK_WRITE(V92) { String s = param.asString();
  if(s=="Enable"){strcpy(Chkbox_SelIP_Sys,"selected");Sel_SelIP_Sys="Static IP";Sel_1_SelIP_Sys=1;}
  else{strcpy(Chkbox_SelIP_Sys,"");Sel_SelIP_Sys="Dynamic IP";Sel_1_SelIP_Sys=0;}
  Blynk.virtualWrite(V92,(Sel_1_SelIP_Sys==1)?"Enabled":"Disabled"); }

BLYNK_READ(V93)  { Blynk.virtualWrite(V93, String(ipAddressValue)); }
BLYNK_WRITE(V93) { String v = param.asString(); if(v.length()>0){v.toCharArray(ipAddressValue,128);Blynk.virtualWrite(V93,v);} }
BLYNK_READ(V94)  { Blynk.virtualWrite(V94, String(gatewayValue)); }
BLYNK_WRITE(V94) { String v = param.asString(); if(v.length()>0){v.toCharArray(gatewayValue,128);Blynk.virtualWrite(V94,v);} }
BLYNK_READ(V95)  { Blynk.virtualWrite(V95, String(netmaskValue)); }
BLYNK_WRITE(V95) { String v = param.asString(); if(v.length()>0){v.toCharArray(netmaskValue,128);Blynk.virtualWrite(V95,v);} }
BLYNK_READ(V96)  { Blynk.virtualWrite(V96, String(primaryDNSValue)); }
BLYNK_WRITE(V96) { String v = param.asString(); if(v.length()>0){v.toCharArray(primaryDNSValue,128);Blynk.virtualWrite(V96,v);} }
BLYNK_READ(V97)  { Blynk.virtualWrite(V97, String(secondaryDNSValue)); }
BLYNK_WRITE(V97) { String v = param.asString(); if(v.length()>0){v.toCharArray(secondaryDNSValue,128);Blynk.virtualWrite(V97,v);} }

BLYNK_READ(V98)  { Blynk.virtualWrite(V98, (Sel_1_Blynk_Mode==1)?"Enabled":"Disabled"); }
BLYNK_WRITE(V98) { String s = param.asString();
  if(s=="Enable"){strcpy(ChkboxSelBlynk_1,"selected");Sel_Blynk_Mode="Blynk ON";Sel_1_Blynk_Mode=1;}
  else{strcpy(ChkboxSelBlynk_1,"");Sel_Blynk_Mode="Blynk OFF";Sel_1_Blynk_Mode=0;}
  Blynk.virtualWrite(V98,(Sel_1_Blynk_Mode==1)?"Enabled":"Disabled"); }

BLYNK_READ(V99)  { Blynk.virtualWrite(V99, String(Blynk_Token_1)); }
BLYNK_WRITE(V99) { String v = param.asString(); if(v.length()>0){v.toCharArray(Blynk_Token_1,34);Blynk.virtualWrite(V99,v);} }
BLYNK_READ(V100) { Blynk.virtualWrite(V100, String(configblynk)); }
BLYNK_WRITE(V100){ String v = param.asString(); if(v.length()>0){v.toCharArray(configblynk,100);Blynk.virtualWrite(V100,v);} }

BLYNK_WRITE(V101) {
  if(param.asInt()==1){
    Serial.println("Apply & Restart via Blynk");
    strcpy(Chkbox_SelIP_Sys, (Sel_1_SelIP_Sys==1)?"selected":"");
    extern IotWebConfTextParameter ipAddressParam, gatewayParam, netmaskParam, primaryDNSParam, secondaryDNSParam;
    extern IotWebConfTextParameter Blynk_Token_11, configblynkserver11;
    extern IotWebConfCheckboxParameter ChkboxSelBlynk_11;
    strcpy(ipAddressParam.valueBuffer, ipAddressValue);
    strcpy(gatewayParam.valueBuffer, gatewayValue);
    strcpy(netmaskParam.valueBuffer, netmaskValue);
    strcpy(primaryDNSParam.valueBuffer, primaryDNSValue);
    strcpy(secondaryDNSParam.valueBuffer, secondaryDNSValue);
    strcpy(Blynk_Token_11.valueBuffer, Blynk_Token_1);
    strcpy(configblynkserver11.valueBuffer, configblynk);
    strcpy(ChkboxSelBlynk_11.valueBuffer, (Sel_1_Blynk_Mode==1)?"selected":"");
    iotWebConf.saveConfig();
    ESP.restart();
  }
}

// ============================================================
// OTA PINS (V122-V125)
// ============================================================
BLYNK_WRITE(V123) { URL_Firmware = param.asString(); }
BLYNK_WRITE(V124) { if(param.asInt()==1) performOTAUpdate(); }
BLYNK_WRITE(V125) { if(param.asInt()==1){ vTaskDelay(2000); ESP.restart(); } }

// ============================================================
// RESET TIME PINS (V126-V127)
// ============================================================
extern int BlynkRst_Hr_int, BlynkRst_Min_int;
BLYNK_WRITE(V126) { BlynkRst_Hr_int = param.asString().toInt(); EEPROM.put(100,BlynkRst_Hr_int); EEPROM.commit(); }
BLYNK_WRITE(V127) { BlynkRst_Min_int = param.asString().toInt(); EEPROM.put(110,BlynkRst_Min_int); EEPROM.commit(); }

// ============================================================
// TELEGRAM CONFIG PINS (V118-V119)
// ============================================================
BLYNK_WRITE(V118) { Blynk_Bot_Token = param.asString(); Blynk_Bot_Token.toCharArray(Bot_Token_1,60); EEPROM.put(0,Bot_Token_1); EEPROM.commit(); }
BLYNK_WRITE(V119) { Blynk_Bot_Group = param.asString(); Blynk_Bot_Group.toCharArray(Bot_Group,20); EEPROM.put(70,Bot_Group); EEPROM.commit(); }

// ============================================================
// MODE TOGGLE (V120)
// ============================================================
BLYNK_WRITE(V120) {
  if(param.asInt()==1){
    EEPROM.put(500, currentMode); EEPROM.commit();
    vTaskDelay(2000); ESP.restart();
  }
}

// ============================================================
// REFRESH WEB CONFIG VALUES TO BLYNK
// ============================================================
void refreshWebConfigValues() {
  Blynk.virtualWrite(V90, String(iotWebConf.getWifiSsidParameter()->valueBuffer));
  Blynk.virtualWrite(V91, String(iotWebConf.getWifiPasswordParameter()->valueBuffer));
  Blynk.virtualWrite(V92, (Sel_1_SelIP_Sys==1)?"Enabled":"Disabled");
  Blynk.virtualWrite(V93, String(ipAddressValue));
  Blynk.virtualWrite(V94, String(gatewayValue));
  Blynk.virtualWrite(V95, String(netmaskValue));
  Blynk.virtualWrite(V96, String(primaryDNSValue));
  Blynk.virtualWrite(V97, String(secondaryDNSValue));
  Blynk.virtualWrite(V98, (Sel_1_Blynk_Mode==1)?"Enabled":"Disabled");
  Blynk.virtualWrite(V99, String(Blynk_Token_1));
  Blynk.virtualWrite(V100, String(configblynk));
  Blynk.virtualWrite(V101, "Ready");
  Blynk.virtualWrite(V102, WiFi.localIP().toString());
}

#endif // COMMCORE_BLYNK_H

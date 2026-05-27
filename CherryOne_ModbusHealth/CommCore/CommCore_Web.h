/**
 * CommCore_Web.h - Web Server Handlers
 * Root page, config, Telegram settings, WiFi scan, restart
 */

#ifndef COMMCORE_WEB_H
#define COMMCORE_WEB_H

#include "CommCore_Params.h"

// Forward declares
void handleTelegramSettings();
void handleTelegramSave();
void handleResetSave();
void handleRestart();
void handleScanWifi();
void handleScanWifiResult();

// WiFi scan state
extern int numberOfNetworks;
extern bool wifiScanInProgress;
extern bool wifiScanComplete;

void configSaved()
{
  if (millis() < 60000) return; // Boot guard
  delay(3000);                  // EEPROM commit delay
  needReset = true;
}

// ---- HANDLERS defined in 01_Social_Handlers.h / inline below ----
// handleTelegramSettings, handleTelegramSave, handleResetSave 
// are project-specific and defined in src/01_Social_Handlers.h

void handleRestart()
{
  needReset = true;
  String s = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Restart</title></head>"
             "<body style='background:#111316;color:white;text-align:center;padding:50px;font-family:verdana;'>"
             "<h2>Restarting...</h2><a href='/' style='color:#80deea;'>Back</a></body></html>";
  server.send(200, "text/html", s);
}

// ---- WiFi Scan (async) ----
void handleScanWifi()
{
  if (wifiScanInProgress) {
    server.send(200, "text/plain", "Scan in progress");
    return;
  }
  WiFi.scanDelete();
  wifiScanInProgress = true;
  wifiScanComplete = false;
  numberOfNetworks = 0;
  int r = WiFi.scanNetworks(true, true, 0);
  if (r == -1) server.send(200, "text/plain", "Scan started");
  else if (r >= 0) {
    numberOfNetworks = r; wifiScanInProgress = false; wifiScanComplete = true;
    server.send(200, "text/plain", "Scan done");
  } else {
    wifiScanInProgress = false;
    server.send(200, "text/plain", "Scan failed");
  }
}

void handleScanWifiResult()
{
  int status = WiFi.scanComplete();
  String result; result.reserve(1000);
  if (status == -1) {
    result = "<li style='color:#ff9800;'>Scanning...</li>";
  } else if (status == -2) {
    wifiScanInProgress = false;
    result = "<li style='color:#ef9a9a;'>Scan failed</li>";
  } else if (status >= 0) {
    numberOfNetworks = status; wifiScanInProgress = false; wifiScanComplete = true;
    if (status == 0) result = "<li style='color:#ef9a9a;'>No networks</li>";
    else {
      int max = (status > 5) ? 5 : status;
      for (int i = 0; i < max; i++) {
        result += "<li style='color:#cfd8dc;'>" + String(i+1) + ": " + WiFi.SSID(i) +
                  " RSSI:" + String(WiFi.RSSI(i)) + " Ch:" + String(WiFi.channel(i)) + "</li>";
      }
    }
  } else {
    result = "<li style='color:#ef9a9a;'>No scan in progress</li>";
  }
  server.send(200, "text/html", result);
}

// ---- ROOT PAGE (Status) ----
void handleRoot()
{
  if (iotWebConf.handleCaptivePortal()) return;

  String s; s.reserve(5000);
  s = "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8' name='viewport' content='width=device-width,maximum-scale=1.8'/>"
      "<title>System Status</title></head><body style='background:#111316;font-family:verdana;margin:0;padding:0;'>"
      "<div style='max-width:600px;margin:10px auto;background:#032E3B;padding:20px;border-radius:5px;'>"
      "<p style='color:#b0bec5;font-size:16px;text-align:center;font-weight:bold;'>";

  extern IotWebConfTextParameter Telegram_Device_Name_Param;
  if (strlen(Telegram_Device_Name_Param.valueBuffer) > 0)
    s += String(Telegram_Device_Name_Param.valueBuffer);
  else s += "Set name in Config";
  s += "</p>";

  // Buttons
  s += "<div style='display:flex;justify-content:center;gap:5px;margin:10px 0;'>"
       "<a href='/config' style='text-decoration:none;'><div style='color:white;font-size:14px;width:100px;height:40px;"
       "background:linear-gradient(#28a745,#218838);border-radius:5px;display:flex;align-items:center;justify-content:center;'>Settings</div></a>"
       "<button onclick='startWifiScan()' style='color:white;font-size:14px;width:100px;height:40px;"
       "background:linear-gradient(#007bff,#0056b3);border:none;border-radius:5px;cursor:pointer;'>Search WiFi</button>"
       "<form action='/restart' method='POST'><button type='submit' style='color:white;font-size:14px;width:100px;height:40px;"
       "background:linear-gradient(#f44336,#d32f2f);border:none;border-radius:5px;cursor:pointer;'>Restart</button></form>"
       "</div>"
       "<ul style='color:#cfd8dc;font-size:14px;'>"
       "<li style='color:#a5d6a7;'>Limited <= 5 networks</li><br>"
       "<div id='wifiScanResult'><li style='color:#ef9a9a;'>Press Search WiFi</li></div><p></p>"
       "<li style='color:#81d4fa;'>MAC: " + WiFi.macAddress() + "</li><p></p>"
       "<li style='color:#ce93d8;'>IP Mode: " + Sel_SelIP_Sys + "</li>";

  extern char ipAddressValue[128], gatewayValue[128], netmaskValue[128];
  extern char primaryDNSValue[128], secondaryDNSValue[128];
  
  if (Sel_1_SelIP_Sys == 1) {
    s += "<li style='color:#19cab3;'>Static IP: " + String(ipAddressValue) + 
         " / GW:" + String(gatewayValue) + " / DNS:" + String(primaryDNSValue) + "</li>";
  } else {
    s += "<li style='color:#ff9800;'>DHCP IP: " + WiFi.localIP().toString() + "</li>";
  }
  s += "<p></p><li>Bot ID: " + String(Bot_Group) + "</li>"
       "<li>Blynk Server: " + String(configblynk) + "</li>"
       "<li style='color:#81d4fa;'>Blynk Token: " + String(Blynk_Token_1) + "</li><p></p>"
       "<div style='display:flex;justify-content:center;margin:20px 0;'>"
       "<a href='/telegram-settings' style='text-decoration:none;'><div style='color:white;font-size:14px;padding:0 20px;height:40px;"
       "background:linear-gradient(#ff9800,#f57c00);border-radius:5px;display:flex;align-items:center;justify-content:center;'>Set Parameters</div></a>"
       "</div></ul></div></div>";

  // WiFi scan JS
  s += "<script>function startWifiScan(){var b=document.getElementById('scanWifiBtn');"
       "var d=document.getElementById('wifiScanResult');"
       "d.innerHTML='<li style=\"color:#ff9800;\">Scanning...</li>';"
       "var x=new XMLHttpRequest();x.onreadystatechange=function(){if(x.readyState==4&&x.status==200){"
       "setTimeout(function(){var x2=new XMLHttpRequest();x2.onreadystatechange=function(){"
       "if(x2.readyState==4&&x2.status==200)d.innerHTML=x2.responseText;};"
       "x2.open('GET','/scan-wifi-result',true);x2.send();},3000);}};"
       "x.open('GET','/scan-wifi',true);x.send();}</script></body></html>";

  server.send(200, "text/html", s);
}

#endif // COMMCORE_WEB_H

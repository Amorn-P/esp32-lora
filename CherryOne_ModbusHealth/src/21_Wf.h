#ifndef _21_WF_H
#define _21_WF_H

/**
   IoT Web Configuration Setup
   Registers Parameter Groups and Server Routes
*/


// Function Declarations
void handleRoot();
void configSaved();
void handleTelegramSettings(); // Forward declaration
void handleTelegramSave();     // Forward declaration
void handleResetSave();        // Forward declaration
void handleRestart();
void handleScanWifi();
void handleScanWifiResult();




void Iotwencof_start()
{
  // Network Settings
  Static_IP_Group.addItem(&Chkbox_SelIP_Sys11); // Enable/Disable Static IP
  Static_IP_Group.addItem(&ipAddressParam);     // Static IP Address
  Static_IP_Group.addItem(&gatewayParam);       // Gateway Address
  Static_IP_Group.addItem(&netmaskParam);       // Subnet Mask
  Static_IP_Group.addItem(&primaryDNSParam);    // Primary DNS
  Static_IP_Group.addItem(&secondaryDNSParam);  // Secondary DNS

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
  // [FIX] Keep AP alive indefinitely for setup access
  strcpy(iotWebConf.getApTimeoutParameter()->valueBuffer, "0");

  iotWebConf.setupUpdateServer(
    [](const char *updatePath)
  {
    httpUpdater.setup(&server, updatePath);
  },
  [](const char *userName, char *password)
  {
    httpUpdater.updateCredentials(userName, password);
  });

  iotWebConf.init();
  Serial.println("[WEB] iotWebConf.init() done");

  server.on("/", handleRoot); // (Status Page)
  server.on("/config", []
  { iotWebConf.handleConfig(); });
  server.on("/telegram-settings", handleTelegramSettings); // Telegram Bot
  server.on("/save-telegram", HTTP_POST, handleTelegramSave);        // Telegram Bot Token  Group ID
  server.on("/restart", HTTP_POST, handleRestart);                    // ESP32
  server.on("/save-reset", HTTP_POST, handleResetSave);              // ESP32 Reset
  server.on("/scan-wifi", HTTP_GET, handleScanWifi);                 // WiFi  async
  server.on("/scan-wifi-result", HTTP_GET, handleScanWifiResult);    // WiFi

  server.onNotFound([]()
  {
    iotWebConf.handleNotFound();
  });
}

/**
   ===============================================
   Web Server Root Handler (Status Page)
   ===============================================

    (Status Page)
    Real-time

   Features:
   - Captive Portal Handling
   - WiFi Network Scanning
   - System Status Display
   - Configuration Access
   ===============================================
*/
void handleRoot()
{
  Serial.println("[WEB] handleRoot() called");
  // ===============================================
  // CAPTIVE PORTAL HANDLING
  // ===============================================
  // captive portal requests  WiFi
  if (iotWebConf.handleCaptivePortal())
  {
    // -- Captive portal request were already served.
    return;
  }

  // memory  memory fragmentation
  // reserve  WiFi ()
  String s;
  if (server.hasArg("scanwifi")) {
    s.reserve(5000); // 5KB  WiFi
  } else {
    s.reserve(4000); // 4KB  HTML page 
  }
  
  s = F("<!DOCTYPE html><html lang=\"en\"><head><meta "
       "charset=\"UTF-8\"name=\"viewport\" content=\"width=device-width, "
       "maximum-scale=1.8\"/>");

  s += F("<title>IotWebConf Report</title> </head><body "
         "style=background-color:#111316;font-family:verdana;margin:0;padding:0;>");

  s += F("<div style='display: flex; justify-content: flex-start; align-items: flex-start; min-height: 100vh; flex-direction: column; margin-top: 10px; padding-bottom: 50px;'>");
  s += F("<div style='background-color: #032E3B; padding: 20px; border-radius: 5px; width: 90%; max-width: 600px; margin: 0 auto;'>");
  s += F("<p style='color:#b0bec5; font-size: 16px; text-align: center; font-weight: bold;'> Project name : ");
  if (strlen(Telegram_Device_Name_Param.valueBuffer) > 0) {
    s += String(Telegram_Device_Name_Param.valueBuffer);
  } else {
    s += F("Set the name on the Config webpage.");
  }
  s += F(" </p>");
  s += F("<p style='color:#80deea; font-size: 14px; text-align: center;'>Current setting value :</p>");
  s += F("<div style='display: flex; justify-content: center; align-items: center; gap: 5px; margin: 20px 0;'>");
  s += F("<a href='/config' style='text-decoration: none;'>");
  s += F("<div style='color: white; font-family: verdana; font-size: 14px; width: 100px; height: 40px; "
         "background: linear-gradient(to right, #28a745, #218838); border: none; border-radius: 5px; "
         "cursor: pointer; transition: 0.3s; display: flex; justify-content: center; align-items: center; "
         "box-shadow: 0px 4px 6px rgba(0, 0, 0, 0.2);'>"
         "Settings"
         "</div>");
  s += F("</a>");
  s += F("<button id='scanWifiBtn' onclick='startWifiScan()' style='color: white; font-family: verdana; font-size: 14px; width: 100px; height: 40px; "
         "background: linear-gradient(to right, #007bff, #0056b3); border: none; border-radius: 5px; "
         "cursor: pointer; transition: 0.3s; box-shadow: 0px 4px 6px rgba(0, 0, 0, 0.2);'>"
         "Search WiFi"
         "</button>");
  s += F("<form action='/restart' method='POST'>");
  s += F("<button type='submit' style='color: white; font-family: verdana; font-size: 14px; width: 100px; height: 40px; "
         "background: linear-gradient(to right, #f44336, #d32f2f); border: none; border-radius: 5px; "
         "cursor: pointer; transition: 0.3s; box-shadow: 0px 4px 6px rgba(0, 0, 0, 0.2);'>"
         "Restart"
         "</button>");
  s += F("</form>");

  s += F("</div>"); // Container 

  s += F("<ul style='color:#cfd8dc; font-size: 14px;'>"); // (#cfd8dc)  14px

  s += F("<li style='color:#a5d6a7; font-size: 14px;'>Limited discovery <= 5 networks</li>");
  s += F("<br>");
  s += F("<div id='wifiScanResult'>");
  s += F("<br><li style='color:#ef9a9a; font-size: 14px;'>Press WiFi search button to scan networks</li><br>");
  s += F("</div>");
  s += F("<p></p>");

  s += F("<p></p>");
  s += F("<li style='color:#81d4fa; font-size: 14px;'>MAC ADDRESS : ");
  s += WiFi.macAddress();
  s += F("</li>");

  s += F("<p></p>");
  s += F("<br><li style='color:#ce93d8; font-size: 14px;'>Selected Mode Status: ");
  s += Sel_SelIP_Sys;
  s += F("<p></p>");

  if (Sel_1_SelIP_Sys == 1)
  {
    s += F("<li style='color:#19cab3; font-size: 14px;'>**** Static IP Configuration (ENABLED) ****");
    s += F("<br>-Static IP  : ");
    s += String(ipAddressValue);
    s += F("<br>");
    s += F("-Static_Subnetmark  : ");
    s += String(netmaskValue);
    s += F("<br>");
    s += F("-Static_Gateway : ");
    s += String(gatewayValue);
    s += F("<br>");
    s += F("-Static_primaryDNS : ");
    s += String(primaryDNSValue);
    s += F("<br>");
    s += F("-Static_secondaryDNS : ");
    s += String(secondaryDNSValue);
    s += F("<br>");
    s += F("<p></p>");
  }
  else
  {
    s += F("<li style='color:#ff9800; font-size: 14px;'>**** Static IP Configuration (DISABLED - Using DHCP) ****"); // s += F("<br>-Current IP: ");
    s += WiFi.localIP().toString();
    s += F("<br>");
    s += F("-Gateway: ");
    s += WiFi.gatewayIP().toString();
    s += F("<br>");
    s += F("-Subnet: ");
    s += WiFi.subnetMask().toString();
    s += F("<br>");
    s += F("<p></p>");
  }

  s += F("<li>Bot ID Group : ");
  s += F("<li style='color:#81d4fa; font-size: 14px; word-wrap: break-word; overflow-wrap: break-word; max-width: 100%;'>");
  s += String(Bot_Group);
  s += F("</li>");
  s += F("<li>Bot Token : ");
  s += F("<li style='color:#81d4fa; font-size: 14px; word-wrap: break-word; overflow-wrap: break-word; max-width: 100%;'>");
  s += String(Bot_Token_1);
  s += F("</li>");
  s += F("<p></p>");

  s += F("<li>Blynk Mode Selection : ");
  s += ChkboxSelBlynk_1;
  s += F("<li>Blynk Server : ");
  s += configblynk;
  s += F("<br>");
  s += F("<li>Blynk Token : ");
  s += F("<li style='color:#81d4fa; font-size: 14px; word-wrap: break-word; overflow-wrap: break-word; max-width: 100%;'>");
  s += String(Blynk_Token_1);
  s += F("</li>");
  s += F("<p></p>");

  s += F("<li>Daily System Reset Time : ");
  bool resetDisabled = (BlynkRst_Hr_int == -1 || BlynkRst_Hr_int == 99) || 
                       (BlynkRst_Min_int == -1 || BlynkRst_Min_int == 99);
  
  if (resetDisabled) {
    s += F("<span style='color:#81d4fa;'>Continuous Operation (No Reset)</span>");
  } else {
    s += String(BlynkRst_Hr_int);
    s += F(" : ");
    s += String(BlynkRst_Min_int);
  }
  s += F("<p></p>");

  s += F("<div class='param-set-container' style='display: flex; justify-content: center; align-items: center; margin: 20px 0; width: 100%;'>");
  s += F("<a href='/telegram-settings' style='text-decoration: none;'>");
  s += F("<div style='color: white; font-family: verdana; font-size: 14px; padding: 0 20px; height: 40px; "
         "background: linear-gradient(to right, #ff9800, #f57c00); border: none; border-radius: 5px; "
         "cursor: pointer; transition: 0.3s; display: flex; justify-content: center; align-items: center; "
         "box-shadow: 0px 4px 6px rgba(0, 0, 0, 0.2); white-space: nowrap;'>"
         "Set Parameters"
         "</div>");
  s += F("</a>");
  s += F("</div>");

   

  s += F("</br></ul>");
  s += F("<style>"
         "a div:hover, button:hover {"
         "  transform: scale(1.05);"
         "  filter: brightness(1.2);"
         "}"
         ".btn-container {"
         "  display: flex;"
         "  justify-content: center;"
         "  align-items: flex-start;"
         "  gap: 10px;"
         "  margin: 20px 0;"
         "  flex-wrap: wrap;"
         "}"
         ".btn-wrapper {"
         "  text-decoration: none;"
         "}"
         ".btn {"
         "  color: white;"
         "  font-family: verdana;"
         "  font-size: 14px;"
         "  width: 120px;"
         "  height: 40px;"
         "  border: none;"
         "  border-radius: 5px;"
         "  cursor: pointer;"
         "  transition: 0.3s;"
         "  display: flex;"
         "  justify-content: center;"
         "  align-items: center;"
         "  box-shadow: 0px 4px 6px rgba(0, 0, 0, 0.2);"
         "}"
         ".btn-orange { background: linear-gradient(to right, #ff9800, #f57c00); }"
         ".btn-purple { background: linear-gradient(to right, #9c27b0, #7b1fa2); }"
         ".btn-cyan { background: linear-gradient(to right, #00bcd4, #0097a7); }"
         "@media (max-width: 768px) {"
         "  .btn-container {"
         "    max-width: 280px;"
         "    margin: 20px auto;"
         "    justify-content: space-between;"
         "  }"
         "  .btn-wrapper:nth-child(3) {"
         "    order: 3;"
         "    margin-left: auto;"
         "  }"
         "  .btn {"
         "    font-size: 14px;"
         "    width: 120px;"
         "    height: 40px;"
         "    border-radius: 5px;"
         "    white-space: nowrap;"
         "    overflow: hidden;"
         "    text-overflow: ellipsis;"
         "    display: flex;"
         "    align-items: center;"
         "    justify-content: center;"
         "  }"
         "  .param-set-container {"
         "    display: flex !important;"
         "    justify-content: center !important;"
         "    align-items: center !important;"
         "    width: 100% !important;"
         "    margin: 20px auto !important;"
         "    padding: 0 !important;"
         "  }"
         "  .param-set-container a {"
         "    margin: 0 auto !important;"
         "  }"
         "}"
         ".param-set-container {"
         "  display: flex;"
         "  justify-content: center;"
         "  align-items: center;"
         "  width: 100%;"
         "  margin: 20px 0;"
         "}"
         "</style>");
  
  s += "<script>";
  s += "function startWifiScan(){";
  s += "var btn=document.getElementById('scanWifiBtn');";
  s += "var resultDiv=document.getElementById('wifiScanResult');";
  s += "if(!btn||!resultDiv){alert('Elements not found');return;}";
  s += "btn.disabled=true;";
  s += "btn.textContent='Scanning...';";
  s += "resultDiv.innerHTML='<li style=\"color:#ff9800;font-size:14px;\">Scanning WiFi networks... Please wait...</li>';";
  s += "var xhr1=new XMLHttpRequest();";
  s += "xhr1.onreadystatechange=function(){";
  s += "if(xhr1.readyState===4){";
  s += "if(xhr1.status===200){";
  s += "setTimeout(function(){";
  s += "var xhr2=new XMLHttpRequest();";
  s += "xhr2.onreadystatechange=function(){";
  s += "if(xhr2.readyState===4){";
  s += "if(xhr2.status===200){";
  s += "resultDiv.innerHTML=xhr2.responseText;";
  s += "btn.disabled=false;";
  s += "btn.textContent='Search WiFi';";
  s += "}else{";
  s += "resultDiv.innerHTML='<li style=\"color:#ef9a9a;font-size:14px;\">Error loading results. Status: '+xhr2.status+'</li>';";
  s += "btn.disabled=false;";
  s += "btn.textContent='Search WiFi';";
  s += "}";
  s += "}";
  s += "};";
  s += "xhr2.open('GET','/scan-wifi-result',true);";
  s += "xhr2.send();";
  s += "},3000);";
  s += "}else{";
  s += "resultDiv.innerHTML='<li style=\"color:#ef9a9a;font-size:14px;\">Error starting scan. Status: '+xhr1.status+'</li>';";
  s += "btn.disabled=false;";
  s += "btn.textContent='Search WiFi';";
  s += "}";
  s += "}";
  s += "};";
  s += "xhr1.onerror=function(){";
  s += "resultDiv.innerHTML='<li style=\"color:#ef9a9a;font-size:14px;\">Network error. Please check connection.</li>';";
  s += "btn.disabled=false;";
  s += "btn.textContent='Search WiFi';";
  s += "};";
  s += "xhr1.open('GET','/scan-wifi',true);";
  s += "xhr1.send();";
  s += "}";
  s += "</script>";
  
  s += F("</div></div><br><br></body></html>");
  server.send(200, "text/html", s);
}

/**
   ===============================================
   Configuration Saved Callback
   ===============================================

   
    ESP32 
   ===============================================
*/
void configSaved()
{
  Serial.printf("[CONFIG] configSaved() called at millis=%lu\n", millis());
  if (millis() < 60000) { Serial.println("[CONFIG] Ignored - within 60s boot window"); return; }
  Serial.println("[CONFIG] Waiting 3s for EEPROM commit...");
  delay(3000);
  needReset = true;
  Serial.println("[CONFIG] needReset set - will reboot");
}

// ===============================================
// TELEGRAM SETTINGS PAGE HANDLER
// ===============================================

// ===============================================
// RESTART HANDLER
// ===============================================

// ===============================================
// ASYNC WIFI SCAN HANDLERS
// ===============================================

/**
   ===============================================
   Handle WiFi Scan Request (Async)
   ===============================================
    WiFi  async  response 
   ===============================================
*/
void handleScanWifi()
{
  Serial.println("🔍 handleScanWifi() called");
  
  if (wifiScanInProgress) {
    Serial.println("⚠️ Scan already in progress");
    server.send(200, "text/plain", "Scan already in progress");
    return;
  }
  
  WiFi.scanDelete();
  Serial.println("🗑️ Old scan results deleted");
  
  wifiScanInProgress = true;
  wifiScanComplete = false;
  numberOfNetworks = 0;
  
  // WiFi  async
  // async=true, show_hidden=true, channel=0 (scan all channels)
  Serial.println("📡 Starting WiFi scan (async)...");
  int scanResult = WiFi.scanNetworks(true, true, 0);
  Serial.print("📊 Scan result code: ");
  Serial.println(scanResult);
  
  // // WiFi.scanNetworks() :
  // WIFI_SCAN_RUNNING (-1) =  ()
  // WIFI_SCAN_FAILED (-2) = 
  // >= 0 =  ()
  if (scanResult == -1) {  // WIFI_SCAN_RUNNING
    // Serial.println("✅ Scan started successfully");
    server.send(200, "text/plain", "Scan started");
  } else if (scanResult == -2) {  // WIFI_SCAN_FAILED
    // Serial.println("❌ Scan failed to start");
    wifiScanInProgress = false;
    server.send(200, "text/plain", "Scan failed to start");
  } else if (scanResult >= 0) {
    // ()
    Serial.print("✅ Scan completed immediately. Found: ");
    Serial.println(scanResult);
    numberOfNetworks = scanResult;
    wifiScanInProgress = false;
    wifiScanComplete = true;
    server.send(200, "text/plain", "Scan completed");
  } else {
    // Serial.print("⚠️ Unknown scan result: ");
    Serial.println(scanResult);
    wifiScanInProgress = false;
    server.send(200, "text/plain", "Scan error");
  }
}

/**
   ===============================================
   Handle WiFi Scan Result Request
   ===============================================
    WiFi  HTML
   ===============================================
*/
void handleScanWifiResult()
{
  Serial.println("🔍 handleScanWifiResult() called");
  
  // // WiFi.scanComplete() :
  // -1 (WIFI_SCAN_RUNNING) = 
  // -2 (WIFI_SCAN_FAILED) = 
  // >= 0 =  ()
  int scanStatus = WiFi.scanComplete();
  Serial.print("📊 Scan status: ");
  Serial.println(scanStatus);
  
  // HTML 
  String result = "";
  result.reserve(1000);
  
  if (scanStatus == -1) {  // WIFI_SCAN_RUNNING
    // Serial.println("⏳ Still scanning...");
    result += F("<li style='color:#ff9800; font-size: 14px;'>Still scanning... Please wait...</li>");
    server.send(200, "text/html", result);
    return;
  }
  
  if (scanStatus == -2) {  // WIFI_SCAN_FAILED
    // Serial.println("❌ Scan failed");
    wifiScanInProgress = false;
    wifiScanComplete = false;
    result += F("<br><li style='color:#ef9a9a; font-size: 14px;'>WiFi scan failed. Please try again.</li><br>");
    server.send(200, "text/html", result);
    return;
  }
  
  // - 
  if (scanStatus >= 0) {
    Serial.print("✅ Scan completed. Found: ");
    Serial.println(scanStatus);
    numberOfNetworks = scanStatus;
    wifiScanInProgress = false;
    wifiScanComplete = true;
    
    if (numberOfNetworks == 0) {
      result += F("<br><li style='color:#ef9a9a; font-size: 14px;'>No networks found</li><br>");
    } else {
      // 5 
      int maxDisplay = (numberOfNetworks > 5) ? 5 : numberOfNetworks;
      Serial.print("📋 Displaying ");
      Serial.print(maxDisplay);
      Serial.println(" networks");
      for (int i = 0; i < maxDisplay; i++) {
        result += F("<li style='color:#cfd8dc; font-size: 14px;'>");
        result += String(i + 1);
        result += F(": ");
        result += WiFi.SSID(i);
        result += F("  RSSI : ");
        result += String(WiFi.RSSI(i));
        result += F(" db  Ch: ");
        result += String(WiFi.channel(i));
        result += F("</li>");
      }
    }
    
    server.send(200, "text/html", result);
  } else {
    // ()
    Serial.println("⚠️ No scan in progress");
    wifiScanInProgress = false;
    wifiScanComplete = false;
    result += F("<br><li style='color:#ef9a9a; font-size: 14px;'>No scan in progress. Please click Search WiFi button.</li><br>");
    server.send(200, "text/html", result);
  }
}

/**
 * handleRestart
 * Sets the reset flag and sends a confirmation page to the browser.
 * The actual reset is handled by iotWebConf in the next loop.
 */
void handleRestart()
{
  needReset = true;
  String s = F("<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Restarting</title></head>");
  s += F("<body style='background-color:#111316;color:white;text-align:center;padding-top:50px;font-family:verdana;'>");
  s += F("<h2>Restarting ESP32...</h2><p>Please wait a few seconds and refresh the home page.</p>");
  s += F("<a href='/' style='color:#80deea;text-decoration:none;'>Back to Home</a></body></html>");
  server.send(200, "text/html", s);
  Serial.println("🔄 Restart requested via Web Interface");
}

#endif

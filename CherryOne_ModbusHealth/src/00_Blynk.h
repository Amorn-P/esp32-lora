/*
  ===============================================
  BLYNK VIRTUAL PINS CONFIGURATION
  ===============================================
  
  Web Configuration Pins (V90-V102):
  V90  = WiFi SSID (Label + Input) -  WiFi SSID
  V91  = WiFi Password (Label + Input) -  WiFi Password
  V92  = Static IP Checkbox (Label + Switch) - / Static IP Mode
  V93  = Static IP Address (Label + Input) -  Static IP Address
  V94  = Gateway (Label + Input) -  Gateway Address
  V95  = Subnet Mask (Label + Input) -  Subnet Mask
  V96  = Primary DNS (Label + Input) -  Primary DNS
  V97  = Secondary DNS (Label + Input) -  Secondary DNS
  V98  = Blynk Checkbox (Label + Switch) - / Blynk Mode
  V99  = Blynk Token (Label + Input) -  Blynk Token
  V100 = Blynk Server (Label + Input) -  Blynk Server
  V101 = Apply & Restart Button (Button) -  ESP32
  V102 = Current IP Address (Label) -  IP  ESP32 

  OTA Firmware Update Pins (V122-V127):
  V122 = 
  V123 =  URL Upload firmware  Blynk  ESP32
  V124 = Start Upload Firmware  PUSH   Blynk
  V125 = Reset ESP32  Reset  Blynk
  V126 =  Reset   Blynk  ESP32
  V127 =  Reset   Blynk  ESP32

 
*/
// ===========================================
// Forward Declarations
// ===========================================
void performOTAUpdate();  // OTA Update ()
void configSaved();       // 21_Wf.h -  21_Wf.h 
void Converse_value();    // 22_Cons.h -  22_Cons.h 
extern void stopSystemAndVerifyAll(bool reportToBlynk); // 00_Blynk1.h

// ===========================================
// External Variables ( 13_Param.h)
// ===========================================
extern String URL_Firmware;  // 13_Param.h 
extern bool needReset;        // 13_Param.h 
// ===========================================
// URL Upload firmware  Blynk  ESP32
// ===========================================



BLYNK_WRITE(V123)
{
  URL_Firmware = param.asString();// Blynk   String
  // Serial.print("URL_Firmware :");  Serial.println(URL_Firmware); // URL

  // URL  Blynk
  if (URL_Firmware.length() > 0) {
    Blynk.virtualWrite(V122, "URL :  Upload Firmware" /*+ URL_Firmware*/);
  } else {
    Blynk.virtualWrite(V122, "URL ");
  }
}
// ===========================================
// Start Upload Firmware
// PUSH   Blynk
// ===========================================
BLYNK_WRITE(V124)
{
  int pinValue = param.asInt();
  if (pinValue == 1) {
    performOTAUpdate(); // OTA Update
  }
}
//------------------------------------------------------



// ===========================================
// Reset ESP32  Reset  Blynk
// ===========================================
BLYNK_WRITE(V125)
{
  int pinValue = param.asInt();
  //Serial.println("V125 received: " + String(pinValue)); // Debug

  if (pinValue == 1) {
    Serial.println("Rebooting ESP32 in 2 seconds...");
    Blynk.virtualWrite(V122, " ESP32...");
    Blynk.syncAll(); // Blynk 

    vTaskDelay(2000 / portTICK_PERIOD_MS); // 2 
    Serial.println("Restarting ESP32 now...");
    ESP.restart();
  }
  if (pinValue == 0) {
    //Serial.println("V125 button released");
  }
}
// ===========================================
// Reset   Blynk  ESP32
// ===========================================
BLYNK_WRITE(V126)
{
  // Local Mode  ( Local Mode )
  if (Sel_1_Mode_Local_Or_Blynk == 1) {
    Serial.println("⚠️ BLYNK_WRITE(V126) ignored - Local Mode active");
    return; // Local Mode
  }
  
  BlynkRst_Hr_St1 = param.asString();// Blynk   String
  BlynkRst_Hr_int = BlynkRst_Hr_St1.toInt();// String  int
  EEPROM.put(100, BlynkRst_Hr_int); // int  BlynkRst_Hr_int  Address 100
  esp_task_wdt_reset();  // watchdog  EEPROM.commit() 
  EEPROM.commit(); // // BlynkRst_Hr_int  Converse_value()  22_Cons.h 
}
//------------------------------------------------------------------------
// ===========================================
// Reset   Blynk  ESP32
// ===========================================
BLYNK_WRITE(V127)
{
  // Local Mode  ( Local Mode )
  if (Sel_1_Mode_Local_Or_Blynk == 1) {
    Serial.println("⚠️ BLYNK_WRITE(V127) ignored - Local Mode active");
    return; // Local Mode
  }
  
  BlynkRst_Min_St1 = param.asString();// Blynk   String
  BlynkRst_Min_int = BlynkRst_Min_St1.toInt();// String  int
  EEPROM.put(110, BlynkRst_Min_int); // int  BlynkRst_Min_int  Address 110
  esp_task_wdt_reset();  // watchdog  EEPROM.commit() 
  EEPROM.commit(); // // BlynkRst_Min_int  Converse_value()  22_Cons.h 
}
//------------------------------------------------------------------------


// ===========================================
// Function to perform OTA update
// ===========================================
void performOTAUpdate() {
  // URL 
  if (URL_Firmware.isEmpty()) {
    Serial.println("Download URL is empty. No update will be performed.");
    Blynk.virtualWrite(V122, "URL  - ");
    return; // URL 
  }

  // [SAFETY] Turn off all relays and pump before starting OTA
  Serial.println("OTA Safety: Stopping all systems...");
  Blynk.virtualWrite(V122, "Safety Check: Stopping Pumps...");
  stopSystemAndVerifyAll(true); 
  vTaskDelay(500 / portTICK_PERIOD_MS);

  Blynk.virtualWrite(V122, "...");
  Serial.println("Starting OTA update...");
  
  HTTPClient http;
  http.setConnectTimeout(10000); // 10s timeout
  // Allow following redirects (essential for GitHub/Dropbox/Firebase)
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.begin(URL_Firmware);

  // Setup Progress Callback
  Update.onProgress([](size_t progress, size_t total) {
    esp_task_wdt_reset(); // Feed the watchdog during the download
    static int lastPercent = -1;
    int percent = (progress * 100) / total;
    if (percent % 5 == 0 && percent != lastPercent) { // Update every 5%
      String progressMsg = "Progress: " + String(percent) + "%";
      // Task4 handles the buffer, so virtualWrite is safe here
      Blynk.virtualWrite(V122, progressMsg);
      Serial.println(progressMsg);
      lastPercent = percent;
    }
  });

  // [FIX]  WDT  OTA
  // Update.writeStream()  30-120  (firmware ~1-2MB)
  // WDT  timeout 30  restart 
  esp_task_wdt_reset(); 

  int httpCode = http.GET();

  if (httpCode == 200) {
    int len = http.getSize(); // Get file size
    float lenMB = len / 1024.0 / 1024.0; // Convert bytes to megabytes

    if (len > 0) {
      Serial.printf("File size to be updated: %d bytes (%.2f MB)\n", len, lenMB);
      Blynk.virtualWrite(V122, ": " + String(len) + " bytes (" + String(lenMB, 2) + " MB)");
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      Blynk.virtualWrite(V122, "...");
      //Serial.println("Starting OTA update...");
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      // if (!Update.begin(len)) { // Serial.printf("Not enough space to begin OTA (Size: %d bytes)\n", len);
        Update.printError(Serial);
        Blynk.virtualWrite(V122, " OTA");
        esp_task_wdt_reset();
        return; // }

      WiFiClient *client = http.getStreamPtr();
      size_t written = Update.writeStream(*client); // Write the update ( 30-120 )

      // bytes 
      if (written == len) {
        Serial.println("OTA update successful.");
        Serial.printf("Bytes written: %d, Expected: %d\n", written, len);

        if (Update.end()) { // Serial.println("OTA completed successfully.");
          Serial.println(" ESP32...");

          // Blynk.virtualWrite(V122, " - ");
          Blynk.syncAll(); // Blynk 

          // Blynk 
          vTaskDelay(3000 / portTICK_PERIOD_MS);

          // ESP32 (WDT  restart )
          Serial.println(" ESP32...");
          ESP.restart();
        } else {
          Serial.println("OTA update failed - Update.end() failed");
          Update.printError(Serial);
          Blynk.virtualWrite(V122, " OTA  - Update.end() failed");
          Blynk.syncAll();
          esp_task_wdt_reset();
        }
      } else {
        Serial.printf("OTA update failed. Bytes written: %d\n", written);
        Update.printError(Serial);
        Blynk.virtualWrite(V122, " OTA  - ");
        Blynk.syncAll();
        esp_task_wdt_reset();
      }
    }
  } else {
    Serial.printf(" URL . Error code: %d\n", httpCode);
    Blynk.virtualWrite(V122, " URL  - Error: " + String(httpCode));
    Blynk.syncAll();
    esp_task_wdt_reset();
  }

  http.end(); // HTTP session
}

// ===============================================
// WEB CONFIG VIRTUAL PINS (V90-V100)
// ===============================================
// ===========================================
// V90: WiFi SSID (Label + Input)
// ===========================================
BLYNK_READ(V90) // WiFi SSID 
{
  Blynk.virtualWrite(V90, String(iotWebConf.getWifiSsidParameter()->valueBuffer));
}
// ===========================================
// WiFi SSID 
// ===========================================
BLYNK_WRITE(V90) 
{
  String newSSID = param.asString();
  if (newSSID.length() > 0) {
    newSSID.toCharArray(iotWebConf.getWifiSsidParameter()->valueBuffer, 32);
    esp_task_wdt_reset();  // watchdog  EEPROM.commit() 
    EEPROM.commit();
    Blynk.virtualWrite(V90, newSSID);
  }
}
// ===========================================
// V91: WiFi Password (Label + Input)
// ===========================================
BLYNK_READ(V91) // WiFi Password 
{
  Blynk.virtualWrite(V91, String(iotWebConf.getWifiPasswordParameter()->valueBuffer));
}
// ===========================================
// WiFi Password 
// ===========================================
BLYNK_WRITE(V91) 
{
  String newPassword = param.asString();
  if (newPassword.length() > 0) {
    newPassword.toCharArray(iotWebConf.getWifiPasswordParameter()->valueBuffer, 64);
    esp_task_wdt_reset();  // watchdog  EEPROM.commit() 
    EEPROM.commit();
    Blynk.virtualWrite(V91, newPassword);
  }
}
// ===========================================
// V92: Static IP Enable/Disable Checkbox
// ===========================================
BLYNK_READ(V92) // Static IP Checkbox
{
  String staticIPStatus = (Sel_1_SelIP_Sys == 1) ? "Enabled" : "Disabled";
  Blynk.virtualWrite(V92, staticIPStatus);
}
// ===========================================
// / Static IP Checkbox
// ===========================================
BLYNK_WRITE(V92) 
{
  String status = param.asString();
  if (status == "Enable") {
    strcpy(Chkbox_SelIP_Sys, "selected");
    Sel_SelIP_Sys = "Static IP";
    Sel_1_SelIP_Sys = 1;
  } else if (status == "Disable") {
    strcpy(Chkbox_SelIP_Sys, "");
    Sel_SelIP_Sys = "Dynamic IP";
    Sel_1_SelIP_Sys = 0;
  }
  Blynk.virtualWrite(V92, (Sel_1_SelIP_Sys == 1) ? "Enabled" : "Disabled");

  // EEPROM -  V101
}
// ===========================================
// V93: Static IP Address (Label + Input)
// ===========================================
BLYNK_READ(V93) // Static IP Address 
{
  Blynk.virtualWrite(V93, String(ipAddressValue));
}
// ===========================================
// Static IP Address 
// ===========================================
BLYNK_WRITE(V93)
{
  String newIP = param.asString();
  if (newIP.length() > 0) {
    // ( EEPROM)
    newIP.toCharArray(ipAddressValue, STRING_LEN);
    Blynk.virtualWrite(V93, newIP);

    // EEPROM -  V101
  }
}
// =============================================
// V94: Gateway (Label + Input)
// =============================================
BLYNK_READ(V94) // Gateway 
{
  Blynk.virtualWrite(V94, String(gatewayValue));
}
// =============================================
// Gateway 
// =============================================
BLYNK_WRITE(V94)
{
  String newGateway = param.asString();
  if (newGateway.length() > 0) {
    // ( EEPROM)
    newGateway.toCharArray(gatewayValue, STRING_LEN);
    Blynk.virtualWrite(V94, newGateway);

    // EEPROM -  V101
  }
}
// =============================================
// V95: Subnet Mask (Label + Input)
// =============================================
BLYNK_READ(V95) // Subnet Mask 
{
  Blynk.virtualWrite(V95, String(netmaskValue));
}
// =============================================
// Subnet Mask 
// =============================================
BLYNK_WRITE(V95)
{
  String newNetmask = param.asString();
  if (newNetmask.length() > 0) {
    // ( EEPROM)
    newNetmask.toCharArray(netmaskValue, STRING_LEN);
    Blynk.virtualWrite(V95, newNetmask);

    // EEPROM -  V101
  }
}
// =============================================
// V96: Primary DNS (Label + Input)
// =============================================
BLYNK_READ(V96) // Primary DNS 
{
  Blynk.virtualWrite(V96, String(primaryDNSValue));
}
// =============================================
// Primary DNS 
// =============================================
BLYNK_WRITE(V96)
{
  String newPrimaryDNS = param.asString();
  if (newPrimaryDNS.length() > 0) {
    // ( EEPROM)
    newPrimaryDNS.toCharArray(primaryDNSValue, STRING_LEN);
    Blynk.virtualWrite(V96, newPrimaryDNS);

    // EEPROM -  V101
  }
}
// =============================================
// V97: Secondary DNS (Label + Input)
// =============================================
BLYNK_READ(V97) // Secondary DNS 
{
  Blynk.virtualWrite(V97, String(secondaryDNSValue));
}
// =============================================
// Secondary DNS 
// =============================================
BLYNK_WRITE(V97)
{
  String newSecondaryDNS = param.asString();
  if (newSecondaryDNS.length() > 0) {
    // ( EEPROM)
    newSecondaryDNS.toCharArray(secondaryDNSValue, STRING_LEN);
    Blynk.virtualWrite(V97, newSecondaryDNS);

    // EEPROM -  V101
  }
}
// =============================================
// V98: Blynk Enable/Disable Checkbox
// =============================================
BLYNK_READ(V98)
{
  String blynkStatus = (Sel_1_Blynk_Mode == 1) ? "Enabled" : "Disabled";
  Blynk.virtualWrite(V98, blynkStatus);
}
// =============================================
// / Blynk Checkbox
// =============================================
BLYNK_WRITE(V98)
{
  String status = param.asString();
  if (status == "Enable") {
    strcpy(ChkboxSelBlynk_1, "selected");
    Sel_Blynk_Mode = "Blynk ";
    Sel_1_Blynk_Mode = 1;
  } else if (status == "Disable") {
    strcpy(ChkboxSelBlynk_1, "");
    Sel_Blynk_Mode = "Blynk ";
    Sel_1_Blynk_Mode = 0;
  }
  Blynk.virtualWrite(V98, (Sel_1_Blynk_Mode == 1) ? "Enabled" : "Disabled");

  // EEPROM -  V101
}
// =============================================
// V99: Blynk Token (Label + Input)
// =============================================
BLYNK_READ(V99) // Blynk Token 
{
  Blynk.virtualWrite(V99, String(Blynk_Token_1));
}
// =============================================
// Blynk Token 
// =============================================
BLYNK_WRITE(V99)
{
  String newToken = param.asString();
  if (newToken.length() > 0) {
    // ( EEPROM)
    newToken.toCharArray(Blynk_Token_1, STRING_LEN);
    Blynk.virtualWrite(V99, newToken);

    // EEPROM -  V101
  }
}

// =============================================
// V100: Blynk Server (Label + Input)
// =============================================
BLYNK_READ(V100) // Blynk Server 
{
  Blynk.virtualWrite(V100, String(configblynk));
}

BLYNK_WRITE(V100) // Blynk Server 
{
  String newServer = param.asString();
  if (newServer.length() > 0) {
    // ( EEPROM)
    newServer.toCharArray(configblynk, STRING_LEN);
    Blynk.virtualWrite(V100, newServer);

    // EEPROM -  V101
  }
}

// =============================================
// V101: Apply & Restart Button
// ( Apply  Config)
// =============================================
BLYNK_WRITE(V101) // Apply & Restart
{
  int pinValue = param.asInt();
  if (pinValue == 1) {
    Serial.println("Apply & Restart button pressed - Saving all settings...");
    Blynk.virtualWrite(V122, "...");

    // EEPROM  iotWebConf
    strcpy(ipAddressParam.valueBuffer, ipAddressValue);
    strcpy(gatewayParam.valueBuffer, gatewayValue);
    strcpy(netmaskParam.valueBuffer, netmaskValue);
    strcpy(primaryDNSParam.valueBuffer, primaryDNSValue);
    strcpy(secondaryDNSParam.valueBuffer, secondaryDNSValue);
    strcpy(Blynk_Token_11.valueBuffer, Blynk_Token_1);
    strcpy(configblynkserver11.valueBuffer, configblynk);

    // Checkbox values
    if (Sel_1_SelIP_Sys == 1) {
      strcpy(Chkbox_SelIP_Sys11.valueBuffer, "selected");
    } else {
      strcpy(Chkbox_SelIP_Sys11.valueBuffer, "");
    }

    if (Sel_1_Blynk_Mode == 1) {
      strcpy(ChkboxSelBlynk_11.valueBuffer, "selected");
    } else {
      strcpy(ChkboxSelBlynk_11.valueBuffer, "");
    }

    // EEPROM
    iotWebConf.saveConfig();

    // Blynk 
    Blynk.syncAll();

    // Blynk 
    vTaskDelay(3000 / portTICK_PERIOD_MS);

    // ESP32
    Serial.println("Restarting ESP32 after applying settings...");
    ESP.restart();
  }
}
// =============================================
// V102: Current IP Address (Label)
// -  IP  ESP32 
// =============================================
BLYNK_READ(V102) // IP 
{
  Blynk.virtualWrite(V102, WiFi.localIP().toString());
}

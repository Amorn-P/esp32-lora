/**
   11_Wf_Main.h - WiFi & Web Configuration
   Includes: Static/Dynamic IP, OTA, Internet Check, WiFi Scanning
**/

// ===============================================
// SYSTEM INCLUDES
// ===============================================

#include "esp_task_wdt.h"
#include "User_config.h"
#include <IotWebConf.h>
#include <IotWebConfUsing.h>

// ===============================================
// CONFIGURATION CONSTANTS
// ===============================================

// WiFi Access Point Configuration
const char thingName[] = DEFAULT_THING_NAME;
const char wifiInitialApPassword[] = DEFAULT_AP_PASSWORD;
#define CONFIG_VERSION DEFAULT_CONFIG_VERSION


// Network Server Objects
DNSServer dnsServer;     // DNS Server  Captive Portal
WebServer server(80);    // Web Server  Configuration

// ===============================================
// OTA FIRMWARE UPDATE CONFIGURATION
// ===============================================

// Platform-specific OTA includes
#ifdef ESP8266
# include <ESP8266HTTPUpdateServer.h>
#elif defined(ESP32)
# include <IotWebConfESP32HTTPUpdateServer.h>
#endif

#ifdef ESP8266
ESP8266HTTPUpdateServer httpUpdater;
#elif defined(ESP32)
HTTPUpdateServer httpUpdater;
#endif

// ===============================================
// IOTWEBCONF CONFIGURATION
// ===============================================

// Optional Group Configuration (/)
#include <IotWebConfOptionalGroup.h>
iotwebconf::OptionalGroupHtmlFormatProvider optionalGroupHtmlFormatProvider;

// Main IotWebConf Object
IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword, CONFIG_VERSION);


// ===============================================
// PIN DEFINITIONS
// ===============================================
#define STATUS_PIN 25 

// ===============================================
// FUNCTION DECLARATIONS
// ===============================================
void handleRoot();
void configSaved();
void Message_telegram(); // Forward declaration for Social Handlers

// ===============================================
// LOCAL INCLUDES
// ===============================================
#include "12_List_Wf.h"
#include "13_Param.h"
#include "15_Set_Tsk.h"

// ===============================================
// WIFI CONNECTION FUNCTIONS
// ===============================================

/**
   ===============================================
   Static IP WiFi Connection
   ===============================================

   Handles Static vs Dynamic IP selection from Web Config
*/
void connectWifi(const char* ssid, const char* password) {

  // โหลดค่าจาก EEPROM ของเว็บ Config ก่อน
  strcpy(ipAddressValue, ipAddressParam.valueBuffer);
  strcpy(gatewayValue, gatewayParam.valueBuffer);
  strcpy(netmaskValue, netmaskParam.valueBuffer);
  strcpy(primaryDNSValue, primaryDNSParam.valueBuffer);
  strcpy(secondaryDNSValue, secondaryDNSParam.valueBuffer);

  // โหลดค่า Static IP Checkbox จาก EEPROM
  if (strcmp(Chkbox_SelIP_Sys11.valueBuffer, "selected") == 0) {
    Sel_1_SelIP_Sys = 1;
    Sel_SelIP_Sys = "Static IP";
  } else {
    Sel_1_SelIP_Sys = 0;
    Sel_SelIP_Sys = "Dynamic IP";
  }

  // ตรวจสอบว่าผู้ใช้เลือกใช้ Static IP หรือไม่
  if (Sel_1_SelIP_Sys == 1) {

    // แปลงค่า IP Address จาก String เป็น IPAddress object
    ipAddress.fromString(String(ipAddressValue));
    netmask.fromString(String(netmaskValue));
    gateway.fromString(String(gatewayValue));
    primaryDNS.fromString(String(primaryDNSValue));
    secondaryDNS.fromString(String(secondaryDNSValue));

    // ตั้งค่า Static IP Configuration
    if (!WiFi.config(ipAddress, gateway, netmask, primaryDNS, secondaryDNS)) {
      Serial.println("❌ Static IP Configuration Failed!");
      Serial.println("🔧 Falling back to DHCP mode...");
    } else {
      Serial.println("✅ Static IP Configuration Success!");
      Serial.print("📡 IP: ");  Serial.println(ipAddress);
      Serial.print("🌐 Gateway: ");  Serial.println(gateway);
      Serial.print("🔒 Subnet: ");  Serial.println(netmask);
      Serial.print("🔍 Primary DNS: ");  Serial.println(primaryDNS);
      Serial.print("🔍 Secondary DNS: ");  Serial.println(secondaryDNS);
    }
  } else {
    Serial.println("🔄 Using DHCP mode...");
  }

  // เริ่มการเชื่อมต่อ WiFi
  // [FIX] Fallback if IotWebConf config is empty/corrupted
  if (strlen(ssid) == 0) {
    ssid = FALLBACK_WIFI_SSID;
    password = FALLBACK_WIFI_PASSWORD;
    Serial.println("⚠️ No saved WiFi config. Using fallback credentials...");
  }
  WiFi.begin(ssid, password);
}

// ===============================================
// WIFI SCANNING FUNCTIONS
// ===============================================

/**
   ===============================================
   WiFi Network Scanner
   [ WiFi  Config]
   ===============================================
*/
void Scanwifi() {
  numberOfNetworks = 0;
  yield();
  
  int scanResult = WiFi.scanNetworks(false, true); 
  
  if (scanResult < 0) {
    numberOfNetworks = 0;
    Serial.println("❌ WiFi scan failed");
    return;
  }
  numberOfNetworks = scanResult;
  if (numberOfNetworks > 5) {
    numberOfNetworks = 5;
  }
}

void Time_Count_Period1() {

  TA1   =  Period1_Timeduration_Min_int  ;
  TA2  =  Period1_Timeduration_Min_int  ;
  TA3  =  TA1 - 1 ;
  TA4  =  TA1 + TA3 ;
  TA5  =  TA4 - 1 ;
  TA6  =  TA1 + TA5 ;
  TA7  =  TA6 - 1 ;
  TA8  =  TA1 + TA7 ;
  TA9  =  TA8 - 1 ;
  TA10   =  TA1 + TA9 ;
  TA11   =  TA10 - 1  ;
  TA12   =  TA1 + TA11  ;
  TA13   =  TA12 - 1  ;
  TA14   =  TA1 + TA13  ;
  TA15   =  TA14 - 1  ;
  TA16   =  TA1 + TA15  ;
  TA17   =  TA16 - 1  ;
  TA18   =  TA1 + TA17  ;
  TA19   =  TA18 - 1  ;
  TA20   =  TA1 + TA19  ;
  TA21   =  TA20 - 1  ;
  TA22   =  TA1 + TA21  ;
  TA23   =  TA22 - 1  ;
  TA24   =  TA1 + TA23  ;
  TA25   =  TA24 - 1  ;
  TA26   =  TA1 + TA25  ;
  TA27   =  TA26 - 1  ;
  TA28   =  TA1 + TA27  ;
  TA29   =  TA28 - 1  ;
  TA30   =  TA1 + TA29  ;
  TA31   =  TA30 - 1  ;
  TA32   =  TA1 + TA31  ;
  TA33   =  TA32 - 1  ;
  TA34   =  TA1 + TA33  ;
  TA35   =  TA34 - 1  ;
  TA36   =  TA1 + TA35  ;
  TA37   =  TA36 - 1  ;
  TA38   =  TA1 + TA37  ;
  TA39   =  TA38 - 1  ;
  TA40   =  TA1 + TA39  ;

  // [FIX] Assign to P1 variable instead of Manual
  TA40_P1 = TA40; 

  totalMinutes1 = TA40;
  hours1 = totalMinutes1 / 60;
  minutes1 = totalMinutes1 % 60;
}




void Time_Count_Period2() {

  TA1   =  Period2_Timeduration_Min_int  ;
  TA2  =  Period2_Timeduration_Min_int  ;
  TA3  =  TA1 - 1 ;
  TA4  =  TA1 + TA3 ;
  TA5  =  TA4 - 1 ;
  TA6  =  TA1 + TA5 ;
  TA7  =  TA6 - 1 ;
  TA8  =  TA1 + TA7 ;
  TA9  =  TA8 - 1 ;
  TA10   =  TA1 + TA9 ;
  TA11   =  TA10 - 1  ;
  TA12   =  TA1 + TA11  ;
  TA13   =  TA12 - 1  ;
  TA14   =  TA1 + TA13  ;
  TA15   =  TA14 - 1  ;
  TA16   =  TA1 + TA15  ;
  TA17   =  TA16 - 1  ;
  TA18   =  TA1 + TA17  ;
  TA19   =  TA18 - 1  ;
  TA20   =  TA1 + TA19  ;
  TA21   =  TA20 - 1  ;
  TA22   =  TA1 + TA21  ;
  TA23   =  TA22 - 1  ;
  TA24   =  TA1 + TA23  ;
  TA25   =  TA24 - 1  ;
  TA26   =  TA1 + TA25  ;
  TA27   =  TA26 - 1  ;
  TA28   =  TA1 + TA27  ;
  TA29   =  TA28 - 1  ;
  TA30   =  TA1 + TA29  ;
  TA31   =  TA30 - 1  ;
  TA32   =  TA1 + TA31  ;
  TA33   =  TA32 - 1  ;
  TA34   =  TA1 + TA33  ;
  TA35   =  TA34 - 1  ;
  TA36   =  TA1 + TA35  ;
  TA37   =  TA36 - 1  ;
  TA38   =  TA1 + TA37  ;
  TA39   =  TA38 - 1  ;
  TA40   =  TA1 + TA39  ;

  // [FIX] Capture Period 2 total
  TA40_P2 = TA40;

  totalMinutes2 = TA40;
  hours2 = totalMinutes2 / 60;
  minutes2 = totalMinutes2 % 60;
}



void Time_Count_Manual() {

  TA1   =  Manual_Timeduration_Min_int  ;
  TA2  =  Manual_Timeduration_Min_int  ;
  TA3  =  TA1 - 1 ;
  TA4  =  TA1 + TA3 ;
  TA5  =  TA4 - 1 ;
  TA6  =  TA1 + TA5 ;
  TA7  =  TA6 - 1 ;
  TA8  =  TA1 + TA7 ;
  TA9  =  TA8 - 1 ;
  TA10   =  TA1 + TA9 ;
  TA11   =  TA10 - 1  ;
  TA12   =  TA1 + TA11  ;
  TA13   =  TA12 - 1  ;
  TA14   =  TA1 + TA13  ;
  TA15   =  TA14 - 1  ;
  TA16   =  TA1 + TA15  ;
  TA17   =  TA16 - 1  ;
  TA18   =  TA1 + TA17  ;
  TA19   =  TA18 - 1  ;
  TA20   =  TA1 + TA19  ;
  TA21   =  TA20 - 1  ;
  TA22   =  TA1 + TA21  ;
  TA23   =  TA22 - 1  ;
  TA24   =  TA1 + TA23  ;
  TA25   =  TA24 - 1  ;
  TA26   =  TA1 + TA25  ;
  TA27   =  TA26 - 1  ;
  TA28   =  TA1 + TA27  ;
  TA29   =  TA28 - 1  ;
  TA30   =  TA1 + TA29  ;
  TA31   =  TA30 - 1  ;
  TA32   =  TA1 + TA31  ;
  TA33   =  TA32 - 1  ;
  TA34   =  TA1 + TA33  ;
  TA35   =  TA34 - 1  ;
  TA36   =  TA1 + TA35  ;
  TA37   =  TA36 - 1  ;
  TA38   =  TA1 + TA37  ;
  TA39   =  TA38 - 1  ;
  TA40   =  TA1 + TA39  ;




  totalMinutes3 = TA40;
  hours3 = totalMinutes3 / 60;
  minutes3 = totalMinutes3 % 60;
}


/**
   ===============================================
   Telegram Message Builder
   ===============================================

    Telegram 
    ESP32 

   :
   - 
   - 
   -  WiFi (SSID, Password)
   -  IP (Static/Dynamic)
   -  Access Point
   - MAC Address
   - Bot Token  Group ID
   -  Reset 
   ===============================================
*/

void internetcheck() {

  // ===============================================
  // TIMER INTERVAL CALCULATION
  // ===============================================

  // ตรวจสอบและปรับ timer interval ตาม mode ปัจจุบัน
  unsigned long current_interval;
  switch (Internet_Check_Mode) {
    case 0:  // Fast Mode - ตรวจสอบทุก 2 วินาที
      current_interval = 2000;
      break;
    case 1:  // Normal Mode - ตรวจสอบทุก 6 วินาที
      current_interval = 6000;
      break;
    case 2:  // Slow Mode - ตรวจสอบทุก 10 วินาที
      current_interval = 10000;
      break;
    default:
      current_interval = 6000;
      break;
  }

  // ปรับ timer interval ถ้าต่างจากปัจจุบัน
  static unsigned long last_interval = 0;
  if (last_interval != current_interval) {
    timer.setInterval(current_interval, internetcheck);
    last_interval = current_interval;
  }

  // ===============================================
  // WIFI CONNECTION CHECK
  // ===============================================

  if (WiFi.status() == WL_CONNECTED) {  // เช็คการเชื่อมต่อ Wi-Fi

    // ===============================================
    // INTERNET CONNECTION TEST
    // ===============================================
    // หมายเหตุ: การตรวจสอบอินเทอร์เน็ตไม่ควรบล็อกการทำงานของ Web Config
    // Web Config ควรทำงานได้ทันทีที่ WiFi เชื่อมต่อแล้ว ไม่ต้องรออินเทอร์เน็ต

    WiFiClient client;
    const char* host = "www.google.com";  // ใช้ host ที่เชื่อถือได้

    // ตั้งค่า timeout สั้นๆ (2 วินาที) เพื่อไม่ให้บล็อกการทำงานนานเกินไป
    // เมื่อไม่มีอินเทอร์เน็ต จะ timeout เร็วและไม่บล็อก Web Config
    // ถ้าไม่ตั้ง timeout จะใช้ค่า default (~5-10 วินาที) ซึ่งจะบล็อก main loop นานเกินไป
    client.setTimeout(2000);  // 2 วินาที timeout

    esp_task_wdt_reset();  // รีเซ็ต watchdog ก่อน network operation ที่ใช้เวลานาน
    if (client.connect(host, 80)) {  // เชื่อมต่ออินเทอร์เน็ตผ่าน HTTP port 80
      // ===============================================
      // SUCCESSFUL CONNECTION
      // ===============================================

      Internet = 1;
      digitalWrite(LED_Internet, 1);  // เปิดไฟ LED แสดงการเชื่อมต่อ

      client.stop();  // ปิดการเชื่อมต่อ

      // อัปเดต IP Address ใน Blynk Dashboard เมื่ออินเทอร์เน็ตเชื่อมต่อได้
      if (BlynkConnected) {
        Blynk.virtualWrite(V102, WiFi.localIP().toString());
      }

      // นับจำนวนครั้งที่สำเร็จติดต่อกัน
      consecutive_internet_success++;
      consecutive_internet_fail = 0; // รีเซ็ตการนับความล้มเหลว

      // ปรับ mode ตามจำนวนครั้งที่สำเร็จ
      if (consecutive_internet_success >= 5 && Internet_Check_Mode == 0) {
        // เปลี่ยนจาก Fast Mode เป็น Normal Mode
        Internet_Check_Mode = 1;
        //Serial.println("🌐 Internet เชื่อมต่อเสถียร - เปลี่ยนเป็น Normal Mode (6 วินาที)");
      }

    } else {

      // ===============================================
      // FAILED CONNECTION
      // ===============================================

      Internet = 0;
      digitalWrite(LED_Internet, 0);
      // นับจำนวนครั้งที่ล้มเหลวติดต่อกัน
      consecutive_internet_fail++;
      consecutive_internet_success = 0; // รีเซ็ตการนับความสำเร็จ

      // ปรับ mode ตามจำนวนครั้งที่ล้มเหลว
      if (consecutive_internet_fail >= 2 && Internet_Check_Mode == 1) {
        // กลับไปใช้ Fast Mode เมื่อเชื่อมต่อล้มเหลว
        Internet_Check_Mode = 0;
        //Serial.println("⚠️ Internet เชื่อมต่อไม่เสถียร - เปลี่ยนเป็น Fast Mode (2 วินาที)");
      }
    }

  } else {

    // ===============================================
    // WIFI NOT CONNECTED
    // ===============================================

    // WiFi ไม่เชื่อมต่อ
    Internet = 0;
    digitalWrite(LED_Internet, 0);
    consecutive_internet_fail++;
    consecutive_internet_success = 0;


    // กลับไปใช้ Fast Mode เมื่อ WiFi ไม่เชื่อมต่อ
    if (Internet_Check_Mode == 1) {
      Internet_Check_Mode = 0;
      //Serial.println("📶 WiFi ไม่เชื่อมต่อ - เปลี่ยนเป็น Fast Mode (2 วินาที)");
    }
  }

  if (Internet  == 1 and Internet_Reset == 0 and Year > 2565) {
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

  if (Internet == 1 and Period1 == 1 and Time_run_once1 == 0) {
    Time_run_once1 = 1;
    if (Sys_Time_Select == 0) {
      t = rtc.getTime();
      message = "\n" + String("Period 1 Started");
      message += "\n" + String("RTC : ") + String(t.date) + String("/") + String(rtc.getMonthStr()) + String("/") + String((t.year) + 543);
      message += "\n" + String(t.hour) + String(":") + String(t.min) + String(":") + String(t.sec);
      bot1.sendMessage(Bot_Group, message, "");
    } else {
      message = "\n" + String("Period 1 Started");
      message += "\n" + String("NTP : ") + String(Day) + String("/") + String(Month) + String("/") + String(Year);
      message += "\n" + String(THour) + String(":") + String(Tmin) + String(":") + String(Tsec);
      bot1.sendMessage(Bot_Group, message, "");
    }
  }

  if (Internet == 1 and Period1 == 0 and Time_run_once1 == 1) {
    Time_run_once1 = 0;
    if (Sys_Time_Select == 0) {
      t = rtc.getTime();
      message = "\n" + String("Period 1 Stopped");
      message += "\n" + String("RTC : ") + String(t.date) + String("/") + String(rtc.getMonthStr()) + String("/") + String((t.year) + 543);
      message += "\n" + String(t.hour) + String(":") + String(t.min) + String(":") + String(t.sec);
      bot1.sendMessage(Bot_Group, message, "");
    } else {
      message = "\n" + String("Period 1 Stopped");
      message += "\n" + String("NTP : ") + String(Day) + String("/") + String(Month) + String("/") + String(Year);
      message += "\n" + String(THour) + String(":") + String(Tmin) + String(":") + String(Tsec);
      bot1.sendMessage(Bot_Group, message, "");
    }
  }

  if (Internet == 1 and Period2 == 1 and Time_run_once2 == 0) {
    Time_run_once2 = 1;
    if (Sys_Time_Select == 0) {
      t = rtc.getTime();
      message = "\n" + String("Period 2 Started");
      message += "\n" + String("RTC : ") + String(t.date) + String("/") + String(rtc.getMonthStr()) + String("/") + String((t.year) + 543);
      message += "\n" + String(t.hour) + String(":") + String(t.min) + String(":") + String(t.sec);
      bot1.sendMessage(Bot_Group, message, "");
    } else {
      message = "\n" + String("Period 2 Started");
      message += "\n" + String("NTP : ") + String(Day) + String("/") + String(Month) + String("/") + String(Year);
      message += "\n" + String(THour) + String(":") + String(Tmin) + String(":") + String(Tsec);
      bot1.sendMessage(Bot_Group, message, "");
    }
  }

  if (Internet == 1 and Period2 == 0 and Time_run_once2 == 1) {
    Time_run_once2 = 0;
    if (Sys_Time_Select == 0) {
      t = rtc.getTime();
      message = "\n" + String("Period 2 Stopped");
      message += "\n" + String("RTC : ") + String(t.date) + String("/") + String(rtc.getMonthStr()) + String("/") + String((t.year) + 543);
      message += "\n" + String(t.hour) + String(":") + String(t.min) + String(":") + String(t.sec);
      bot1.sendMessage(Bot_Group, message, "");
    } else {
      message = "\n" + String("Period 2 Stopped");
      message += "\n" + String("NTP : ") + String(Day) + String("/") + String(Month) + String("/") + String(Year);
      message += "\n" + String(THour) + String(":") + String(Tmin) + String(":") + String(Tsec);
      bot1.sendMessage(Bot_Group, message, "");
    }
  }

  if (Internet == 1 and Manual == 1 and Time_run_once3 == 0) {
    Time_run_once3 = 1;
    if (Sys_Time_Select == 0) {
      t = rtc.getTime();
      message = "\n" + String("Manual Mode Started");
      message += "\n" + String("RTC : ") + String(t.date) + String("/") + String(rtc.getMonthStr()) + String("/") + String((t.year) + 543);
      message += "\n" + String(t.hour) + String(":") + String(t.min) + String(":") + String(t.sec);
      bot1.sendMessage(Bot_Group, message, "");
    } else {
      message = "\n" + String("Manual Mode Started");
      message += "\n" + String("NTP : ") + String(Day) + String("/") + String(Month) + String("/") + String(Year);
      message += "\n" + String(THour) + String(":") + String(Tmin) + String(":") + String(Tsec);
      bot1.sendMessage(Bot_Group, message, "");
    }
  }

  if (Internet == 1 and Manual == 0 and Time_run_once3 == 1) {
    Time_run_once3 = 0;
    if (Sys_Time_Select == 0) {
      t = rtc.getTime();
      message = "\n" + String("Manual Mode Stopped");
      message += "\n" + String("RTC : ") + String(t.date) + String("/") + String(rtc.getMonthStr()) + String("/") + String((t.year) + 543);
      message += "\n" + String(t.hour) + String(":") + String(t.min) + String(":") + String(t.sec);
      bot1.sendMessage(Bot_Group, message, "");
    } else {
      message = "\n" + String("Manual Mode Stopped");
      message += "\n" + String("NTP : ") + String(Day) + String("/") + String(Month) + String("/") + String(Year);
      message += "\n" + String(THour) + String(":") + String(Tmin) + String(":") + String(Tsec);
      bot1.sendMessage(Bot_Group, message, "");
    }
  }

  // ---- v2.1.1: Slave offline/recovery alert (set by Task3 heartbeat) ----
  extern volatile bool g_slaveAlertPending;
  extern volatile uint8_t g_slaveAlertId;
  extern volatile bool g_slaveAlertIsDown;

  if (Internet == 1 && g_slaveAlertPending) {
    g_slaveAlertPending = false;
    if (g_slaveAlertIsDown) {
      message = "⚠️ ALERT: Slave Board " + String(g_slaveAlertId) + " OFFLINE\nตรวจสอบสาย RS485 และไฟเลี้ยง slave";
    } else {
      message = "✅ RECOVERED: Slave Board " + String(g_slaveAlertId) + " กลับมาออนไลน์แล้ว";
    }
    bot1.sendMessage(Bot_Group, message, "");
  }

  esp_task_wdt_reset();
}

//------------------------------------------------------------------------------------------------------------------------

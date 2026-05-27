/**
   ==================================================
   Task2: Date-Time Handler (NTP Only)
   ==================================================

    Task  NTP Server
    Core 0  ESP32
 
   Features:
   - Sync NTP 
   -  Reset 
   ==================================================
*/

// ==================================================
// Task2: Date-Time Handler (NTP Only)
// ==================================================

void Task2code(void * pvParameters) {
  DEBUG_PRINT("Task2 running on core ");
  DEBUG_PRINTLN(xPortGetCoreID());
  
  esp_task_wdt_add(NULL);  // NULL =  current task (Task2)

  for (;;) {
    // ==================================================
    // Periodic System Report (Every 5 seconds)
    // ==================================================
    static unsigned long lastStatusReport = 0;
    if (millis() - lastStatusReport > 5000) {
      lastStatusReport = millis();
      DEBUG_PRINTF("\n--- [TASK 2 STATUS] ---\n");
      DEBUG_PRINTF("Time: %02d:%02d:%02d | Date: %02d/%02d/%d\n", THour, Tmin, Tsec, Day, Month, Year);
      DEBUG_PRINTF("Internet: %s | Mode: %s\n", (Internet == 1 ? "OK" : "DISCONNECTED"), (Sys_Time_Select == 1 ? "NTP" : "RTC"));
      DEBUG_PRINTF("Triggers: P1=%02d:%02d, P2=%02d:%02d\n", Period1_Time_Start_Hr_int, Period1_Time_Start_Min_int, Period2_Time_Start_Hr_int, Period2_Time_Start_Min_int);
      DEBUG_PRINTF("Durations: P1=%d min, P2=%d min, TA40=%d\n", Period1_Timeduration_Min_int, Period2_Timeduration_Min_int, TA40);
      DEBUG_PRINTF("State: Period1=%d, Period2=%d, Manual=%d, StopReq=%d\n", Period1, Period2, Manual, stopInProgress);
      DEBUG_PRINTLN("-----------------------");
    }

    // ==================================================
    // NTP TIME SYNCHRONIZATION
    // ==================================================
    if (Internet == 1) {
      if (NTP_connect == 0) {
        timeClient.begin();
        NTP_connect = 1;
      }
      if (NTP_connect == 1) {
        timeClient.update();
        unsigned long unix_epoch = timeClient.getEpochTime();
        if (unix_epoch > 0) {
          Tsec = second(unix_epoch);
          Tmin = minute(unix_epoch);
          THour = hour(unix_epoch);
          Day = day(unix_epoch);
          Month = month(unix_epoch);
          Year_1 = year(unix_epoch);
          Year = Year_1 + 543;
          
          static int lastLogSec = -1;
          if (Tsec % 10 == 0 && Tsec != lastLogSec) {
            DEBUG_PRINTLN("🕐 NTP Time: " + String(Day) + "/" + String(Month) + "/" + String(Year) + 
                           " " + String(THour) + ":" + String(Tmin) + ":" + String(Tsec));
            lastLogSec = Tsec;
          }
        } else {
          t = rtc.getTime();
          THour = t.hour; Tmin = t.min; Tsec = t.sec; Day = t.date; Month = t.mon; Year_1 = t.year;
          Year = Year_1 + 543;
        }
      }
    } else {
      if (Sys_Time_Select == 1) {
        t = rtc.getTime();
        THour = t.hour; Tmin = t.min; Tsec = t.sec; Day = t.date; Month = t.mon; Year_1 = t.year;
        Year = Year_1 + 543;
      }
    }

    // ==================================================
    // Reset ESP32 Schedule
    // ==================================================
    bool resetEnabled = (BlynkRst_Hr_int != -1 && BlynkRst_Hr_int != 99) && 
                        (BlynkRst_Min_int != -1 && BlynkRst_Min_int != 99);
    
    if (resetEnabled && 
        BlynkRst_Hr_int >= 0 && BlynkRst_Hr_int <= 23 && 
        BlynkRst_Min_int >= 0 && BlynkRst_Min_int <= 59 &&
        THour == BlynkRst_Hr_int && Tmin == BlynkRst_Min_int && Tsec >= 0 && Tsec <= 3) {
      Serial.println("[TASK2] Scheduled reboot triggered");
      if (millis() < 60000) { Serial.println("[TASK2] Blocked - within 60s boot window"); }
      else {
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      ESP.restart();
      }
    }

    // ==================================================
    // RTC TIME SYNCHRONIZATION (NTP to RTC)
    // ==================================================
    t = rtc.getTime();

    if (rtcUpdated == 0 and Internet == 1 and Year > 2565) {
      rtc.setTime(THour, Tmin, Tsec);
      rtc.setDate(Day, Month, Year_1);
      rtcUpdated = 1;
      TimeSync_OK1 = 1;
    }

    // RTC SYNC before P1/P2
    int ph1, pm1, ph2, pm2;
    if (Period1_Time_Start_Min_int == 0) { pm1=59; ph1=(Period1_Time_Start_Hr_int==0)?23:Period1_Time_Start_Hr_int-1; } else { pm1=Period1_Time_Start_Min_int-1; ph1=Period1_Time_Start_Hr_int; }
    if (Period2_Time_Start_Min_int == 0) { pm2=59; ph2=(Period2_Time_Start_Hr_int==0)?23:Period2_Time_Start_Hr_int-1; } else { pm2=Period2_Time_Start_Min_int-1; ph2=Period2_Time_Start_Hr_int; }

    if (Internet==1 && Year>2565 && THour==ph1 && Tmin==pm1 && Tsec==0) { rtc.setTime(THour, Tmin, Tsec); rtc.setDate(Day, Month, Year_1); TimeSync_OK2=1; }
    if (Internet==1 && Year>2565 && THour==ph2 && Tmin==pm2 && Tsec==0) { rtc.setTime(THour, Tmin, Tsec); rtc.setDate(Day, Month, Year_1); TimeSync_OK3=1; }

    if (TimeSync_OK1) { BLYNK_WRITE_SAFE(V21, "RTC Sync Initial OK"); TimeSync_OK1=0; }
    if (TimeSync_OK2) { BLYNK_WRITE_SAFE(V21, "RTC Sync P1 OK"); TimeSync_OK2=0; }
    if (TimeSync_OK3) { BLYNK_WRITE_SAFE(V21, "RTC Sync P2 OK"); TimeSync_OK3=0; }

    // ==================================================
    // PUMP CONTROL - RTC/NTP Trigger Logic
    // ==================================================
    if (Sys_Time_Select == 0) {
      t = rtc.getTime();
      if ((t.year + 543) > 2565) { THour = t.hour; Tmin = t.min; Tsec = t.sec; Year = t.year + 543; }
    }

    // Manual Status
    if (Manual == 1 && BlynkRun_once1 == 0) {
      TA40 = Manual_Timeduration_Min_int;
      BLYNK_WRITE_SAFE(V24, String(TA40) + " ");
      BLYNK_WRITE_SAFE(V4, "Manual START");
      BlynkRun_once1 = 1;
    }
    if (Manual == 0 && BlynkRun_once1 == 1) { BlynkRun_once1 = 0; }

    // TRIGGER LOGIC
    if (Period1 == 0 && Period2 == 0 && Manual == 0 && stopInProgress == false) {
        
        // P1 TRIGGER
        if (Period1_Timeduration_Min_int > 0 && THour == Period1_Time_Start_Hr_int && Tmin == Period1_Time_Start_Min_int) {
            Serial.println("!!! TRIGGER: P1 START !!!");
            CountP1Min = 0; CountP1 = 0;
            Time_Count_Period1();
            TA40 = TA40_P1;
            if (TA40 <= 0) TA40 = Period1_Timeduration_Min_int * 21; 
            Period1 = 1;
            BLYNK_WRITE_SAFE(V4, "Period 1 Running");
            BLYNK_WRITE_SAFE(V24, String(TA40) + " ");
        }
        // P2 TRIGGER
        else if (Period2_Timeduration_Min_int > 0 && THour == Period2_Time_Start_Hr_int && Tmin == Period2_Time_Start_Min_int) {
            Serial.println("!!! TRIGGER: P2 START !!!");
            CountP1Min = 0; CountP1 = 0;
            Time_Count_Period2();
            TA40 = TA40_P2;
            if (TA40 <= 0) TA40 = Period2_Timeduration_Min_int * 21; 
            Period2 = 1;
            BLYNK_WRITE_SAFE(V4, "Period 2 Running");
            BLYNK_WRITE_SAFE(V24, String(TA40) + " ");
        }
    }

    // Resume Logic
    static bool p1R=false, p2R=false, mR=false;
    if (Period1==1 && CountP1Min>0 && !p1R) { BLYNK_WRITE_SAFE(V4, "Period1 RESUME"); p1R=true; }
    if (Period1==0) p1R=false;
    if (Period2==1 && CountP1Min>0 && !p2R) { BLYNK_WRITE_SAFE(V4, "Period2 RESUME"); p2R=true; }
    if (Period2==0) p2R=false;
    if (Manual==1 && CountP1Min>0 && !mR)  { BLYNK_WRITE_SAFE(V4, "Manual RESUME"); mR=true; }
    if (Manual==0) mR=false;

    // EEPROM SAVE
    if (Period1 != prevPeriod1) { EEPROM.put(240, (int)Period1); EEPROM.commit(); prevPeriod1 = Period1; }
    if (Period2 != prevPeriod2) { EEPROM.put(250, (int)Period2); EEPROM.commit(); prevPeriod2 = Period2; }
    if (Manual != prevManual)   { EEPROM.put(260, (int)Manual);  EEPROM.commit(); prevManual = Manual; }

    vTaskDelay(1000 / portTICK_PERIOD_MS);
    esp_task_wdt_reset();
  }
}

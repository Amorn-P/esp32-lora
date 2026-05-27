/**
   ==================================================
   Task4: Blynk Communication Handler
   ==================================================
   
    Task  Blynk
    Core 1  ESP32
   
   Features:
   -  Blynk
   -  Web Config
   -  (Button Debouncing)
   - 
   ==================================================
*/

void Task4code(void * pvParameters) {
  DEBUG_PRINT("Task4 running on core ");
  DEBUG_PRINTLN(xPortGetCoreID());
  
  // Task4  Watchdog Timer 
  esp_task_wdt_add(NULL);  // NULL =  current task (Task4)

  for (;;) {
    // ==================================================
    // Blynk.run()  Internet-check (google.com)
    // checkbox  Web Config : Sel_1_Blynk_Mode
    // ==================================================
    if (Sel_1_Blynk_Mode == 1 && WiFi.status() == WL_CONNECTED) {
      // [FIX] Take blynkMutex  Blynk.run()  Task2 Core0 write 
      if (blynkMutex != NULL && xSemaphoreTake(blynkMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        Blynk.run();
        xSemaphoreGive(blynkMutex);
      }
    }

    // Blynk  ( Blynk Mode) - only attempt every 5s to avoid blocking Core1
    static unsigned long lastBlynkConnect = 0;
    if (!Blynk.connected() && Sel_1_Blynk_Mode == 1 && WiFi.status() == WL_CONNECTED) {
      if (millis() - lastBlynkConnect > 5000) {
        lastBlynkConnect = millis();
        DEBUG_PRINTLN("Blynk disconnected, reconnecting...");
        esp_task_wdt_reset();
        Blynk.connect();
      }
    }



    // ===============================================
    // Web Config
    // ESP32  Apply  Web Config
    // ===============================================
    if (needReset) {
      Serial.println("[TASK4] needReset=TRUE - Rebooting after 1 second.");
      vTaskDelay(1000 / portTICK_PERIOD_MS);  // vTaskDelay  iotWebConf.delay() -  watchdog 
      ESP.restart();  // ESP32
    }

    // ===============================================
    // (Button Debouncing)
    // ===============================================
    // (debouncing)
    debouncer0.update();

    // ()
    if (debouncer0.fell()) {
      // ===============================================
      // // ===============================================
      // AP_MODE  FAST_MODE
      currentMode = (currentMode == AP_MODE) ? FAST_MODE : AP_MODE;

      // ===============================================
      // EEPROM
      // ===============================================
      EEPROM.put(500, currentMode);  // address 500
      esp_task_wdt_reset();  // watchdog  EEPROM.commit() 
      EEPROM.commit();               // // ===============================================
      // LED
      // ===============================================
      // LED  FAST_MODE  AP_MODE
      //digitalWrite(LED_FA_Mode, currentMode == FAST_MODE ? HIGH : LOW);

      // (Debug)
      DEBUG_PRINT(": ");
      DEBUG_PRINTLN(currentMode == AP_MODE ? "AP Mode" : "Fast Mode");

      // ===============================================
      // // ===============================================
      Serial.println("[TASK4] Mode change reboot triggered");
      if (millis() < 60000) { Serial.println("[TASK4] Blocked - within 60s boot window"); }
      else {
      vTaskDelay(2000 / portTICK_PERIOD_MS);  // vTaskDelay  delay() -  watchdog 
      ESP.restart();      // ESP32 
      }
    }

    vTaskDelay(50 / portTICK_PERIOD_MS);  // 50ms - faster yield for loop()
    esp_task_wdt_reset();  // Watchdog Timer
  }
}

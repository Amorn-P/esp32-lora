/**
   ==================================================
   Task5: Time Counter & Data Persistence
   ==================================================

    Task  (CountP1, CountP1Min)  Preferences
    Core 1  ESP32

   Features:
   -  (CountP1)  (CountP1Min)
   -  Preferences  1  ( CountP1 == 0)
   -  3 : Period1, Period2, Manual
   -  ESP32 Restart ( 1 )
   ==================================================
*/

// ==================================================
// Task5code
// ==================================================

void Task5code(void *pvParameters)
{
  DEBUG_PRINT("Task5 running on core ");
  DEBUG_PRINTLN(xPortGetCoreID());

  // Task5  Watchdog Timer 
  esp_task_wdt_add(NULL);  // NULL =  current task (Task5)

  for (;;)
  {
    // ==================================================
    // TIME COUNTING & DATA PERSISTENCE
    // // ==================================================
    if (Period1 == 1 || Period2 == 1 || Manual == 1) {

      // ==================================================
      // TIME COUNTING - 
      // ==================================================
      CountP1++;  // // 60  (CountP1 > 59)  0 
      if (CountP1 > 59) {
        CountP1 = 0;
        CountP1Min++;  // }
      }  // [FIX] Close if(CountP1 > 59)

      DEBUG_PRINT("Count: ");
      DEBUG_PRINT(CountP1);
      DEBUG_PRINT(" | Min: ");
      DEBUG_PRINTLN(CountP1Min);

      // [FIX] Save both CountP1 and CountP1Min every minute boundary
      // Also properly end() preferences after use
      if (CountP1 == 0) {
        if (preferencesMutex != NULL && xSemaphoreTake(preferencesMutex, pdMS_TO_TICKS(2000)) == pdTRUE) {
          preferences.begin("storage", false);
          preferences.putInt("CountP1", CountP1);
          preferences.putInt("CountP1Min", CountP1Min);  // [FIX] Save minute counter
          preferences.end();  // [FIX] Always close
          xSemaphoreGive(preferencesMutex);
        }
      }

    } //if (Period1 == 1 || Period2 == 1 || Manual == 1)

    // ==================================================
    // TASK DELAY & WATCHDOG RESET
    // ==================================================
    vTaskDelay(1000 / portTICK_PERIOD_MS);  // 1 
    esp_task_wdt_reset();                    // Watchdog Timer
  }
}

/**
   ==================================================
   Task8: Software Watchdog Timer
   ==================================================
   
    Task  Software Watchdog Timer
    Core 0  ESP32
   
   Features:
   - Software Watchdog Timer ( Hardware Watchdog  timeout)
   -  25  ( Hardware Watchdog 30 )
   - 
   ==================================================
*/

// ==================================================
// Software Watchdog Configuration
// ==================================================
#define SW_PROACTIVE_THRESHOLD 25000   // 25  → feed  ( Hardware Watchdog 30  5 )

// ==================================================
// Task8: Software Watchdog Timer (Minimal & Efficient)
// ==================================================
// - Proactive feed HW Watchdog  timeout 30 
// - Feed  ( feed  loop)
// - 
// - Stack size: 10,000 bytes ()
// - Priority: 1 ( Task )
// - Core: 0
void Task8code(void *pvParameters) {
  DEBUG_PRINT("Task8 running on core ");
  DEBUG_PRINTLN(xPortGetCoreID());
  
  // Task8  Watchdog Timer 
  esp_task_wdt_add(NULL);  // NULL =  current task (Task8)
  
  unsigned long last_feed_time = millis();
  
  for (;;) {
    unsigned long now = millis();
    unsigned long elapsed = now - last_feed_time;
    
    // ==================================================
    // Proactive feed  25 
    // ==================================================
    // Feed  ( feed  loop)
    // CPU 
    if (elapsed >= SW_PROACTIVE_THRESHOLD) {
      esp_task_wdt_reset();
      last_feed_time = now;
      
      DEBUG_PRINTLN("🔄 Software Watchdog: Hardware Watchdog reset");
    }
    
    // ==================================================
    // Task Maintenance
    // ==================================================
    // 2  ( ~2 )
    // elapsed  25 
    // CPU  vTaskDelay(100ms)
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

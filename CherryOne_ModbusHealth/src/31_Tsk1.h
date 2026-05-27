/**
   ==================================================
   Task1: IoT Web Configuration Handler
   ==================================================
   
    Task  IoT Web Configuration
    Core 1  ESP32
   
   Features:
   -  Web Configuration Loop
   - 
   -  Watchdog Timer
   ==================================================
*/
void Task1code(void * pvParameters) {
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());
  
  // Task1  Watchdog Timer 
  esp_task_wdt_add(NULL);  // NULL =  current task (Task1)

  for (;;) {
    iotWebConf.doLoop();  // Web Configuration Loop
    vTaskDelay(1 / portTICK_PERIOD_MS);  // 1ms
    esp_task_wdt_reset();  // Watchdog Timer
  }
}

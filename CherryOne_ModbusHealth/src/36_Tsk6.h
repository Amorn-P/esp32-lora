/**
  ==================================================
  Task6: Stop Worker (Non-blocking Blynk callback)
  ==================================================
  :
  -  (Modbus read/write )  BLYNK_WRITE()
     callback  Task  Blynk.run() (Task4)
      Blynk / 
  - Task6  worker  stop/verify 
*/

#ifndef _36_TSK6_H
#define _36_TSK6_H

// flag  ( set  BLYNK_WRITE(V0))
// 13_Param.h  include
extern volatile bool stopRequested;

void Task6code(void *pvParameters) {
  // Task6  Watchdog Timer 
  esp_task_wdt_add(NULL);

  for (;;) {
    if (stopRequested) {
      stopRequested = false;

      // debug  Blynk ( Serial)
      if (Sel_1_Blynk_Mode == 1) {
        Blynk.virtualWrite(V31, "STOP WORKER: start");
      }

      // stop/verify  Blynk callback  block Task4
      stopSystemAndVerifyAll(true);

      if (Sel_1_Blynk_Mode == 1) {
        Blynk.virtualWrite(V31, "STOP WORKER: done");
      }
    }

    vTaskDelay(50 / portTICK_PERIOD_MS);
    esp_task_wdt_reset();
  }
}

#endif

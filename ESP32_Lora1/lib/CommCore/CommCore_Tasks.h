/**
 * CommCore_Tasks.h - Communication Core FreeRTOS Tasks
 * Task1: IotWebConf doLoop (keep AP + web alive)
 * Task4: Blynk.run() + Blynk.connect() (throttled)
 */

#ifndef COMMCORE_TASKS_H
#define COMMCORE_TASKS_H

#include "CommCore_Params.h"

// Task handles (defined in CommCore.cpp)
extern TaskHandle_t CommTask1;
extern TaskHandle_t CommTask4;

// ============================================================
// TASK1: IotWebConf doLoop (Core 0)
// ============================================================
void Task1_CommCore(void *pvParameters) {
  esp_task_wdt_add(NULL);
  for (;;) {
    iotWebConf.doLoop();
    vTaskDelay(1 / portTICK_PERIOD_MS);
    esp_task_wdt_reset();
  }
}

// ============================================================
// TASK4: Blynk Communication (Core 1)
// ============================================================
void Task4_CommCore(void *pvParameters) {
  esp_task_wdt_add(NULL);
  static unsigned long lastBlynkConnect = 0;

  for (;;) {
    // Blynk.run() with WiFi guard
    if (Sel_1_Blynk_Mode == 1 && WiFi.status() == WL_CONNECTED) {
      if (blynkMutex != NULL && xSemaphoreTake(blynkMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        Blynk.run();
        xSemaphoreGive(blynkMutex);
      }
    }

    // Blynk.connect() throttled to every 5s
    if (!Blynk.connected() && Sel_1_Blynk_Mode == 1 && WiFi.status() == WL_CONNECTED) {
      if (millis() - lastBlynkConnect > 5000) {
        lastBlynkConnect = millis();
        esp_task_wdt_reset();
        Blynk.connect();
      }
    }

    // needReset handler
    if (needReset) {
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      ESP.restart();
    }

    vTaskDelay(50 / portTICK_PERIOD_MS);
    esp_task_wdt_reset();
  }
}

#endif // COMMCORE_TASKS_H

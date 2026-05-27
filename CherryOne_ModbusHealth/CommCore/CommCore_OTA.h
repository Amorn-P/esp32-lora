/**
 * CommCore_OTA.h - OTA Firmware Update via Blynk
 */

#ifndef COMMCORE_OTA_H
#define COMMCORE_OTA_H

#include "CommCore_Params.h"

// Forward declare app-specific stop function
extern void stopSystemAndVerifyAll(bool reportToBlynk);

void performOTAUpdate() {
  if (URL_Firmware.isEmpty()) {
    Blynk.virtualWrite(V122, "URL empty");
    return;
  }

  // Safety: stop all relays before OTA
  Blynk.virtualWrite(V122, "Stopping systems...");
  stopSystemAndVerifyAll(true);
  vTaskDelay(500 / portTICK_PERIOD_MS);

  HTTPClient http;
  http.setConnectTimeout(10000);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.begin(URL_Firmware);

  Update.onProgress([](size_t progress, size_t total) {
    esp_task_wdt_reset();
    static int last = -1;
    int pct = (progress * 100) / total;
    if (pct % 5 == 0 && pct != last) {
      Blynk.virtualWrite(V122, "OTA: " + String(pct) + "%");
      last = pct;
    }
  });

  esp_task_wdt_reset();
  int httpCode = http.GET();

  if (httpCode == 200) {
    int len = http.getSize();
    if (len > 0) {
      Blynk.virtualWrite(V122, "Size: " + String(len) + " bytes");
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      
      WiFiClient *client = http.getStreamPtr();
      size_t written = Update.writeStream(*client);

      if (written == len && Update.end()) {
        Blynk.virtualWrite(V122, "OTA OK - Rebooting");
        Blynk.syncAll();
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        ESP.restart();
      } else {
        Blynk.virtualWrite(V122, "OTA FAILED");
        Update.printError(Serial);
        esp_task_wdt_reset();
      }
    }
  } else {
    Blynk.virtualWrite(V122, "HTTP Error: " + String(httpCode));
    esp_task_wdt_reset();
  }
  http.end();
}

#endif // COMMCORE_OTA_H

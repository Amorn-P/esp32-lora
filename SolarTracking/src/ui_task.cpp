/**
 * @file ui_task.cpp
 * @brief FreeRTOS task that refreshes the LCD2004 I2C display.
 *        Shows: Target Az/El, Current Az/El, Wind Speed, System Mode.
 *        Uses the existing lcd_functions.h mutex-protected LCD helpers.
 */

#include "ui_task.h"
#include "config.h"
#include "lcd_functions.h"
#include <LiquidCrystal_I2C.h>
#include <esp_task_wdt.h>
#include <stdio.h>

/**
 * @brief Determine system mode string from state flags.
 */
static const char* system_mode_string(bool wind_alarm, bool sensor_fault, bool rtc_ok) {
    if (!rtc_ok) {
        return "RTC FLT";
    } else if (wind_alarm) {
        return "STOW  ";
    } else if (sensor_fault) {
        return "WND FLT";
    } else {
        return "TRACK ";
    }
}


// ============================================================
// FreeRTOS UI Task
// ============================================================
void ui_task(void *pvParameters) {
    (void)pvParameters;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(UI_UPDATE_INTERVAL_MS);

    Serial.println("[UITask] Running...");

    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        esp_task_wdt_reset();

        // --- Snapshot system state under mutex ---
        float cur_el, cur_az, tgt_el, tgt_az, wind_kph;
        bool  wind_alarm, sensor_fault, rtc_ok;

        if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
            cur_el       = sysState.current_elevation;
            cur_az       = sysState.current_azimuth;
            tgt_el       = sysState.target_elevation;
            tgt_az       = sysState.target_azimuth;
            wind_kph     = sysState.wind_speed_kph;
            wind_alarm   = sysState.wind_alarm;
            sensor_fault = sysState.wind_sensor_fault;
            rtc_ok       = sysState.rtc_valid;
            xSemaphoreGive(stateMutex);
        } else {
            continue;
        }

        // --- Format Display Lines ---
        // Line 0: Target  Az:XXX.X El:XX.X
        // Line 1: Current Az:XXX.X El:XX.X
        // Line 2: Wind: XX.X km/h
        // Line 3: Mode: XXXX

        char line0[21], line1[21], line2[21], line3[21];

        snprintf(line0, sizeof(line0), "Tgt Az:%06.1f El:%04.1f", tgt_az, tgt_el);
        snprintf(line1, sizeof(line1), "Cur Az:%06.1f El:%04.1f", cur_az, cur_el);
        snprintf(line2, sizeof(line2), "Wind: %05.1f km/h", wind_kph);
        snprintf(line3, sizeof(line3), "Mode: %s", system_mode_string(wind_alarm, sensor_fault, rtc_ok));

        // --- Write to LCD (mutex-protected inside lcd_functions) ---
        showFourLineMessage(line0, line1, line2, line3);
    }
}

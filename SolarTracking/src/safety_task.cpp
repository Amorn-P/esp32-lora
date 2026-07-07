/**
 * @file safety_task.cpp
 * @brief FreeRTOS task that polls a Modbus wind speed sensor via RS485 (MAX3485).
 *        Pins: RX=16, TX=17, EN=4 (DE/RE combined).
 *        Sets sysState.wind_alarm / wind_sensor_fault under mutex.
 *        Wind hysteresis: arm at 40 kph, clear at 30 kph with 60-sec hold.
 *        Uses ModbusMaster library.
 */

#include "safety_task.h"
#include "config.h"
#include <ModbusMaster.h>
#include <esp_task_wdt.h>

static ModbusMaster windSensor;
static bool sensor_initialized = false;

// --- Wind alarm hysteresis state ---
static bool  alarm_armed        = false;
static unsigned long clear_since_ms = 0;     // timestamp when wind first dropped below clear threshold

// ============================================================
// Public: Initialize Wind Sensor (RS485 + Modbus on MAX3485)
// ============================================================
void init_wind_sensor() {
    // GPIO4 as EN (DE+RE combined): HIGH = transmit, LOW = receive
    pinMode(RS485_EN_PIN, OUTPUT);
    digitalWrite(RS485_EN_PIN, LOW);  // Start in receive mode

    // Initialize Serial2 for RS485
    Serial2.begin(MODBUS_BAUD, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);

    // Configure ModbusMaster
    windSensor.begin(WIND_SENSOR_SLAVE_ID, Serial2);
    windSensor.preTransmission([]() {
        digitalWrite(RS485_EN_PIN, HIGH);
    });
    windSensor.postTransmission([]() {
        digitalWrite(RS485_EN_PIN, LOW);
    });

    sensor_initialized = true;
    Serial.println("[SafetyTask] RS485 initialized: RX=16 TX=17 EN=4, baud=9600");
}


// ============================================================
// FreeRTOS Safety Task
// ============================================================
void safety_task(void *pvParameters) {
    (void)pvParameters;

    if (!sensor_initialized) {
        init_wind_sensor();
    }

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(1000); // Poll every 1 second

    Serial.println("[SafetyTask] Running...");

    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        esp_task_wdt_reset();

        // --- Poll Wind Speed via Modbus ---
        uint8_t result = windSensor.readHoldingRegisters(WIND_SPEED_REGISTER, 1);

        float wind_kph = 0.0f;

        if (result == windSensor.ku8MBSuccess) {
            uint16_t raw = windSensor.getResponseBuffer(0);
            float wind_ms = raw * 0.1f;
            wind_kph = wind_ms * 3.6f;
        }
        // On failure: wind_kph stays 0.0 (sensor fault logic handles it)

        // ============================================
        // Wind Alarm Hysteresis (3-state)
        //   ARM:   wind > 40 kph
        //   CLEAR: wind < 30 kph for 60 seconds
        //   Otherwise: hold current state
        // ============================================
        if (!alarm_armed) {
            // Currently tracking — check if we need to arm
            if (wind_kph > STOW_WIND_SPEED_KPH) {
                alarm_armed = true;
                clear_since_ms = 0;
            }
        } else {
            // Currently armed — check if we can clear
            if (wind_kph < 30.0f) {
                if (clear_since_ms == 0) {
                    clear_since_ms = millis();
                } else if (millis() - clear_since_ms >= 60000) {
                    alarm_armed = false;
                }
            } else {
                // Wind rose back above clear threshold — reset timer
                clear_since_ms = 0;
            }
        }

        // --- Update Global State under Mutex ---
        if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {

            if (result == windSensor.ku8MBSuccess) {
                sysState.wind_speed_kph   = wind_kph;
                sysState.wind_sensor_fault = false;
                sysState.wind_alarm        = alarm_armed;
            } else {
                // Modbus timeout / error
                sysState.wind_sensor_fault = true;
                sysState.wind_speed_kph    = 0.0f;
                sysState.wind_alarm        = false; // Fault-tolerant: allow normal tracking
            }

            xSemaphoreGive(stateMutex);
        }
    }
}

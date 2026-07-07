/**
 * @file motor_task.cpp
 * @brief FreeRTOS motor control task using LEDC PWM.
 *        Pins: PAN=25,26 / TILT=27,14 (from User_config.h).
 *        
 *        Priority logic:
 *          1. If wind_alarm, drive TILT down to elevation <= 2.0° (stow flat). Stop PAN.
 *          2. Otherwise, compare target vs current with DEAD_BAND_DEGREES.
 *             Ramp PWM smoothly using ledcWrite().
 */

#include "motor_task.h"
#include "config.h"
#include <esp_task_wdt.h>

// --- Internal motor state ---
static int pan_duty  = 0;
static int tilt_duty = 0;

// ============================================================
// Public: Initialize Motor Drivers (LEDC PWM)
// ============================================================
void init_motor_drivers() {
    // Configure LEDC channels
    // Channel 0: PAN Motor Pin 1 (25)
    ledcSetup(PAN_CHANNEL_1, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(PAN_MOTOR_PIN_1, PAN_CHANNEL_1);

    // Channel 1: PAN Motor Pin 2 (26)
    ledcSetup(PAN_CHANNEL_2, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(PAN_MOTOR_PIN_2, PAN_CHANNEL_2);

    // Channel 2: TILT Motor Pin 1 (27)
    ledcSetup(TILT_CHANNEL_1, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(TILT_MOTOR_PIN_1, TILT_CHANNEL_1);

    // Channel 3: TILT Motor Pin 2 (14)
    ledcSetup(TILT_CHANNEL_2, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(TILT_MOTOR_PIN_2, TILT_CHANNEL_2);

    // Ensure motors are stopped
    stop_all_motors();

    Serial.println("[MotorTask] LEDC PWM initialized on pins 25, 26, 27, 14");
}

// ============================================================
// Public: Stop All Motors
// ============================================================
void stop_all_motors() {
    ledcWrite(PAN_CHANNEL_1, 0);
    ledcWrite(PAN_CHANNEL_2, 0);
    ledcWrite(TILT_CHANNEL_1, 0);
    ledcWrite(TILT_CHANNEL_2, 0);
    pan_duty  = 0;
    tilt_duty = 0;
}

// ============================================================
// Internal: Ramp Helper
// ============================================================

/**
 * @brief Smoothly ramp a duty value toward a target with fixed step size.
 * @param current Current duty value
 * @param target Desired duty value
 * @return New duty value one step closer to target
 */
static int ramp_duty(int current, int target) {
    if (current < target) {
        int next = current + PWM_RAMP_STEP;
        return (next > target) ? target : next;
    } else if (current > target) {
        int next = current - PWM_RAMP_STEP;
        return (next < target) ? target : next;
    }
    return current;
}

/**
 * @brief Drive the PAN motor: positive duty = clockwise, negative = counter-clockwise.
 * @param duty Signed duty (-255..255), positive = forward, negative = reverse
 */
static void drive_pan(int duty) {
    if (duty > 0) {
        ledcWrite(PAN_CHANNEL_1, duty);
        ledcWrite(PAN_CHANNEL_2, 0);
    } else if (duty < 0) {
        ledcWrite(PAN_CHANNEL_1, 0);
        ledcWrite(PAN_CHANNEL_2, -duty);
    } else {
        ledcWrite(PAN_CHANNEL_1, 0);
        ledcWrite(PAN_CHANNEL_2, 0);
    }
}

/**
 * @brief Drive the TILT motor: positive duty = tilt up, negative = tilt down.
 * @param duty Signed duty (-255..255), positive = tilt up, negative = tilt down
 */
static void drive_tilt(int duty) {
    if (duty > 0) {
        ledcWrite(TILT_CHANNEL_1, duty);
        ledcWrite(TILT_CHANNEL_2, 0);
    } else if (duty < 0) {
        ledcWrite(TILT_CHANNEL_1, 0);
        ledcWrite(TILT_CHANNEL_2, -duty);
    } else {
        ledcWrite(TILT_CHANNEL_1, 0);
        ledcWrite(TILT_CHANNEL_2, 0);
    }
}


// ============================================================
// FreeRTOS Motor Task
// ============================================================
void motor_task(void *pvParameters) {
    (void)pvParameters;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(100); // 10 Hz control loop

    Serial.println("[MotorTask] Running...");

    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        esp_task_wdt_reset();

        // --- Snapshot system state under mutex ---
        float cur_el, cur_az, tgt_el, tgt_az;
        bool  wind_alarm, sensor_fault;

        if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
            cur_el      = sysState.current_elevation;
            cur_az      = sysState.current_azimuth;
            tgt_el      = sysState.target_elevation;
            tgt_az      = sysState.target_azimuth;
            wind_alarm  = sysState.wind_alarm;
            sensor_fault = sysState.wind_sensor_fault;
            xSemaphoreGive(stateMutex);
        } else {
            // Should never happen with portMAX_DELAY, but safety fallback
            continue;
        }

        int target_pan_duty  = 0;
        int target_tilt_duty = 0;

        // ============================================
        // Priority 1: Wind Stow
        // ============================================
        if (wind_alarm) {
            // Drive TILT motor down until elevation <= 2.0 degrees (flat/stow)
            if (cur_el > 2.0f) {
                // Need to tilt down (reverse)
                target_tilt_duty = -PWM_MAX_DUTY; // full reverse
            } else {
                target_tilt_duty = 0; // already stowed
            }
            // Stop PAN completely during stow
            target_pan_duty = 0;
        }
        // ============================================
        // Priority 2: Sun Tracking
        // ============================================
        else {
            // --- Tilt (Elevation) Control ---
            float tilt_error = tgt_el - cur_el;
            if (fabsf(tilt_error) > DEAD_BAND_DEGREES) {
                if (tilt_error > 0) {
                    // Target is higher, tilt up
                    target_tilt_duty = PWM_MAX_DUTY;
                } else {
                    // Target is lower, tilt down
                    target_tilt_duty = -PWM_MAX_DUTY;
                }
            } else {
                target_tilt_duty = 0; // within dead band
            }

            // --- Pan (Azimuth) Control ---
            // Compute shortest angular distance
            float pan_error = tgt_az - cur_az;
            // Normalize to [-180, 180]
            if (pan_error > 180.0f)  pan_error -= 360.0f;
            if (pan_error < -180.0f) pan_error += 360.0f;

            if (fabsf(pan_error) > DEAD_BAND_DEGREES) {
                if (pan_error > 0) {
                    // Positive error -> clockwise
                    target_pan_duty = PWM_MAX_DUTY;
                } else {
                    // Negative error -> counter-clockwise
                    target_pan_duty = -PWM_MAX_DUTY;
                }
            } else {
                target_pan_duty = 0; // within dead band
            }
        }

        // --- Ramp PWM Smoothly ---
        pan_duty  = ramp_duty(pan_duty,  target_pan_duty);
        tilt_duty = ramp_duty(tilt_duty, target_tilt_duty);

        // --- Apply PWM ---
        drive_pan(pan_duty);
        drive_tilt(tilt_duty);
    }
}

#include <Arduino.h>
#include <Wire.h>
#include <esp_task_wdt.h>
#include "config.h"
#include "system_state.h"
#include "sensor_task.h"
#include "astro_task.h"
#include "safety_task.h"
#include "motor_task.h"
#include "ui_task.h"
#include "lcd_functions.h"
#include "rtc_functions.h"

// --- Global System State Definition ---
SystemState sysState = {
    .current_elevation  = 0.0f,
    .current_azimuth    = 0.0f,
    .target_elevation   = 0.0f,
    .target_azimuth     = 0.0f,
    .wind_speed_kph     = 0.0f,
    .wind_sensor_fault  = false,
    .wind_alarm         = false,
    .rtc_valid          = false
};

// --- Global Mutex Definitions ---
SemaphoreHandle_t stateMutex = NULL;

// --- LCD Object Definition (extern'd in lcd_functions.h) ---
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

// Task handles
TaskHandle_t SensorTaskHandle = NULL;
TaskHandle_t AstroTaskHandle  = NULL;
TaskHandle_t SafetyTaskHandle = NULL;
TaskHandle_t MotorTaskHandle  = NULL;
TaskHandle_t UITaskHandle     = NULL;

void setup() {
    // CRITICAL: Wokwi sometimes fails to attach Serial to the terminal if it's too fast.
    // Adding a short delay before begin ensures the terminal catches the very first print.
    delay(100);
    Serial.begin(115200);
    while(!Serial) { delay(10); } // Wait for serial to attach
    
    Serial.println("\n\n--- WOKWI SERIAL ATTACHED ---");
    Serial.println("Starting Solar Tracker 2-Axis (Astronomical)");
    Serial.flush();

    // Initialize core hardware
    Wire.begin();

    // --- Initialize Watchdog ---
    esp_task_wdt_init(10, true);     // 10s timeout, panic + reboot on expiry
    esp_task_wdt_add(NULL);          // Watch current task (setup/loop)

    // --- Create Mutexes ---
    stateMutex = xSemaphoreCreateMutex();

    if (!stateMutex) {
        Serial.println("CRITICAL: Mutex creation failed. System Halted.");
        while(1) { delay(1000); }
    }

    // --- Initialize Peripherals ---
    initLCD();
    
    // Test direct print after init to ensure LCD library is fully responsive
    lcd.setCursor(0, 1);
    lcd.print("LCD Boot OK");
    
    // We are flushing the serial buffer before the delay to ensure it prints to Wokwi terminal
    Serial.flush();
    delay(50);

    Serial.println("LCD init passed, moving to RTC...");
    Serial.flush();
    initRTC();
    Serial.println("RTC init passed, moving to Wind...");
    init_wind_sensor();
    Serial.println("Wind init passed, moving to Motors...");
    init_motor_drivers();
    Serial.println("Motor init passed, moving to Sensors...");

    if (!init_sensors()) {
        Serial.println("WARNING: Sensor Init Failed. Continuing without position feedback.");
        // Non-critical: allow system to run with astro tracking only
    }

    Serial.println("System Initializing Complete - Spawning Tasks");

    // ======================================================
    // Create FreeRTOS Tasks (locked configuration)
    // ======================================================

    // Sensor task: 20 Hz, Priority 3 (highest — needs fresh data)
    xTaskCreatePinnedToCore(
        sensor_task,
        "Sensor_Task",
        4096,
        NULL,
        3,
        &SensorTaskHandle,
        1  // Core 1 (App Core)
    );

    // Astro task: 0.5 Hz, Priority 2
    xTaskCreatePinnedToCore(
        astro_task,
        "Astro_Task",
        4096,
        NULL,
        2,
        &AstroTaskHandle,
        1
    );

    // Safety task: 1 Hz, Priority 2 (wind monitoring is important)
    xTaskCreatePinnedToCore(
        safety_task,
        "Safety_Task",
        4096,
        NULL,
        2,
        &SafetyTaskHandle,
        1
    );

    // Motor task: 10 Hz, Priority 1 (acts on data from higher-priority tasks)
    xTaskCreatePinnedToCore(
        motor_task,
        "Motor_Task",
        4096,
        NULL,
        1,
        &MotorTaskHandle,
        1
    );

    // UI task: ~1 Hz, Priority 4
    xTaskCreatePinnedToCore(
        ui_task,
        "UI_Task",
        4096,
        NULL,
        1,
        &UITaskHandle,
        1
    );

    Serial.println("All tasks created. System operational.");
}

void loop() {
    // Empty. FreeRTOS tasks handle everything.
    vTaskDelete(NULL);
}

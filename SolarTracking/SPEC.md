# System Specification: ESP32 2-Axis Astronomical Solar Tracker

## 1. System Overview
A robust, closed-loop 2-axis solar tracking system driven by astronomical calculations rather than light-dependent resistors (LDRs). The system calculates the exact position of the sun based on Time (RTC) and Location (Lat/Lon). It then reads the actual physical position of the panel using an IMU/Compass on the frame, and uses BTS7960 motor drivers to align the panel to the sun. An RS485 Modbus wind sensor provides severe-weather protection.

## 2. Hardware Architecture & Pin Mapping (Proposed)
*Note: Pin map is conceptual for Wokwi/Code generation and can be adapted by DAD during physical wiring.*

### I2C Bus (Shared) - Pins 21 (SDA), 22 (SCL)
*   **DS3231 RTC:** I2C Address `0x68`
*   **MPU6050 (Tilt):** I2C Address `0x68` (Note: Must change AD0 pin to 3.3V to change address to `0x69` to avoid conflict with RTC)
*   **QMC5883L (Azimuth):** I2C Address `0x0D`
*   **LCD 2004 (UI):** I2C Address `0x27`

### RS485 Bus (UART2) - Modbus Wind Sensor
*   **RX2:** Pin 16
*   **TX2:** Pin 17
*   **DE/RE (Control):** Pin 4

### Motor Drivers (BTS7960)
*   **Pan (Slew Drive) - BTS7960 #1:**
    *   L_PWM: Pin 25
    *   R_PWM: Pin 26
    *   L_EN / R_EN: Tied to 3.3V (Always enabled)
*   **Tilt (Actuator) - BTS7960 #2:**
    *   L_PWM: Pin 27
    *   R_PWM: Pin 14
    *   L_EN / R_EN: Tied to 3.3V (Always enabled)

## 3. FreeRTOS Task Architecture

### Task 1: `sensor_task` (The Reality)
*   **Rate:** 10Hz (100ms)
*   **Action:** Reads MPU6050 (calculates Pitch/Elevation) and QMC5883L (calculates Heading/Azimuth). Applies moving average filter to smooth vibrations. Updates global state variables.

### Task 2: `astro_task` (The Target)
*   **Rate:** 1Hz (1000ms)
*   **Action:** Reads DS3231 RTC. Uses Solar Position Algorithm library (e.g., `SolarPosition.h`) with hardcoded farm Lat/Lon to calculate target Sun Azimuth and Sun Elevation. Updates global target variables.

### Task 3: `safety_task` (Wind Monitor)
*   **Rate:** 0.5Hz (2000ms)
*   **Action:** Polls Modbus wind speed sensor via RS485. 
*   **Logic:**
    *   If Timeout/No Response -> Set `WIND_FAULT = TRUE` (Unplugged state).
    *   If Speed > `STOW_THRESHOLD` -> Set `WIND_ALARM = TRUE`.
    *   Else -> Clear flags.

### Task 4: `motor_task` (The Control Loop)
*   **Rate:** 5Hz (200ms)
*   **Action:** Compares Target vs Reality.
*   **Logic Rules:**
    *   **Priority 1 (Stow):** If `WIND_ALARM == TRUE`, override all targets. Set Target Elevation = 0° (Flat).
    *   **Priority 2 (Fault Tolerant):** If `WIND_FAULT == TRUE`, ignore wind data, continue normal sun tracking.
    *   **Deadband:** Only move if error > 2° (prevents micro-jittering the motors).
    *   **Soft Start:** Use `ledcWrite()` to ramp up PWM signals to BTS7960 for smooth movement.

### Task 5: `ui_task` (LCD Display)
*   **Rate:** 1Hz (1000ms)
*   **Action:** Updates 2004 LCD.
    *   Line 1: `Sun: Az 180 El 45`
    *   Line 2: `Pan: Az 178 El 44`
    *   Line 3: `Wind: 12 km/h` (Or `Wind: UNPLUGGED!`)
    *   Line 4: `Mode: TRACKING` (Or `Mode: WIND STOW`)

## 4. Calibration Requirements (Future)
The code MUST include a calibration mode for the QMC5883L to account for Hard Iron interference from the metal solar frame. This will be a specialized function executed during initial installation.

## 5. DAD's Physical Rules
*   Do NOT generate `connections` array in Wokwi `diagram.json`. Parts only.
*   Keep `main.cpp` clean. All tasks must be modularized.

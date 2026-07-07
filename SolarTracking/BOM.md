# ESP32 2-Axis Astronomical Solar Tracker
## Bill of Materials (BOM)

### 1. Core Processing
* **ESP32 Development Board** (NodeMCU-32S or equivalent)
  * *Purpose:* Main controller, astronomical math calculations, FreeRTOS task scheduling.

### 2. Positioning & Time Sensors (The Target & The Reality)
* **DS3231 RTC Module (I2C)**
  * *Purpose:* Provides exact time/date for the Solar Position Algorithm. Highly accurate over long periods.
* **MPU6050 Accelerometer/Gyroscope (I2C)**
  * *Purpose:* Mounted on the panel frame. Measures absolute **Elevation/Tilt** relative to gravity.
* **QMC5883L Magnetometer (I2C)**
  * *Purpose:* Mounted on the panel frame. Acts as a digital compass to measure absolute **Azimuth/Heading**.

### 3. Motor Control (Actuation)
* **2x BTS7960 43A High-Power Motor Drivers**
  * *Purpose:* Drives the heavy 12V/24V DC motors for Pan and Tilt. Capable of PWM for smooth, non-jerky starts/stops to protect the mechanical gearing.
* **Linear Actuator (12V/24V DC)**
  * *Purpose:* Controls panel **Tilt (Elevation)**.
* **Slew Drive / Gear Motor (12V/24V DC)**
  * *Purpose:* Controls panel **Pan (Azimuth)**.

### 4. Safety & Fail-safes
* **RS485 Modbus RTU Wind Speed Sensor (Anemometer)**
  * *Purpose:* Detects high wind speeds to trigger horizontal "Stow Mode". Modbus allows reliable "unplugged" detection.
* **MAX485 / RS485 to TTL Module**
  * *Purpose:* Interfaces the industrial Modbus wind sensor with the ESP32 UART pins.

### 5. User Interface
* **2004 LCD Display (20x4) with I2C Backpack**
  * *Purpose:* Displays current Azimuth, Elevation, Sun Target, Wind Speed, and critical warnings (e.g., "WARN: Check Anemometer" if Modbus times out).

### 6. Power Supply
* **Buck Converter (LM2596 or similar)**
  * *Purpose:* Steps down 12V/24V system power to a clean 5V to power the ESP32 and I2C peripherals. (Do NOT power the BTS7960 motors from this).

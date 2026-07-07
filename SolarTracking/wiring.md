# wiring.md — ESP32 2-Axis Astronomical Solar Tracker

**Version:** 1.0.0  |  **Date:** 2026-06-08  |  **Linked SPEC:** SPEC.md

> **Purpose:** Single reference for DAD at the bench. All GPIO pins, component connections, and hardware wiring in one place.

---

## 1. MCU BOARD

| Parameter | Value |
|-----------|-------|
| **Board** | ESP32 NodeMCU-32S (or DOIT DevKit V1 equivalent) |
| **Power Input** | 5V via buck converter from 12V/24V system |
| **GPIO Voltage** | 3.3V — do NOT feed 5V to any GPIO |

---

## 2. GPIO PINOUT

| GPIO | Function | Direction | Component | Notes |
|------|----------|-----------|-----------|-------|
| 21 | I2C SDA | I/O | ALL I2C devices | Shared bus — 4 devices |
| 22 | I2C SCL | OUTPUT | ALL I2C devices | Shared bus |
| 16 | RS485 RX | INPUT | MAX485 module | UART2 RXD2 |
| 17 | RS485 TX | OUTPUT | MAX485 module | UART2 TXD2 |
| 4 | RS485 DE/RE | OUTPUT | MAX485 module | Direction control (HIGH=TX, LOW=RX) |
| 25 | PWM L_PWM | OUTPUT | BTS7960 #1 (Pan) | Pan motor forward/left |
| 26 | PWM R_PWM | OUTPUT | BTS7960 #1 (Pan) | Pan motor backward/right |
| 27 | PWM L_PWM | OUTPUT | BTS7960 #2 (Tilt) | Tilt motor up |
| 14 | PWM R_PWM | OUTPUT | BTS7960 #2 (Tilt) | Tilt motor down |

---

## 3. I2C BUS (Shared: GPIO 21 SDA, GPIO 22 SCL)

| Device | Address | Pull-ups | Notes |
|--------|---------|----------|-------|
| DS3231 RTC | 0x68 | External 4.7kΩ to 3.3V | Time source for solar position algorithm |
| MPU6050 IMU | **0x69** | Shared bus | **AD0 pin MUST be tied to 3.3V** to change from default 0x68 (conflicts with DS3231) |
| QMC5883L Magnetometer | 0x0D | Shared bus | Digital compass for azimuth feedback |
| LCD 2004 (I2C) | 0x27 | Shared bus | 20x4 character display for system UI |

> **⚠️ ADDRESS CONFLICT WARNING:** MPU6050 defaults to 0x68, same as DS3231 RTC. Solder or jumper the MPU6050 AD0 pin to 3.3V to change its address to 0x69. Both devices will fail silently if this is not done.

---

## 4. COMPONENT CONNECTIONS

### 4.1 DS3231 RTC Module

| Pin | Connect To | Notes |
|-----|-----------|-------|
| VCC | 3.3V | |
| GND | GND | |
| SDA | GPIO 21 | I2C data — shared bus |
| SCL | GPIO 22 | I2C clock — shared bus |

### 4.2 MPU6050 Accelerometer/Gyroscope

| Pin | Connect To | Notes |
|-----|-----------|-------|
| VCC | 3.3V | |
| GND | GND | |
| SDA | GPIO 21 | |
| SCL | GPIO 22 | |
| AD0 | **3.3V** | **MANDATORY** — changes I2C address to 0x69 |
| INT | NC | Not used |

### 4.3 QMC5883L Magnetometer (Digital Compass)

| Pin | Connect To | Notes |
|-----|-----------|-------|
| VCC | 3.3V | |
| GND | GND | |
| SDA | GPIO 21 | |
| SCL | GPIO 22 | |
| DRDY | NC | Not used |

### 4.4 LCD 2004 (20x4) with I2C Backpack

| Pin | Connect To | Notes |
|-----|-----------|-------|
| VCC | 5V | Backpack typically runs on 5V |
| GND | GND | |
| SDA | GPIO 21 | 5V-tolerant on ESP32 I2C |
| SCL | GPIO 22 | 5V-tolerant on ESP32 I2C |

**Display Layout (4 lines):**
```
Line 1: Sun: Az 180 El 45     (Target sun position)
Line 2: Pan: Az 178 El 44     (Actual panel position)
Line 3: Wind: 12 km/h         (Or "Wind: UNPLUGGED!")
Line 4: Mode: TRACKING        (Or "Mode: WIND STOW")
```

### 4.5 MAX485 RS485 Module (Wind Sensor)

| Pin | Connect To | Notes |
|-----|-----------|-------|
| VCC | 5V | |
| GND | GND | |
| RO | GPIO 16 (RXD2) | UART2 RX |
| DI | GPIO 17 (TXD2) | UART2 TX |
| RE | GPIO 4 | LOW = receive mode |
| DE | GPIO 4 | HIGH = transmit mode (RE + DE tied together) |
| A(+) | Anemometer A | Twisted pair |
| B(-) | Anemometer B | Twisted pair |

> **Direction Control:** GPIO 4 drives both RE and DE. HIGH = TX, LOW = RX. Software handles auto-direction.

### 4.6 BTS7960 #1 — Pan Motor (Azimuth Slew Drive)

| BTS7960 Pin | Connect To | Notes |
|------------|-----------|-------|
| VCC | 5V (ESP32 side) | Logic power |
| GND | GND | Common ground with ESP32 |
| R_PWM | GPIO 26 | Reverse/right direction |
| L_PWM | GPIO 25 | Forward/left direction |
| R_EN | 3.3V | Always enabled (tie HIGH) |
| L_EN | 3.3V | Always enabled (tie HIGH) |
| B+ | 12V/24V Battery (+) | Motor power — separate from ESP32 supply |
| B- | 12V/24V Battery (-) | Motor power ground |
| M+ | Slew Drive Motor (+) | |
| M- | Slew Drive Motor (-) | |

### 4.7 BTS7960 #2 — Tilt Motor (Linear Actuator)

| BTS7960 Pin | Connect To | Notes |
|------------|-----------|-------|
| VCC | 5V (ESP32 side) | Logic power |
| GND | GND | Common ground with ESP32 |
| R_PWM | GPIO 14 | Reverse/down direction |
| L_PWM | GPIO 27 | Forward/up direction |
| R_EN | 3.3V | Always enabled (tie HIGH) |
| L_EN | 3.3V | Always enabled (tie HIGH) |
| B+ | 12V/24V Battery (+) | Motor power — separate from ESP32 supply |
| B- | 12V/24V Battery (-) | Motor power ground |
| M+ | Linear Actuator (+) | |
| M- | Linear Actuator (-) | |

---

## 5. MOTOR CONTROL LOGIC

| GPIO | Action | PWM Behavior |
|------|--------|-------------|
| 25 (Pan L_PWM) | Slew clockwise / right | PWM duty = speed |
| 26 (Pan R_PWM) | Slew counter-clockwise / left | PWM duty = speed |
| 27 (Tilt L_PWM) | Tilt up | PWM duty = speed |
| 14 (Tilt R_PWM) | Tilt down | PWM duty = speed |

**Soft Start:** `ledcWrite()` ramps PWM from 0 to target duty over ~500ms to protect mechanical gearing.
**Deadband:** Motor only activates when error > 2° (prevents micro-jittering).

---

## 6. POWER WIRING

```
[12V/24V Battery]
    │
    ├──→ BTS7960 #1 (B+/B-) — Motor power
    ├──→ BTS7960 #2 (B+/B-) — Motor power
    └──→ Buck Converter (LM2596)
              │
              └──→ 5V → ESP32 VIN (USB or 5V pin)
                        │
                        ├──→ 3.3V Regulator (on-board) → I2C devices (DS3231, MPU6050, QMC5883L)
                        └──→ 5V → MAX485, LCD 2004
```

| Source | Voltage | Feeds | Notes |
|--------|---------|-------|-------|
| Battery | 12V/24V | BTS7960 motor power (B+/B-) | High current — use thick gauge wire |
| Buck Converter | 5V | ESP32, MAX485, LCD backlight | Clean regulated supply |
| ESP32 3.3V Regulator | 3.3V | DS3231, MPU6050, QMC5883L | On-board regulator |

---

## 7. RS485 WIND SENSOR WIRING

| Parameter | Value |
|-----------|-------|
| **Baud Rate** | 9600 (verify with sensor datasheet) |
| **Protocol** | Modbus RTU |
| **Termination** | 120Ω at sensor end (if long cable) |
| **Cable** | Shielded twisted pair (A+B) with drain to GND at one end |

---

## 8. SAFETY NOTES

- [ ] MPU6050 AD0 → 3.3V confirmed (address 0x69, no DS3231 conflict)
- [ ] I2C pull-up resistors installed (4.7kΩ to 3.3V on SDA/SCL)
- [ ] Motor power (12V/24V) completely separate from ESP32 logic power
- [ ] BTS7960 ground tied to ESP32 ground (common reference)
- [ ] RE/DE pin (GPIO 4) configured — floating would cause RS485 bus contention
- [ ] Buck converter output verified at 5V ±0.2V before connecting ESP32
- [ ] All I2C devices respond to `i2c_scanner` sketch before deploying field code

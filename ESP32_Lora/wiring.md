# wiring.md — ESP32_Lora Irrigation Control (8 Boards)

**Version:** 1.2.0  |  **Date:** 2026-06-12  |  **Linked SPEC:** ESP32_Lora.md  |  **BOM:** BOM.md

> **Purpose:** Single reference for DAD at the bench. All GPIO pins, component connections, and hardware wiring for Master (AA) and Slaves (B1-B7).

---

## 1. MCU BOARD

| Parameter | Value |
|-----------|-------|
| **Board** | ESP32 DOIT DevKit V1 |
| **Power Input** | USB 5V (micro USB) or 5V VIN |
| **GPIO Voltage** | 3.3V — do NOT feed 5V to any GPIO |
| **⚠️ GPIO 12** | **SKIPPED** — boot strapping pin, unreliable for I/O |

---

## 2. GPIO PINOUT — ALL BOARDS

| GPIO | Function | Direction | Board | Notes |
|------|----------|-----------|-------|-------|
| 2 | LoRa DIO0 | INPUT | All | SX1278 interrupt |
| 4 | LoRa RST | OUTPUT | All | SX1278 reset |
| 5 | LoRa SS | OUTPUT | All | SPI chip select (NSS) |
| 12 | UNUSED | — | — | **Skipped** — boot strapping pin |
| 13 | Relay 1 | OUTPUT | B4-B7 | Optocoupler IN1, active LOW |
| 14 | Relay 2 | OUTPUT | B4-B7 | Optocoupler IN2, active LOW |
| 16 | Fan Gate | OUTPUT | B2, B3 | AO3400 MOSFET, solar-gated |
| 17 | DS18B20 | I/O | B2, B3 | OneWire DQ, 4.7kΩ pull-up |
| 18 | LoRa SCK | OUTPUT | All | SPI clock |
| 19 | LoRa MISO | INPUT | All | SPI data RX |
| 21 | I2C SDA | I/O | All | Shared — RTC + LCD + INA226 |
| 22 | I2C SCL | OUTPUT | All | Shared — RTC + LCD + INA226 |
| 23 | LoRa MOSI | OUTPUT | All | SPI data TX |
| 25 | Relay 5 | OUTPUT | B5, B7 | Optocoupler IN5, active LOW |
| 26 | Relay 4 | OUTPUT | B4-B7 | Optocoupler IN4, active LOW |
| 27 | Relay 3 | OUTPUT | B4-B7 | Optocoupler IN3, active LOW |
| 32 | Relay 7 | OUTPUT | B7 | Optocoupler IN7, active LOW |
| 33 | Relay 6 | OUTPUT | B5, B7 | Optocoupler IN6, active LOW |
| 34 | Dip Bit2 | INPUT | All | DIP switch #3, INPUT_PULLUP |
| 35 | Dip Bit3 | INPUT | All | DIP switch #4 (MSB), INPUT_PULLUP |
| 36 | Dip Bit0 | INPUT | All | DIP switch #1 (LSB), INPUT_PULLUP |
| 39 | Dip Bit1 | INPUT | All | DIP switch #2, INPUT_PULLUP |

---

## 3. COMPONENT CONNECTIONS

### 3.1 SX1278 LoRa Module (ALL Boards)

| SX1278 Pin | ESP32 GPIO | Wire Color | Notes |
|-----------|-----------|------------|-------|
| VCC | 3.3V | Red | |
| GND | GND | Black | |
| NSS | GPIO 5 | Yellow | SPI chip select |
| SCK | GPIO 18 | Orange | SPI clock |
| MOSI | GPIO 23 | Green | SPI data TX |
| MISO | GPIO 19 | Blue | SPI data RX |
| RST | GPIO 4 | Purple | Reset |
| DIO0 | GPIO 2 | White | Interrupt |

### 3.2 DS3231 RTC Module (ALL Boards)

| DS3231 Pin | ESP32 GPIO | Wire Color | Notes |
|-----------|-----------|------------|-------|
| VCC | 3.3V | Red | |
| GND | GND | Black | |
| SDA | GPIO 21 | Blue | I2C data |
| SCL | GPIO 22 | Yellow | I2C clock |
| I2C Address | — | — | 0x68 |

### 3.3 LCD2004 with I2C Backpack (Master AA Only)

| LCD Pin | ESP32 GPIO | Wire Color | Notes |
|---------|-----------|------------|-------|
| VCC | 5V (VIN) | Red | |
| GND | GND | Black | |
| SDA | GPIO 21 | Blue | I2C data |
| SCL | GPIO 22 | Yellow | I2C clock |
| I2C Address | — | — | 0x27 |

### 3.4 INA226 Voltage/Current Sensor (B2, B3 Only)

| INA226 Pin | Connection | Wire Color | Notes |
|-----------|-----------|------------|-------|
| VCC | 3.3V | Red | |
| GND | GND | Black | |
| SDA | GPIO 21 | Blue | I2C |
| SCL | GPIO 22 | Yellow | I2C |
| VBUS | Solar Panel (+) | Red | 0-26V range |
| VIN+ | Shunt 0.1Ω (panel side) | — | Current sense |
| VIN− | Shunt 0.1Ω (load side) | — | Current sense |
| I2C Address | — | — | 0x40 |

> **Solar threshold:** Vbus ≥ 12.0V = sun (solarOK). ≤ 11.0V = night. 1V hysteresis, 3-reading debounce. **Fail-safe:** INA226 failure → solarOK = true (never block pump on sensor fault).

### 3.5 DS18B20 Temperature Sensor (B2, B3 Only)

| DS18B20 Pin | Connection | Notes |
|------------|-----------|-------|
| VCC (Red) | 3.3V | |
| GND (Black) | GND | |
| DQ (Yellow) | GPIO 17 | OneWire data line |
| Pull-up | 4.7kΩ DQ → 3.3V | **MANDATORY** |

### 3.6 Fan MOSFET Circuit (B2, B3 Only)

| Component | Connection | Notes |
|-----------|-----------|-------|
| AO3400 Gate | GPIO 16 (via 100Ω) | 3.3V logic drive |
| AO3400 Source | GND | |
| AO3400 Drain | Fan (−) | |
| Fan (+) | Solar 12V (fused) | |
| Gate pull-down | 10kΩ GPIO16 → GND | Prevents floating during boot |

> **Fan logic:** ON at ≥ 40°C, OFF at ≤ 38°C (2°C hysteresis). Solar-gated (fan only runs when solarOK = true).

### 3.7 DIP Switch (ALL Boards)

| DIP Pin | GPIO | Bit | Notes |
|---------|------|-----|-------|
| Switch 1 | GPIO 36 | Bit 0 (LSB) | INPUT_PULLUP |
| Switch 2 | GPIO 39 | Bit 1 | INPUT_PULLUP |
| Switch 3 | GPIO 34 | Bit 2 | INPUT_PULLUP |
| Switch 4 | GPIO 35 | Bit 3 (MSB) | INPUT_PULLUP |

### 3.8 Relay Modules (B1-B7 Slaves)

| Channel | GPIO | Global ID | Board | Active |
|---------|------|-----------|-------|--------|
| R1 | 13 | 1 | B4 | LOW |
| R2 | 14 | 2 | B4 | LOW |
| R3 | 27 | 3 | B4 | LOW |
| R4 | 26 | 4 | B4 | LOW |
| R5 | 25 | 5 | B5 | LOW |
| R6 | 33 | 6 | B5 | LOW |
| R7 | 32 | 7 | B5/B7 | LOW |
| ... | ... | 8-20 | B5-B7 | LOW |

> **All relays are active-LOW optocoupler modules.** `RELAY_ON = LOW`, `RELAY_OFF = HIGH`. JD-VCC jumper ON for optocoupler mode.

---

## 4. DIPSWITCH BOARD ID

| ID | Board | Switch 4 (35) | Switch 3 (34) | Switch 2 (39) | Switch 1 (36) |
|----|-------|:---:|:---:|:---:|:---:|
| 0 | AA Master | OFF | OFF | OFF | OFF |
| 1 | B1 Pump | OFF | OFF | OFF | ON |
| 2 | B2 Pump+Solar | OFF | OFF | ON | OFF |
| 3 | B3 Group+Solar | OFF | OFF | ON | ON |
| 4 | B4 Valves (4) | OFF | ON | OFF | OFF |
| 5 | B5 Valves (6) | OFF | ON | OFF | ON |
| 6 | B6 Valves (4) | OFF | ON | ON | OFF |
| 7 | B7 Valves (6) | OFF | ON | ON | ON |

---

## 5. I2C BUS SUMMARY

| Device | Address | Boards | Notes |
|--------|---------|--------|-------|
| DS3231 RTC | 0x68 | All | Time source |
| LCD 2004 | 0x27 | Master AA only | 20x4 display |
| INA226 | 0x40 | B2, B3 | Solar voltage/current sensor |

> SDA = GPIO 21, SCL = GPIO 22. Pull-ups: 4.7kΩ to 3.3V on both lines.

---

## 6. BOARD-SPECIFIC WIRING

| Board | ID | Relays | Relays GPIO | Sensors | Fan | LCD | Solar |
|-------|:--:|:------:|-------------|---------|:---:|:---:|:-----:|
| AA Master | 0 | — | — | — | — | ✅ | — |
| B1 Pump | 1 | 1 | GPIO13 | — | — | — | — |
| B2 Pump+Solar | 2 | 1 | GPIO13 | INA226 + DS18B20 | ✅ | — | ✅ |
| B3 Group+Solar | 3 | 1 | GPIO13 | INA226 + DS18B20 | ✅ | — | ✅ |
| B4 Valves | 4 | 4 | 13,14,27,26 | — | — | — | — |
| B5 Valves | 5 | 6 | 13,14,27,26,25,33 | — | — | — | — |
| B6 Valves | 6 | 4 | 13,14,27,26 | — | — | — | — |
| B7 Valves | 7 | 6 | 13,14,27,26,25,33,32 | — | — | — | — |

> **All boards:** LoRa ✅ | RTC ✅ | DIP Switch ✅ | 433MHz Antenna ✅

---

## 7. TOTAL HARDWARE COUNTS (ALL 8 BOARDS)

| Component | Total Qty | Boards |
|-----------|:---:|--------|
| ESP32 DOIT DevKit V1 | **8** | AA, B1-B7 |
| SX1278 LoRa 433MHz | **8** | All |
| DS3231 RTC Module | **8** | All |
| 433MHz Spring Antenna | **8** | All |
| 4-DIP Switch | **8** | All |
| 1-Ch Relay Module (optocoupler) | **23** | B1(1), B2(1), B3(1), B4(4), B5(6), B6(4), B7(6) |
| 4.7kΩ Resistor ¼W | **18** | All (2×I2C) + B2/B3 (+1 DS18B20) |
| LCD2004 w/ I2C Backpack | **1** | AA only |
| INA226 Sensor | **2** | B2, B3 |
| DS18B20 Sensor | **2** | B2, B3 |
| AO3400 N-Ch MOSFET | **2** | B2, B3 |
| 1N5819 Schottky Diode | **2** | B2, B3 |
| 12V DC Fan 40mm | **2** | B2, B3 |
| Solar Panel 12V 20W+ | **2** | B2, B3 |
| LM2596 Buck Converter | **2** | B2, B3 |
| 0.1Ω Shunt Resistor 2W | **2** | B2, B3 |
| SS34 Schottky 3A/40V | **2** | B2, B3 |
| 2A PTC Fuse (RXEF020) | **2** | B2, B3 |
| P6KE18A TVS Diode | **2** | B2, B3 |
| SMAJ5.0A TVS Diode | **2** | B2, B3 |
| 2200µF 50V Electrolytic Cap | **2** | B2, B3 |
| 470µF 16V Electrolytic Cap | **2** | B2, B3 |
| 100nF MLCC Capacitor | **2** | B2, B3 |
| 100Ω Resistor ¼W | **2** | B2, B3 |
| 10kΩ Resistor ¼W | **2** | B2, B3 |
| Screw Terminal Block 2P | **4** | B2, B3 (2 each) |

> 📋 **Full per-board breakdown and purchase notes:** See [BOM.md](BOM.md)

## 7. POWER WIRING

| Source | Voltage | Feeds | Boards |
|--------|---------|-------|--------|
| USB (micro USB) | 5V | ESP32, peripherals | All (bench) |
| Solar panel → buck | 5V regulated | ESP32, SX1278, DS3231 | All (field) |
| Solar panel direct | 12V | Fan via AO3400 | B2, B3 |

---

## 8. SAFETY NOTES

- [ ] GPIO 12 **must be UNCONNECTED** — boot strapping pin
- [ ] All relays active-LOW — test with `RELAY_ON LOW` before connecting pumps
- [ ] DS18B20 4.7kΩ pull-up **required** — sensor won't read without it
- [ ] AO3400 gate pull-down (10kΩ) **required** — prevents floating fan during boot
- [ ] I2C pull-ups (4.7kΩ SDA/SCL → 3.3V) required for bus stability
- [ ] DIP switch uses INPUT_PULLUP — connect switch between GPIO and GND only

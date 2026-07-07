# BOM.md — ESP32_Lora Irrigation Control (8 Boards)

**Version:** 1.2.0  |  **Date:** 2026-06-12  |  **Linked SPEC:** ESP32_Lora.md

> **Scope:** 1× Master (AA) + 7× Slave (B1-B7). All physical hardware — no simulator.

---

## 1. PER-BOARD BOM

### 1.1 AA — Master (Board ID 0)

| # | Component | Qty | Notes |
|---|-----------|:---:|-------|
| 1 | ESP32 DOIT DevKit V1 | 1 | Main MCU |
| 2 | SX1278 LoRa Module 433MHz | 1 | SPI, RadioLib |
| 3 | DS3231 RTC Module | 1 | I2C 0x68 |
| 4 | LCD2004 20×4 w/ I2C Backpack | 1 | I2C 0x27 |
| 5 | 4-DIP Switch | 1 | Board ID setting |
| 6 | Spring Antenna 433MHz | 1 | SMA/SMD solder |
| 7 | 4.7kΩ Resistor | 2 | I2C pull-up (SDA, SCL) |
| 8 | Dupont Wires (M-F, 20cm) | ~15 | Bench wiring |
| 9 | USB Micro Cable | 1 | Power + flash |

### 1.2 B1 — Pump (Board ID 1)

| # | Component | Qty | Notes |
|---|-----------|:---:|-------|
| 1 | ESP32 DOIT DevKit V1 | 1 | |
| 2 | SX1278 LoRa Module 433MHz | 1 | |
| 3 | DS3231 RTC Module | 1 | I2C 0x68 |
| 4 | 1-Ch Relay Module (optocoupler) | 1 | Active-LOW, JD-VCC |
| 5 | 4-DIP Switch | 1 | |
| 6 | Spring Antenna 433MHz | 1 | |
| 7 | 4.7kΩ Resistor | 2 | I2C pull-up |
| 8 | Dupont Wires (M-F, 20cm) | ~10 | |

### 1.3 B2 — Pump + Solar (Board ID 2)

| # | Component | Qty | Notes |
|---|-----------|:---:|-------|
| 1 | ESP32 DOIT DevKit V1 | 1 | |
| 2 | SX1278 LoRa Module 433MHz | 1 | |
| 3 | DS3231 RTC Module | 1 | I2C 0x68 |
| 4 | INA226 Voltage/Current Sensor | 1 | I2C 0x40, Vbus 0-26V |
| 5 | DS18B20 Temperature Sensor | 1 | OneWire GPIO17 |
| 6 | 1-Ch Relay Module (optocoupler) | 1 | Active-LOW pump relay |
| 7 | 4-DIP Switch | 1 | |
| 8 | Spring Antenna 433MHz | 1 | |
| 9 | AO3400 N-Ch MOSFET | 1 | Fan gate driver |
| 10 | 1N5819 Schottky Diode | 1 | Fan flyback protection |
| 11 | 12V DC Fan (40×40mm) | 1 | Cooling, solar-gated |
| 12 | Solar Panel 12V 20W+ | 1 | Power source |
| 13 | LM2596 Buck Converter | 1 | Solar → 5V regulated |
| 14 | 0.1Ω Shunt Resistor (2W) | 1 | INA226 current sense |
| 15 | SS34 Schottky Diode (3A/40V) | 1 | Solar reverse-polarity |
| 16 | 2A PTC Resettable Fuse (RXEF020) | 1 | Solar(+) overcurrent |
| 17 | P6KE18A TVS Diode | 1 | Solar input surge clamp |
| 18 | SMAJ5.0A TVS Diode | 1 | 5V rail protection |
| 19 | 2200µF 50V Electrolytic Cap | 1 | Solar input filter (C1) |
| 20 | 470µF 16V Electrolytic Cap | 1 | 5V output filter (C2) |
| 21 | 100nF MLCC Capacitor | 1 | HF decoupling across C2 |
| 22 | 100Ω Resistor | 1 | Gate series |
| 23 | 10kΩ Resistor | 1 | Gate pull-down |
| 24 | 4.7kΩ Resistor | 3 | I2C (2) + DS18B20 pull-up |
| 25 | Dupont Wires (M-F, 20cm) | ~15 | |
| 26 | Screw Terminal Block 2P | 2 | Solar in + Fan out |

### 1.4 B3 — Group Pump + Solar (Board ID 3)

| # | Component | Qty | Notes |
|---|-----------|:---:|-------|
| | *Identical to B2 (all 26 items)* | | *Plus: Valve relay window logic* |

### 1.5 B4 — Valves 4-ch (Board ID 4)

| # | Component | Qty | Notes |
|---|-----------|:---:|-------|
| 1 | ESP32 DOIT DevKit V1 | 1 | |
| 2 | SX1278 LoRa Module 433MHz | 1 | |
| 3 | DS3231 RTC Module | 1 | I2C 0x68 |
| 4 | 1-Ch Relay Module (optocoupler) | 4 | Active-LOW, R1-R4 |
| 5 | 4-DIP Switch | 1 | |
| 6 | Spring Antenna 433MHz | 1 | |
| 7 | 4.7kΩ Resistor | 2 | I2C pull-up |
| 8 | Dupont Wires (M-F, 20cm) | ~12 | |

### 1.6 B5 — Valves 6-ch (Board ID 5)

| # | Component | Qty | Notes |
|---|-----------|:---:|-------|
| 1 | ESP32 DOIT DevKit V1 | 1 | |
| 2 | SX1278 LoRa Module 433MHz | 1 | |
| 3 | DS3231 RTC Module | 1 | I2C 0x68 |
| 4 | 1-Ch Relay Module (optocoupler) | 6 | Active-LOW, R5-R10 |
| 5 | 4-DIP Switch | 1 | |
| 6 | Spring Antenna 433MHz | 1 | |
| 7 | 4.7kΩ Resistor | 2 | I2C pull-up |
| 8 | Dupont Wires (M-F, 20cm) | ~14 | |

### 1.7 B6 — Valves 4-ch (Board ID 6)

| # | Component | Qty | Notes |
|---|-----------|:---:|-------|
| 1 | ESP32 DOIT DevKit V1 | 1 | |
| 2 | SX1278 LoRa Module 433MHz | 1 | |
| 3 | DS3231 RTC Module | 1 | I2C 0x68 |
| 4 | 1-Ch Relay Module (optocoupler) | 4 | Active-LOW, R11-R14 |
| 5 | 4-DIP Switch | 1 | |
| 6 | Spring Antenna 433MHz | 1 | |
| 7 | 4.7kΩ Resistor | 2 | I2C pull-up |
| 8 | Dupont Wires (M-F, 20cm) | ~12 | |

### 1.8 B7 — Valves 6-ch (Board ID 7)

| # | Component | Qty | Notes |
|---|-----------|:---:|-------|
| 1 | ESP32 DOIT DevKit V1 | 1 | |
| 2 | SX1278 LoRa Module 433MHz | 1 | |
| 3 | DS3231 RTC Module | 1 | I2C 0x68 |
| 4 | 1-Ch Relay Module (optocoupler) | 6 | Active-LOW, R15-R20 |
| 5 | 4-DIP Switch | 1 | |
| 6 | Spring Antenna 433MHz | 1 | |
| 7 | 4.7kΩ Resistor | 2 | I2C pull-up |
| 8 | Dupont Wires (M-F, 20cm) | ~14 | |

---

## 2. MASTER TOTAL — ALL 8 BOARDS

| # | Component | Qty | Boards |
|---|-----------|:---:|--------|
| 1 | ESP32 DOIT DevKit V1 | **8** | AA, B1-B7 |
| 2 | SX1278 LoRa Module 433MHz | **8** | All |
| 3 | DS3231 RTC Module | **8** | All |
| 4 | Spring Antenna 433MHz | **8** | All |
| 5 | 4-DIP Switch | **8** | All |
| 6 | 1-Ch Relay Module (optocoupler) | **23** | B1(1), B2(1), B3(1), B4(4), B5(6), B6(4), B7(6) |
| 7 | 4.7kΩ Resistor (¼W) | **18** | All (2 per board) + B2/B3 (+1 DS18B20 each) |
| 8 | LCD2004 20×4 w/ I2C Backpack | **1** | AA only |
| 9 | INA226 Voltage/Current Sensor | **2** | B2, B3 |
| 10 | DS18B20 Temperature Sensor | **2** | B2, B3 |
| 11 | AO3400 N-Ch MOSFET | **2** | B2, B3 |
| 12 | 1N5819 Schottky Diode | **2** | B2, B3 |
| 13 | 12V DC Fan 40×40mm | **2** | B2, B3 |
| 14 | Solar Panel 12V 20W+ | **2** | B2, B3 |
| 15 | LM2596 Buck Converter | **2** | B2, B3 |
| 16 | 0.1Ω Shunt Resistor 2W | **2** | B2, B3 |
| 17 | SS34 Schottky Diode (3A/40V) | **2** | B2, B3 |
| 18 | 2A PTC Resettable Fuse (RXEF020) | **2** | B2, B3 |
| 19 | P6KE18A TVS Diode | **2** | B2, B3 |
| 20 | SMAJ5.0A TVS Diode | **2** | B2, B3 |
| 21 | 2200µF 50V Electrolytic Capacitor | **2** | B2, B3 |
| 22 | 470µF 16V Electrolytic Capacitor | **2** | B2, B3 |
| 23 | 100nF MLCC Capacitor | **2** | B2, B3 |
| 24 | 100Ω Resistor (¼W) | **2** | B2, B3 |
| 25 | 10kΩ Resistor (¼W) | **2** | B2, B3 |
| 26 | Screw Terminal Block 2P | **4** | B2, B3 (2 each) |
| 27 | Dupont Wires M-F 20cm (assorted) | ~100 | All boards |
| 28 | USB Micro Cable | **1** | Master AA only (field: 5V buck) |

---

## 3. CATEGORY BREAKDOWN

### Core (Every Board)
| Component | Qty |
|-----------|:---:|
| ESP32 DOIT DevKit V1 | 8 |
| SX1278 LoRa 433MHz | 8 |
| DS3231 RTC | 8 |
| 433MHz Spring Antenna | 8 |
| 4-DIP Switch | 8 |

### Relay (Total: 23 channels)
| Board | Channels |
|-------|:---:|
| B1 | 1 |
| B2 | 1 |
| B3 | 1 |
| B4 | 4 |
| B5 | 6 |
| B6 | 4 |
| B7 | 6 |

### Solar Power System (B2 + B3 Only)
| Component | Qty |
|-----------|:---:|
| Solar Panel 12V 20W+ | 2 |
| LM2596 Buck Converter | 2 |
| INA226 Sensor | 2 |
| DS18B20 Temp Sensor | 2 |
| 12V Fan 40mm | 2 |

---

## 4. PURCHASE NOTES

- **AliExpress / Lazada:** ESP32 DOIT, SX1278, DS3231, relay modules — buy 10-packs for spares
- **Relay modules:** Spec "optocoupler isolated, active LOW trigger, JD-VCC jumper ON"
- **SX1278:** Must be **433 MHz** variant (not 868/915 MHz)
- **Spring antennas:** 433 MHz helical/spring, SMA male connector
- **DIP Switch:** 4-position, 2.54mm pitch, piano or slide style
- **Resistors:** ¼W carbon film, 4.7kΩ / 100Ω / 10kΩ — buy 50-packs
- **LM2596:** Module with heatsink and adjustable output (set to 5.0V before connecting)
- **Solar panels:** 12V nominal, 20-50W, open-circuit Voc 18-22V within INA226 26V limit

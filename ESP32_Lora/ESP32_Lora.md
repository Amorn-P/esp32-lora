# ESP32_Lora — Project Documentation

**Date:** 2026-05-21 | **Version:** 1.1.0 | **Status:** Active Development
**Last Review:** 2026-06-07 (Q/A + Design Approval + Wokwi Simulation)

---

## 1. SYSTEM OVERVIEW

1 Master (AA) + 7 Slaves (B1-B7) irrigation control system.
LoRa star topology. Non-centralized — each slave runs autonomously from local RTC.

```
AA (Master ID:0)                    B1-B7 (Slaves ID:1-7)
┌─────────────────────┐    LoRa     ┌─────────────────────┐
│ CommCore: WiFi      │◄──────────►│ LoRa Only (no WiFi) │
│  Blynk Legacy       │  heartbeat  │ DS3231 RTC          │
│  IotWebConf         │  schedules  │ Dipswitch ID        │
│  Telegram           │  commands   │ 7 Relays            │
│  Web Config         │             │ NVM (Preferences)   │
│ LCD2004 Display     │             │ Autonomous schedule │
│ DS3231 RTC          │             └─────────────────────┘
└─────────────────────┘
```

---

## 2. BOARD ROLES & DAILY SCHEDULE

### Daily Timeline

```
B1_Window_Start ═══════════════════════════════════════════════════ B1_Window_End (17:00 default)
│                                                                     │
│ B1 PUMP:     [50m ON][10m OFF]...repeat (Blynk-configurable)        │
│                                                                     │
│ B3-B7 R1:    [B4-R1──1m overlap──B4-R2──...──B7-R20]               │
│ B3 PUMP:     ═══════════════════════════ (ON with valves, solar)   │
│                                                                     │
│ B2 PUMP:                   [Blynk_Start]──[50/10]...[50/10]──┤     │
│ B2 MUST END:                              R2_Start − 15min ◄──┘     │
│                                                                     │
│ B3-B7 R2:                                     [Blynk P2 Start]─────→│
│ B3 PUMP:                                      ═════════════════════→│
```

### Board Functions

| Board | Type | Function | Relays |
|-------|------|----------|--------|
| AA (0) | Master | Blynk gateway, LoRa broker, LCD, NTP→RTC sync | — |
| B1 (1) | Slave | Independent pump (Blynk-configurable) | 1 relay |
| B2 (2) | Slave | Independent pump + solar sensor + fan | 1 relay + INA226 + DS18B20 |
| B3 (3) | Slave | Group pump + solar sensor + fan | 1 relay + INA226 + DS18B20 |
| B4 (4) | Slave | Sequential valves | 4 relays (global 1-4) |
| B5 (5) | Slave | Sequential valves | 6 relays (global 5-10) |
| B6 (6) | Slave | Sequential valves | 4 relays (global 11-14) |
| B7 (7) | Slave | Sequential valves | 6 relays (global 15-20) |

### Timing Rules

| Parameter | Value |
|-----------|-------|
| B1 ON/OFF | V141 min ON / V143 min OFF (default 50/10) |
| B1 window | V140 start to 17:00 (Blynk-configurable) |
| B2 ON/OFF | V145 min ON / V147 min OFF (default 50/10) |
| B2 start | V144 (Blynk TimeInput, user-set) |
| B2 stop | Auto: R2_Start − 15 min (Master computes) |
| B2-B3 power rule | NEVER overlap (enforced by schedule window) |
| B3-B7 rounds | 2 per day (R1=V130, R2=V132 default) |
| Valve overlap | 1 minute between sequential relays |
| Solar sensor | B2, B3 only — solar panel + INA226 (Vbus-based) |
| Solar threshold | Vbus ≥12.0V = sun, ≤11.0V = night (1V hysteresis, 3-reading debounce) |
| Solar fail-safe | INA226 failure → solarOK=true (never block pump on sensor fault) |
| Fan (B2/B3) | GPIO16 AO3400, ON at 40°C, OFF at 38°C, solar-gated |

---

## 3. HARDWARE PIN MAPPING

### GPIO Reference Map

```
                     ESP32 DOIT DevKit V1
                  ┌─────────────────────────┐
    LoRa SCK  ← 18│                         │23 → LoRa MOSI
    LoRa MISO ← 19│                         │22 → I2C SCL (RTC+LCD+INA226)
                  │                         │21 → I2C SDA (RTC+LCD+INA226)
     Fan Gate  ← 16│         ESP32           │17 → DS18B20 (B2/B3)
    Relay 1   ← 13│                         │ 5 → LoRa SS
    Relay 2   ← 14│                         │ 4 → LoRa RST
    Relay 3   ← 27│                         │ 2 → LoRa DIO0
    Relay 4   ← 26│                         │36 → Dipswitch Bit0 (LSB)
    Relay 5   ← 25│                         │39 → Dipswitch Bit1
    Relay 6   ← 33│                         │34 → Dipswitch Bit2
    Relay 7   ← 32│                         │35 → Dipswitch Bit3
                  │                GPIO12   │   → SKIPPED (boot strap pin)
                  │          EN, 3V3, GND   │
                  └─────────────────────────┘
```

### Full GPIO Assignment

| GPIO | Function | Connection | Board | Notes |
|------|----------|-----------|-------|-------|
| 2 | LoRa DIO0 | SX1278 DIO0 | All | Interrupt |
| 4 | LoRa RST | SX1278 RST | All | Reset |
| 5 | LoRa SS | SX1278 NSS | All | SPI Chip Select |
| 12 | — | — | — | **SKIPPED** — boot strapping pin, unreliable |
| 13 | Relay 1 | Optocoupler IN1 | Slave | Active LOW |
| 14 | Relay 2 | Optocoupler IN2 | Slave | Active LOW |
| 16 | Fan Gate | AO3400 MOSFET Gate | B2, B3 | Solar-gated, 3.3V logic |
| 17 | DS18B20 | OneWire DQ | B2, B3 | 4.7kΩ pull-up to 3.3V |
| 18 | LoRa SCK | SX1278 SCK | All | SPI Clock |
| 19 | LoRa MISO | SX1278 MISO | All | SPI Data RX |
| 21 | I2C SDA | DS3231 + LCD + INA226 | All | Shared I2C bus |
| 22 | I2C SCL | DS3231 + LCD + INA226 | All | Shared I2C bus |
| 23 | LoRa MOSI | SX1278 MOSI | All | SPI Data TX |
| 25 | Relay 5 | Optocoupler IN5 | Slave | Active LOW |
| 26 | Relay 4 | Optocoupler IN4 | Slave | Active LOW |
| 27 | Relay 3 | Optocoupler IN3 | Slave | Active LOW |
| 32 | Relay 7 | Optocoupler IN7 | Slave | Active LOW |
| 33 | Relay 6 | Optocoupler IN6 | Slave | Active LOW |
| 34 | Dip Bit2 | DIP Switch #3 | All | INPUT_PULLUP, 1 = bit value |
| 35 | Dip Bit3 | DIP Switch #4 | All | INPUT_PULLUP, MSB |
| 36 | Dip Bit0 | DIP Switch #1 | All | INPUT_PULLUP, LSB |
| 39 | Dip Bit1 | DIP Switch #2 | All | INPUT_PULLUP, 1 = bit value |

### Dipswitch ID Encoding

| ID | Board | Bit3 | Bit2 | Bit1 | Bit0 |
|----|-------|------|------|------|------|
| 0 | AA Master | OFF | OFF | OFF | OFF |
| 1 | B1 Pump | OFF | OFF | OFF | ON |
| 2 | B2 Pump+Solar | OFF | OFF | ON | OFF |
| 3 | B3 Group+Solar | OFF | OFF | ON | ON |
| 4 | B4 Valves 4 | OFF | ON | OFF | OFF |
| 5 | B5 Valves 6 | OFF | ON | OFF | ON |
| 6 | B6 Valves 4 | OFF | ON | ON | OFF |
| 7 | B7 Valves 6 | OFF | ON | ON | ON |

### SX1278 LoRa Wiring

| SX1278 Pin | ESP32 GPIO | Wire Color (suggested) |
|-----------|-----------|------------------------|
| VCC | 3.3V | Red |
| GND | GND | Black |
| NSS | GPIO 5 | Yellow |
| SCK | GPIO 18 | Orange |
| MOSI | GPIO 23 | Green |
| MISO | GPIO 19 | Blue |
| RST | GPIO 4 | Purple |
| DIO0 | GPIO 2 | White |

### DS3231 RTC Wiring

| DS3231 Pin | ESP32 GPIO | Wire Color |
|-----------|-----------|-----------|
| VCC | 3.3V | Red |
| GND | GND | Black |
| SDA | GPIO 21 | Blue |
| SCL | GPIO 22 | Yellow |

### LCD2004 I2C Wiring (Master only)

| LCD2004 Pin | ESP32 GPIO | Wire Color |
|------------|-----------|-----------|
| VCC | 5V (VIN) | Red |
| GND | GND | Black |
| SDA | GPIO 21 | Blue |
| SCL | GPIO 22 | Yellow |
| I2C Address | 0x27 | — |

### INA226 Wiring (B2, B3 only)

| INA226 Pin | Connection | Wire Color |
|-----------|-----------|-----------|
| VCC | 3.3V | Red |
| GND | GND | Black |
| SDA | GPIO 21 | Blue |
| SCL | GPIO 22 | Yellow |
| VBUS | Solar Panel (+) | Red |
| VIN+/VIN− | Shunt 0.1Ω | — |
| I2C Address | 0x40 | — |

### DS18B20 Wiring (B2, B3 only)

| DS18B20 Pin | Connection | Wire Color |
|------------|-----------|-----------|
| VCC (Red) | 3.3V | Red |
| GND (Black) | GND | Black |
| DQ (Yellow) | GPIO 17 | Yellow |
| Pull-up | 4.7kΩ between DQ and 3.3V | — |

### Fan MOSFET Circuit (B2, B3 only)

| Component | Connection |
|-----------|-----------|
| AO3400 Gate | GPIO 16 (via 100Ω resistor) |
| AO3400 Source | GND |
| AO3400 Drain | Fan (−) |
| Fan (+) | Solar 12V (fused) |
| Gate pull-down | 10kΩ GPIO16 → GND |

### Relay Output (7-Channel, Active LOW)

| Channel | GPIO | Global Relay ID | Board |
|---------|------|----------------|-------|
| R1 | 13 | 1 → B4-R1 | B4 |
| R2 | 14 | 2 → B4-R2 | B4 |
| R3 | 27 | 3 → B4-R3 | B4 |
| R4 | 26 | 4 → B4-R4 | B4 |
| R1 | 13 | 5 → B5-R1 | B5 |
| R2 | 14 | 6 → B5-R2 | B5 |
| R3 | 27 | 7 → B5-R3 | B5 |
| R4 | 26 | 8 → B5-R4 | B5 |
| R5 | 25 | 9 → B5-R5 | B5 |
| R6 | 33 | 10 → B5-R6 | B5 |
| R1 | 13 | 11 → B6-R1 | B6 |
| R2 | 14 | 12 → B6-R2 | B6 |
| R3 | 27 | 13 → B6-R3 | B6 |
| R4 | 26 | 14 → B6-R4 | B6 |
| R1 | 13 | 15 → B7-R1 | B7 |
| R2 | 14 | 16 → B7-R2 | B7 |
| R3 | 27 | 17 → B7-R3 | B7 |
| R4 | 26 | 18 → B7-R4 | B7 |
| R5 | 25 | 19 → B7-R5 | B7 |
| R6 | 33 | 20 → B7-R6 | B7 |

**Relay activation:** `RELAY_ON = LOW` — GPIO goes LOW to energize relay coil.
**JD-VCC jumper:** Set to ON (single power supply for both logic + relay coil).

### Solar Panel Circuit (B2, B3)

```
Solar Panel (18V Voc) → SS34 Diode → PTC Fuse → LM2596 Buck → 5V Bus
                                                    ↓
                                              TVS 5.0V → GND
                                              2200µF Cap → GND
                                                    ↓
                                              INA226 VBUS (sense)
                                              INA226 Shunt → LOAD
                                                    ↓
                                              ESP32 3.3V Regulator
                                              Fan (via AO3400)
```

---

## 4. LoRa PROTOCOL

### LuckyPacket (19 bytes, packed)

```cpp
struct LuckyPacket {
    uint8_t senderId;      // 0=Master, 1-7=Slave
    uint8_t targetId;      // 0=Master, 1-7=Specific, 255=Broadcast
    uint8_t cmdType;       // 0=Heartbeat, 1=Manual, 2=StopAll, 3=TimeSync, 4=SetSchedule
    uint8_t relayStatus;   // 7-bit bitmap
    uint8_t currentMode;   // 0=Idle, 1=P1, 2=P2, 3=Manual
    uint8_t battery;       // Reserved
    uint8_t activeRelay;   // Global relay ID (1-20)
    uint8_t startHr1;
    uint8_t startMin1;
    uint8_t duration1;     // Minutes
    uint8_t startHr2;
    uint8_t startMin2;
    uint8_t duration2;
    uint8_t reserved[2];
    uint16_t checksum;     // Sum of bytes 0-16
} __attribute__((packed));
```

### Command Types

| CMD | Value | Direction | Meaning |
|-----|-------|-----------|---------|
| CMD_HEARTBEAT | 0 | Slave→Master | Status report every 10s |
| CMD_MANUAL | 1 | Master→Slave | Manual relay activation |
| CMD_STOP_ALL | 2 | Master→Slave | Emergency stop |
| CMD_TIME_SYNC | 3 | Master→Slave | RTC sync (NTP→Master→Slaves) |
| CMD_SET_SCHEDULE | 4 | Master→Slave | Schedule update |

### Packet Field Usage by Target

| targetId | startHr1 | startMin1 | duration1 | startHr2 | startMin2 | duration2 |
|----------|----------|-----------|-----------|----------|-----------|-----------|
| 1 (B1) | V140 Hr | V140 Min | V141 (ON) | V143 (OFF) | — | — |
| 2 (B2) | V144 Hr | V144 Min | V145 (ON) | V147 (OFF) | stopHr* | stopMin* |
| 255 (B3-B7) | R1 Hr | R1 Min | R1 Dur | R2 Hr | R2 Min | R2 Dur |

*B2 stop time packed in reserved[0..1] (computed as R2_Start − 15 min by Master)

### LoRa Radio Settings

| Parameter | Value |
|-----------|-------|
| Frequency | 433.0 MHz |
| Bandwidth | 125 kHz |
| Spreading Factor | 9 |
| Coding Rate | 7 |
| TX Power | 17 dBm |
| Sync Word | 0x12 |

---

## 5. BLYNK VIRTUAL PIN MAP

### Blynk Server
- **Type:** Blynk Legacy (Local Server)
- **Server:** `43.229.135.169:8080`
- **App:** Blynk Legacy (iOS/Android)

### Dashboard Tab (V1-V89) — Status Monitoring

| Pin | Widget | Label | Description |
|-----|--------|-------|-------------|
| V1 | Value Display | System Status | Shows current mode (Idle/P1/P2/Manual/Stopped) |
| V11 | Value Display | Time Remaining | Pump time remaining this round |
| V12 | Progress Bar | Session Progress | Overall irrigation progress (0-100%) |
| V19 | Button | **SYNC ALL** | Broadcasts schedule to all slaves via LoRa |
| V20 | Label | Network Status | WiFi + Blynk connection status |
| V21 | Terminal | Slave Status | Detailed relay debug output |
| V60 | LED | Master AA | Master online (always on) |
| V61 | LED | B1 Pump | B1 pump relay active |
| V62 | LED | B2 Pump | B2 pump relay active |
| V63 | LED | B3 Pump | B3 group pump relay active |
| V66 | LED | B4 R1 | Global relay 1 |
| V67 | LED | B4 R2 | Global relay 2 |
| V68 | LED | B4 R3 | Global relay 3 |
| V69 | LED | B4 R4 | Global relay 4 |
| V70 | LED | B5 R1 | Global relay 5 |
| V71 | LED | B5 R2 | Global relay 6 |
| V72 | LED | B5 R3 | Global relay 7 |
| V73 | LED | B5 R4 | Global relay 8 |
| V74 | LED | B5 R5 | Global relay 9 |
| V75 | LED | B5 R6 | Global relay 10 |
| V76 | LED | B6 R1 | Global relay 11 |
| V77 | LED | B6 R2 | Global relay 12 |
| V78 | LED | B6 R3 | Global relay 13 |
| V79 | LED | B6 R4 | Global relay 14 |
| V81 | LED | B7 R1 | Global relay 15 |
| V82 | LED | B7 R2 | Global relay 16 |
| V83 | LED | B7 R3 | Global relay 17 |
| V84 | LED | B7 R4 | Global relay 18 |
| V85 | LED | B7 R5 | Global relay 19 |
| V86 | LED | B7 R6 | Global relay 20 |
| V150 | LED | B1 Online | B1 LoRa online (green=OK, off=offline) |
| V151 | LED | B2 Online | B2 LoRa online |
| V152 | LED | B3 Online | B3 LoRa online |
| V153 | LED | B4 Online | B4 LoRa online |
| V154 | LED | B5 Online | B5 LoRa online |
| V155 | LED | B6 Online | B6 LoRa online |
| V156 | LED | B7 Online | B7 LoRa online |

### Configuration Tab (V90-V255) — Schedule & Control

| Pin | Widget | Label | Range | Default | Description |
|-----|--------|-------|-------|---------|-------------|
| V130 | TimeInput | P1 Start | 00:00-23:59 | 09:00 | **Round 1** start time (B3-B7 valves) |
| V131 | NumericInput | P1 Duration | 1-60 min | 5 | **Round 1** duration per valve (min) |
| V132 | TimeInput | P2 Start | 00:00-23:59 | 14:00 | **Round 2** start time (B3-B7 valves) |
| V133 | NumericInput | P2 Duration | 1-60 min | 5 | **Round 2** duration per valve (min) |
| V134 | NumericInput | Manual Relay | 1-20 | — | Target relay ID for manual mode |
| V135 | NumericInput | Manual Duration | 1-240 min | — | Manual ON time (minutes) |
| V136 | Button | **START Manual** | PUSH | — | Execute manual relay activation |
| V137 | Button | **STOP ALL** | PUSH | — | Emergency stop — all relays OFF, persists until next schedule broadcast |
| V138 | Button | **Mute Telegram** | PUSH | Unmuted | Toggle Telegram alerts mute/unmute. LED=255 when muted |
| V140 | TimeInput | B1 Start | 00:00-23:59 | 09:00 | B1 independent pump start time |
| V141 | NumericInput | B1 ON Duration | 1-120 min | 50 | B1 pump ON time per cycle |
| V142 | — | (reserved) | — | — | Future: B1 Period 2 |
| V143 | NumericInput | B1 OFF Duration | 1-60 min | 10 | B1 pump OFF time per cycle |
| V144 | TimeInput | B2 Start | 00:00-23:59 | 09:00 | B2 independent pump start time |
| V145 | NumericInput | B2 ON Duration | 1-120 min | 50 | B2 pump ON time per cycle |
| V146 | — | (reserved) | — | — | Future: B2 Period 2 |
| V147 | NumericInput | B2 OFF Duration | 1-60 min | 10 | B2 pump OFF time per cycle |

### CommCore Reserved Pins (DO NOT USE)

| Pin Range | Owner | Function |
|-----------|-------|----------|
| V90-V101 | CommCore | WiFi + Blynk + IP config |
| V118-V120 | CommCore | Telegram bot token + group + mode |
| V122-V125 | CommCore | OTA firmware update |
| V126-V127 | CommCore | RTC reset time |

### Blynk Dashboard Layout (Recommended)

**Tab 1: Status (Dashboard)**
```
┌─────────────┬──────────────────────────────┐
│  V60 LED    │  V1 Status Display           │
│  Master AA  │  V20 Network Label           │
├─────────────┼──────────────────────────────┤
│ V61  B1 Pump│ V66-V86 Relay LEDs (4x5 grid)│
│ V62  B2 Pump│   R1  R2  R3  R4  R5  R6     │
│ V63  B3 Pump│ B4 ○  ○  ○  ○               │
├─────────────┤ B5 ○  ○  ○  ○  ○  ○         │
│ V150 B1 OK  │ B6 ○  ○  ○  ○               │
│ V151 B2 OK  │ B7 ○  ○  ○  ○  ○  ○         │
│ ...  ...    │                              │
│ V156 B7 OK  │                              │
└─────────────┴──────────────────────────────┘
│ V19 SYNC   │ V137 STOP  │ V138 MUTE       │
│ V11 Timer  │ V12 Progress Bar             │
└─────────────────────────────────────────────┘
```

**Tab 2: Schedule (Config)**
```
┌───────────────┬───────────────┬───────────────┐
│   ROUND 1     │   ROUND 2     │   MANUAL      │
│ V130  09:00   │ V132  14:00   │ V134 Relay ID │
│ V131  5 min   │ V133  5 min   │ V135 Duration │
│               │               │ V136 START ▶  │
├───────────────┼───────────────┼───────────────┤
│    B1 PUMP    │    B2 PUMP    │               │
│ V140  09:00   │ V144  09:00   │               │
│ V141  50 min  │ V145  50 min  │               │
│ V143  10 min  │ V147  10 min  │               │
└───────────────┴───────────────┴───────────────┘
```

---

## 6. SOFTWARE ARCHITECTURE

### Build System

Two-environment PlatformIO build from single codebase:

```ini
[env:master]  BOARD_TYPE=0  # AA Master with CommCore
[env:slave]   BOARD_TYPE=1  # B1-B7 lightweight
```

| Env | Flash | RAM |
|-----|-------|-----|
| master | 84.6% (1,109 KB) | 14.4% (47 KB) |
| slave | 20.6% (269 KB) | 4.7% (15 KB) |

### Source Files

```
ESP32_Lora/
  platformio.ini
  src/
    main.cpp                  ← Unified entry (#if BOARD_TYPE)
    User_config.h             ← LoRa settings, Telegram cert
    Hardware_Map.h            ← Pin definitions
    Hardware_Service.h/.cpp   ← GPIO/Relay/Dipswitch
    Time_Manager.h/.cpp       ← DS3231 RTC wrapper
    Lora_Protocol.h           ← LuckyPacket struct
    Lora_Service.h/.cpp       ← SX1278 RadioLib engine
    Scheduler.h/.cpp          ← B1-B7 schedule logic
    INA226_Sensor.h/.cpp      ← Solar sensor (B2/B3 only)
    00_Blynk_App.h            ← Blynk callbacks (master)
    01_Social_Handlers.h      ← Web handlers
    12_List_Wf.h              ← IotWebConf params
    CommCore_Globals.cpp      ← Extern definitions
  lib/CommCore/               ← CommCore v1.0.1 (inline-def)
```

### Master — Key Logic

1. **NTP→RTC auto-sync:** On boot and every 60 min, Master pulls NTP time → writes DS3231. If NTP fails, continues with existing RTC. Broadcasts epoch to slaves via CMD_TIME_SYNC.

2. **broadcastSchedule():** Sends 3 targeted packets:
   - B1 (targetId=1): V140 Start, V141 ON, V143 OFF (Blynk-configurable, defaults 09:00/50/10)
   - B2 (targetId=2): V144 Start, V145 ON, V147 OFF + auto-computed stop (R2_Start − 15 min)
   - B3-B7 (targetId=255): R1 + R2 schedules (V130-V133)

3. **B2 stop time formula:**
   ```
   B2_stop = R2_Start − 15 min  (Master computes, packs in reserved[0..1])
   ```

4. **broadcastManual():** Sends to valve board + B3 pump simultaneously.

5. **Heartbeat handler:** Decodes relayStatus bitmap → Blynk V61-V86 LEDs.

6. **Telegram alerts:** Pump start/stop, solar pause/resume, slave offline/online. Mutable via V138.

7. **Offline detection:** Slave >30s without heartbeat → marked offline → Telegram alert + Blynk LED off.

### Slave — Key Logic

1. **Scheduler::run() — Per-board dispatch:**

   | Board | Logic |
   |-------|-------|
   | B1 | 50/10 cycle (Blynk-configurable ON/OFF), V140 start to 17:00 |
   | B2 | 50/10 cycle (Blynk ON/OFF), V144 start to auto-computed stop, **solar check** |
   | B3 | Valve window check, **solar check** |
   | B4-B7 | Staggered overlap, R1 + R2 separate durations |

2. **B1 schedule:** Reads from sched1 (V140/V141/V143). If NVM has no schedule, defaults to hardcoded 09:00/50/10.

3. **B2 schedule:** Reads from sched1 (V144/V145/V147). Stop time from reserved bytes. Solar gates the pump.

4. **B3 pump window:** Computes full 20-relay sequence. If ANY relay active → pump ON (if solar OK).

5. **Staggered overlap:**
   ```
   relay[i].start = periodStart + i × (duration − 1)
   relay[i].end   = relay[i].start + duration
   ```

6. **Solar sensor (B2/B3):** INA226 Vbus read every 5s. ≥12.0V = sun, ≤11.0V = night (1V hysteresis). 3 consecutive readings to change state. INA226 failure → solarOK=true (fail-safe). DS18B20 enclosure temp → fan control.

7. **Autonomous operation:** Schedule stored in NVM. Runs from local DS3231 RTC. If LoRa lost → continues last received schedule.

### Global Relay Mapping

| Global ID | Board | Local ID |
|-----------|-------|----------|
| 1-4 | B4 | 0-3 |
| 5-10 | B5 | 0-5 |
| 11-14 | B6 | 0-3 |
| 15-20 | B7 | 0-5 |

---

## 7. OPERATION MANUAL

> **Full manual:** See [`operation_manual.md`](operation_manual.md) — step-by-step setup, daily operation, wiring, troubleshooting.

### Quick Reference

1. Set dipswitch to Board ID (0=Master, 1-7=Slave)
2. Power on → Master creates WiFi AP `CherryOne_Copy` / password `11111111`
3. Connect to AP, open `192.168.4.1`, configure WiFi + Blynk token
4. Apply → ESP32 reboots and connects

### Daily Use

1. Open Blynk app
2. Set B1 config: V140 (start), V141 (ON min), V143 (OFF min)
3. Set B2 config: V144 (start), V145 (ON min), V147 (OFF min)
4. Set Round 1: V130 (start time) + V131 (duration per valve)
5. Set Round 2: V132 (start time) + V133 (duration)
6. Press **V19 (SYNC ALL)** → Master broadcasts all schedules
7. Monitor status LEDs V60-V86 + V150-V156 (LoRa online)
8. Toggle **V138** to mute/unmute Telegram alerts

### Manual Override

1. Set V134 (relay ID 1-20)
2. Set V135 (duration in minutes)
3. Press V136 (START) → Master sends LoRa command
4. B3 pump auto-activates with manual command

### Emergency

Press **V137 (STOP ALL)** → all relays OFF on all boards.

### Solar Behavior

- B2/B3 monitor PV panel voltage via INA226 Vbus
- Vbus ≥12.0V = sunlight → pump runs (3-reading debounce)
- Vbus ≤11.0V = night/cloud → pump pauses (safety: no high voltage at farm)
- Sunlight returns → pump resumes automatically
- INA226 failure → solarOK=true (fail-safe, never block pump on sensor fault)
- **B4-B7 valves continue regardless** (low power, no solar dependency)
- DS18B20 monitors enclosure temp → fan ON at 40°C, OFF at 38°C, solar-gated

### Telegram Alerts

Events that trigger Telegram messages (when V138 = unmuted):
- Slave offline (>30s no heartbeat)
- Slave back online
- Pump start / pump stop (B1, B2, B3)
- Solar pause / solar resume (B2, B3)
- System boot notification (Master)

---

## 8. FAILURE MODES

| Failure | Behavior |
|---------|----------|
| Master offline | Slaves run last NVM schedule from RTC |
| LoRa down | Same as above |
| Slave power loss | Restarts, loads NVM schedule, resumes |
| RTC dead | Schedule doesn't run (future: add fallback) |
| INA226 failure | Default: solarOK=true (allow pump) |
| INA226 stuck reading | 5s interval, no blocking |
| DS18B20 failure | Fan disabled (safe: no overheat risk without reading) |

---

## 9. FUTURE EXTENSIONS

- [x] ~~CMD_TIME_SYNC: Master NTP → slave RTC sync~~ (v1.1.0)
- [x] ~~B1/B2 individual Blynk config (V140-V147)~~ (v1.1.0)
- [x] ~~Telegram alerts (pump start/stop, solar low, offline)~~ (v1.1.0)
- [x] ~~Telegram mute toggle (V138)~~ (v1.1.0)
- [ ] Blynk solar threshold adjustment
- [ ] Blynk fan temp threshold adjustment
- [ ] OTA firmware updates via Blynk
- [ ] Master NVM backup of schedule (restore after power loss)

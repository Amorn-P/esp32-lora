# ESP32_Lora — Project Documentation

**Date:** 2026-05-21 | **Version:** 1.0.0 | **Status:** Active Development

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
09:00 ═══════════════════════════════════════════════════════════ 17:00
│                                                                     │
│ B1 PUMP:     [50m ON][10m OFF]...repeat to 17:00                    │
│                                                                     │
│ B3-B7 R1:    [B4-R1──1m overlap──B4-R2──...──B7-R20]               │
│ B3 PUMP:     ═══════════════════════════ (ON with valves)           │
│                                                    ~10:44           │
│ B2 PUMP:                               [start@computed]──[50/10]──┐│
│ B2 MUST END:                                           13:50 ◄────┘│
│                                                                     │
│ B3-B7 R2:                                     [14:00 start]────────→│
│ B3 PUMP:                                      ═════════════════════→│
```

### Board Functions

| Board | Type | Function | Relays |
|-------|------|----------|--------|
| AA (0) | Master | Blynk gateway, LoRa broker, LCD, NTP | — |
| B1 (1) | Slave | Independent pump | 1 relay |
| B2 (2) | Slave | Independent pump + solar sensor | 1 relay + INA226 |
| B3 (3) | Slave | Group pump + solar sensor | 1 relay + INA226 |
| B4 (4) | Slave | Sequential valves | 4 relays (global 1-4) |
| B5 (5) | Slave | Sequential valves | 6 relays (global 5-10) |
| B6 (6) | Slave | Sequential valves | 4 relays (global 11-14) |
| B7 (7) | Slave | Sequential valves | 6 relays (global 15-20) |

### Timing Rules

| Parameter | Value |
|-----------|-------|
| B1 ON/OFF | 50 min ON / 10 min OFF |
| B1 window | 09:00 - 17:00 |
| B2 ON/OFF | 50 min ON / 10 min OFF |
| B2 start | Auto: next 15-min boundary after R1 ends |
| B2 stop | 13:50 (10 min before R2) |
| B2-B3 power rule | NEVER overlap |
| B3-B7 rounds | 2 per day (R1=09:00, R2=14:00 default) |
| Valve overlap | 1 minute between sequential relays |
| Solar sensor | B2, B3 only — 30x30cm cell + INA226 |
| Solar threshold | < 50 mA = pause pump (3-reading debounce) |

---

## 3. HARDWARE PIN MAPPING

### Common (All Boards)

| Function | GPIO |
|----------|------|
| **LoRa SX1278 MOSI** | 23 |
| **LoRa SX1278 MISO** | 19 |
| **LoRa SX1278 SCK** | 18 |
| **LoRa SX1278 SS** | 5 |
| **LoRa SX1278 RST** | 4 |
| **LoRa SX1278 DIO0** | 2 |
| **I2C SDA** (DS3231 + LCD + INA226) | 21 |
| **I2C SCL** | 22 |
| **Dipswitch Bit0** (ID LSB) | 36 |
| **Dipswitch Bit1** | 39 |
| **Dipswitch Bit2** | 34 |
| **Dipswitch Bit3** | 35 |

### Relays (Slaves)

| Relay | GPIO | Note |
|-------|------|------|
| Relay 1 | 13 | |
| Relay 2 | 14 | |
| Relay 3 | 27 | |
| Relay 4 | 26 | |
| Relay 5 | 25 | |
| Relay 6 | 33 | |
| Relay 7 | 32 | |
| — | 12 | **SKIPPED** (boot strapping) |

### INA226 (B2, B3 only)

| Pin | Connection |
|-----|-----------|
| VCC | 3.3V |
| GND | GND |
| SDA | GPIO 21 |
| SCL | GPIO 22 |
| Address | 0x40 |
| Shunt | 0.1Ω, max 2A |

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
| CMD_TIME_SYNC | 3 | Master→Slave | (Future) RTC sync |
| CMD_SET_SCHEDULE | 4 | Master→Slave | Schedule update |

### Packet Field Usage by Target

| targetId | startHr1 | startMin1 | duration1 | startHr2 | startMin2 | duration2 |
|----------|----------|-----------|-----------|----------|-----------|-----------|
| 1 (B1) | 9 | 0 | 50 (ON) | 10 (OFF) | — | — |
| 2 (B2) | computed | computed | 50 (ON) | 10 (OFF) | — | — |
| 255 (B3-B7) | R1 Hr | R1 Min | R1 Dur | R2 Hr | R2 Min | R2 Dur |

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

### Dashboard (V1-V89)

| Pin | Widget | Function |
|-----|--------|----------|
| V1 | Value Display | System Status |
| V11 | Value Display | Time Remaining |
| V12 | Progress Bar | Session Progress |
| V19 | Button | **SYNC ALL SLAVES** |
| V20 | Label | Network Status |
| V21 | Terminal | Slave Relays Status |
| V60 | LED | Master AA |
| V61 | LED | B1 Pump |
| V62 | LED | B2 Pump |
| V63 | LED | B3 Main Pump |
| V66-V69 | LED | B4-R1 to B4-R4 |
| V70-V75 | LED | B5-R1 to B5-R6 |
| V76-V79 | LED | B6-R1 to B6-R4 |
| V81-V86 | LED | B7-R1 to B7-R6 |

### Configuration (V90-V255)

| Pin | Widget | Function |
|-----|--------|----------|
| V130 | TimeInput | B3-B7 P1 Start Time |
| V131 | NumericInput | B3-B7 P1 Duration (min/valve) |
| V132 | TimeInput | B3-B7 P2 Start Time |
| V133 | NumericInput | B3-B7 P2 Duration (min/valve) |
| V134 | NumericInput | Manual Relay ID (1-20) |
| V135 | NumericInput | Manual Duration (min) |
| V136 | Button | Manual START |
| V137 | Button | **STOP ALL** |
| V140-V147 | (reserved) | B1/B2 future config |

**CommCore reserved pins:** V90-V101, V118-V127 — DO NOT USE.

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
| master | 84.6% (1,108 KB) | 14.4% |
| slave | 20.0% (262 KB) | 4.7% |

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

1. **broadcastSchedule():** Sends 3 targeted packets:
   - B1 (targetId=1): Fixed 09:00, 50/10
   - B2 (targetId=2): Computed start after R1 ends
   - B3-B7 (targetId=255): R1 + R2 schedules

2. **B2 start time formula:**
   ```
   R1_end = P1_start + (20 × duration − 19)
   B2_start = ceil(R1_end / 15) × 15  (next 15-min boundary)
   ```

3. **broadcastManual():** Sends to valve board + B3 pump simultaneously.

4. **Heartbeat handler:** Decodes relayStatus bitmap → Blynk V61-V86 LEDs.

### Slave — Key Logic

1. **Scheduler::run() — Per-board dispatch:**

   | Board | Logic |
   |-------|-------|
   | B1 | 50/10 cycle, 09:00-17:00 |
   | B2 | 50/10 cycle, start-to-13:50, **solar check** |
   | B3 | Valve window check, **solar check** |
   | B4-B7 | Staggered overlap, R1 + R2 separate durations |

2. **B3 pump window:** Computes full 20-relay sequence. If ANY relay active → pump ON (if solar OK).

3. **Staggered overlap:**
   ```
   relay[i].start = periodStart + i × (duration − 1)
   relay[i].end   = relay[i].start + duration
   ```

4. **Solar sensor (B2/B3):** INA226 read every 5s. 3 consecutive low readings → pause pump. 3 consecutive good readings → resume.

5. **Autonomous operation:** Schedule stored in NVM. Runs from local DS3231 RTC. If LoRa lost → continues last received schedule.

### Global Relay Mapping

| Global ID | Board | Local ID |
|-----------|-------|----------|
| 1-4 | B4 | 0-3 |
| 5-10 | B5 | 0-5 |
| 11-14 | B6 | 0-3 |
| 15-20 | B7 | 0-5 |

---

## 7. OPERATION MANUAL

### Initial Setup

1. Set dipswitch to Board ID (0=Master, 1-7=Slave)
2. Power on → Master creates WiFi AP `CherryOne_Copy` / password `11111111`
3. Connect to AP, open `192.168.4.1`, configure WiFi + Blynk token
4. Apply → ESP32 reboots and connects

### Daily Use

1. Open Blynk app
2. Set Round 1: V130 (start time) + V131 (duration per valve)
3. Set Round 2: V132 (start time) + V133 (duration)
4. Press **V19 (SYNC ALL)** → Master broadcasts schedules
5. Monitor status LEDs V60-V86

### Manual Override

1. Set V134 (relay ID 1-20)
2. Set V135 (duration in minutes)
3. Press V136 (START) → Master sends LoRa command
4. B3 pump auto-activates with manual command

### Emergency

Press **V137 (STOP ALL)** → all relays OFF on all boards.

### Solar Behavior

- B2/B3 monitor their own 30×30cm solar cell via INA226
- Sunlight < 50 mA → pump pauses (3-reading debounce)
- Sunlight returns → pump resumes automatically
- **B4-B7 valves continue regardless** (low power)

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

---

## 9. FUTURE EXTENSIONS

- [ ] CMD_TIME_SYNC: Master NTP → slave RTC sync
- [ ] B1/B2 individual Blynk config (V140-V147)
- [ ] Blynk solar threshold adjustment
- [ ] Telegram alerts (pump start/stop, solar low, offline)
- [ ] OTA firmware updates via Blynk

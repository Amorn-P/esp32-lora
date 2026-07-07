# Blynk.md — ESP32_Lora Irrigation Control

**Version:** 1.1.1  |  **Date:** 2026-06-08  |  **Linked SPEC:** ESP32_Lora.md

> **Purpose:** Single reference for DAD to configure the Blynk app dashboard. All V-pins, widgets, labels, ranges, defaults, and dashboard layout. Generated from Blynk_tab.txt + ESP32_Lora.md.

**Blynk Server:** 43.229.135.169:8080 (Blynk Legacy, private server)

---

## 1. DASHBOARD LAYOUT

### Tab 1: Dashboard (Main Status)

```
┌──────────────────────────┐
│  V1: System Status       │  (Value Display)
│  V11: Time Remaining     │  (Value Display)
│  V12: Session Progress   │  (Progress Bar)
│  V19: SYNC ALL           │  (Button)
│  V20: Network Status     │  (Label)
│  V21: Relay Status       │  (Terminal)
│                          │
│  Master: V60             │  (LED)
│  B1 Pump: V61            │  (LED)
│  B2 Pump: V62            │  (LED)
│  B3 Pump: V63            │  (LED)
│                          │
│  B4 Valves: V66-V69      │  (LEDs)
│  B5 Valves: V70-V75      │  (LEDs)
│  B6 Valves: V76-V79      │  (LEDs)
│  B7 Valves: V81-V86      │  (LEDs)
│                          │
│  LoRa Online: V150-V156  │  (LEDs)
└──────────────────────────┘
```

### Tab 2: Configuration

```
┌──────────────────────────┐
│  B3-B7 P1: V130 (Time)   │
│  B3-B7 P1: V131 (Min)    │
│  B3-B7 P2: V132 (Time)   │
│  B3-B7 P2: V133 (Min)    │
│                          │
│  B1 P1: V140-V141        │
│  B1 P2: V142-V143        │
│  B2 P1: V144-V145        │
│  B2 P2: V146-V147        │
│                          │
│  Manual: V134-V136       │
│  EMERGENCY: V137 STOP    │
└──────────────────────────┘
```

---

## 2. STATUS & CONTROL PINS

| V-pin | Widget | Label | Range | Default | R/W | Description |
|-------|--------|-------|-------|---------|:---:|-------------|
| V1 | Value Display | System Status | Text | "OFFLINE" | R | Current activity label |
| V11 | Value Display | Time Remaining | Text | "—" | R | Remaining time in P1/P2/Manual |
| V12 | Progress Bar | Session Progress | 0-100 | 0 | R | Pump session percentage |
| V19 | Button | SYNC ALL SLAVES | 0/1 | 0 | W | Broadcast current schedules to all slaves |
| V20 | Label | Network Status | Text | — | R | Slave connectivity health summary |
| V21 | Terminal | Relay Status | Text | — | R | Per-slave relay status feed |
| V60 | LED | Master AA | 0/1 | 0 | R | Master online |

---

## 3. RELAY STATUS LEDs

| V-pin | Widget | Label | Board | Relay | Global ID |
|-------|--------|-------|-------|-------|-----------|
| V61 | LED | B1 Pump | B1 | R1 | — |
| V62 | LED | B2 Pump | B2 | R1 | — |
| V63 | LED | B3 Pump | B3 | R1 | — |
| V66 | LED | B4-R1 | B4 | R1 | 1 |
| V67 | LED | B4-R2 | B4 | R2 | 2 |
| V68 | LED | B4-R3 | B4 | R3 | 3 |
| V69 | LED | B4-R4 | B4 | R4 | 4 |
| V70 | LED | B5-R1 | B5 | R5 | 5 |
| V71 | LED | B5-R2 | B5 | R6 | 6 |
| V72 | LED | B5-R3 | B5 | R7 | 7 |
| V73 | LED | B5-R4 | B5 | R8 | 8 |
| V74 | LED | B5-R5 | B5 | R9 | 9 |
| V75 | LED | B5-R6 | B5 | R10 | 10 |
| V76 | LED | B6-R1 | B6 | R11 | 11 |
| V77 | LED | B6-R2 | B6 | R12 | 12 |
| V78 | LED | B6-R3 | B6 | R13 | 13 |
| V79 | LED | B6-R4 | B6 | R14 | 14 |
| V81 | LED | B7-R1 | B7 | R15 | 15 |
| V82 | LED | B7-R2 | B7 | R16 | 16 |
| V83 | LED | B7-R3 | B7 | R17 | 17 |
| V84 | LED | B7-R4 | B7 | R18 | 18 |
| V85 | LED | B7-R5 | B7 | R19 | 19 |
| V86 | LED | B7-R6 | B7 | R20 | 20 |

> **Note:** V64, V65, V80 are unused/available. V80 intentionally skipped.

---

## 4. LoRa CONNECTION STATUS LEDs

| V-pin | Widget | Label | Description |
|-------|--------|-------|-------------|
| V150 | LED | B1 Online | Green = LoRa heartbeat received from B1 |
| V151 | LED | B2 Online | Green = LoRa heartbeat received from B2 |
| V152 | LED | B3 Online | Green = LoRa heartbeat received from B3 |
| V153 | LED | B4 Online | Green = LoRa heartbeat received from B4 |
| V154 | LED | B5 Online | Green = LoRa heartbeat received from B5 |
| V155 | LED | B6 Online | Green = LoRa heartbeat received from B6 |
| V156 | LED | B7 Online | Green = LoRa heartbeat received from B7 |

---

## 5. B3-B7 MAIN SYSTEM SCHEDULE

| V-pin | Widget | Label | Range | Default | R/W | Description |
|-------|--------|-------|-------|---------|:---:|-------------|
| V130 | Time Input | B3-B7 P1 Start | HH:MM | — | W | Period 1 valve sequence start |
| V131 | Numeric Input | P1 Duration (min) | 1-120 | — | W | Minutes per valve in P1 |
| V132 | Time Input | B3-B7 P2 Start | HH:MM | — | W | Period 2 valve sequence start |
| V133 | Numeric Input | P2 Duration (min) | 1-120 | — | W | Minutes per valve in P2 |

---

## 6. B1 INDEPENDENT PUMP (Blynk-Configurable)

| V-pin | Widget | Label | Range | Default | R/W | Description |
|-------|--------|-------|-------|---------|:---:|-------------|
| V140 | Time Input | B1 P1 Start | HH:MM | — | W | B1 pump window start |
| V141 | Numeric Input | B1 ON Duration | 1-120 | 50 | W | Minutes ON per cycle |
| V142 | Time Input | B1 P2 Start | HH:MM | — | W | P2 start (optional) |
| V143 | Numeric Input | B1 OFF Duration | 1-60 | 10 | W | Minutes OFF between cycles |

**B1 logic:** Cycles [ON for V141 min] → [OFF for V143 min] within window V140→17:00.

---

## 7. B2 INDEPENDENT PUMP + SOLAR

| V-pin | Widget | Label | Range | Default | R/W | Description |
|-------|--------|-------|-------|---------|:---:|-------------|
| V144 | Time Input | B2 P1 Start | HH:MM | — | W | B2 pump start time |
| V145 | Numeric Input | B2 ON Duration | 1-120 | 50 | W | Minutes ON per cycle |
| V146 | Time Input | B2 P2 Start | HH:MM | — | W | P2 start (optional) |
| V147 | Numeric Input | B2 OFF Duration | 1-60 | 10 | W | Minutes OFF between cycles |

**B2 logic:** Cycles [ON for V145 min] → [OFF for V147 min]. Must END before R2_Start − 15 minutes (Master-enforced). **B2-B3 NEVER overlap.**

---

## 8. MANUAL CONTROL & EMERGENCY

| V-pin | Widget | Label | Range | Default | R/W | Description |
|-------|--------|-------|-------|---------|:---:|-------------|
| V134 | Numeric Input | Manual Relay ID | 1-20 | 1 | W | Global relay ID to activate |
| V135 | Numeric Input | Manual Duration | 1-120 | 5 | W | Minutes to keep relay ON |
| V136 | Button | Manual START | 0/1 | 0 | W | Execute manual relay command |
| V137 | Button | STOP ALL | 0/1 | 0 | W | **EMERGENCY SHUTDOWN** — all relays OFF |

> **V137 (STOP ALL):** Immediately turns off all relays system-wide. Persistent — no auto-recovery. Requires V19 (SYNC ALL) to resume schedule.

---

## 9. RESERVED SYSTEM PINS (CommCore — DO NOT MODIFY)

| V-pin Range | Purpose |
|-------------|---------|
| V90-V100 | WiFi / IP Configuration |
| V101 | Apply & Restart |
| V118-V119 | Telegram Bot Token + Group ID |
| V120 | Mode Toggle (AP/Fast) |
| V123-V127 | OTA Firmware Update + Reset Time |

---

## 10. WIDGET TYPE REFERENCE

| Blynk Widget | Used For |
|-------------|----------|
| Button | V19 (SYNC), V136 (Manual START), V137 (STOP ALL) |
| LED | V60-V86 (status), V150-V156 (LoRa online) |
| Value Display | V1 (status), V11 (remaining time) |
| Progress Bar | V12 (session progress) |
| Label | V20 (network status) |
| Terminal | V21 (relay status feed) |
| Time Input | V130, V132, V140, V142, V144, V146 |
| Numeric Input | V131, V133, V134, V135, V141, V143, V145, V147 |

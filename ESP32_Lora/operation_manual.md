# ESP32_Lora — Operation Manual (v1.1.1)

**Project:** ESP32_Lora Irrigation Control System  
**Date:** 2026-06-07 | **Updated:** 2026-06-07 (Post-Audit Fixes)  

---

## TABLE OF CONTENTS
1. [System Overview](#1-system-overview)
2. [Hardware Setup](#2-hardware-setup)
3. [First-Time Configuration](#3-first-time-configuration)
4. [Daily Operation](#4-daily-operation)
5. [Manual Override](#5-manual-override)
6. [Emergency Procedures](#6-emergency-procedures)
7. [Understanding the Schedule](#7-understanding-the-schedule)
8. [Telegram Alerts](#8-telegram-alerts)
9. [Solar & Fan System](#9-solar--fan-system)
10. [Troubleshooting](#10-troubleshooting)
11. [LED Status Reference](#11-led-status-reference)

---

## 1. SYSTEM OVERVIEW

The ESP32_Lora system controls irrigation across 7 slave boards from 1 master controller. All communication is via LoRa radio (433 MHz). Each slave operates independently — if the Master goes offline, slaves continue from their last received schedule using their own real-time clock.

```
Master AA (house)          LoRa 433MHz            Slaves (field)
┌──────────────┐       ┌──────────────┐       ┌──────────────┐
│ Blynk App    │  WiFi │              │  LoRa │ B1: Pump     │
│ Telegram     │◄─────►│  Master AA   │◄─────►│ B2: Pump+Sol │
│ Web Config   │       │              │       │ B3: GrpPump  │
└──────────────┘       └──────────────┘       │ B4: Valves 4 │
                                               │ B5: Valves 6 │
                                               │ B6: Valves 4 │
                                               │ B7: Valves 6 │
                                               └──────────────┘
```

### Board Quick Reference

| Board | ID | What It Controls | Special Hardware |
|-------|----|-----------------|------------------|
| AA | 0 | Master gateway | LCD2004, WiFi |
| B1 | 1 | Pump #1 | 1 relay |
| B2 | 2 | Pump #2 + Solar | 1 relay + INA226 + DS18B20 + Fan |
| B3 | 3 | Group Pump + Solar | 1 relay + INA226 + DS18B20 + Fan |
| B4 | 4 | Valves 1-4 | 4 relays |
| B5 | 5 | Valves 5-10 | 6 relays |
| B6 | 6 | Valves 11-14 | 4 relays |
| B7 | 7 | Valves 15-20 | 6 relays |

---

## 2. HARDWARE SETUP

### Setting the Board ID (Dipswitch)

Before powering on, set the 4-position DIP switch on each board:

| Board | DIP 1 | DIP 2 | DIP 3 | DIP 4 |
|-------|-------|-------|-------|-------|
| AA Master | OFF | OFF | OFF | OFF |
| B1 | ON | OFF | OFF | OFF |
| B2 | OFF | ON | OFF | OFF |
| B3 | ON | ON | OFF | OFF |
| B4 | OFF | OFF | ON | OFF |
| B5 | ON | OFF | ON | OFF |
| B6 | OFF | ON | ON | OFF |
| B7 | ON | ON | ON | OFF |

**⚠️ IMPORTANT:** Set DIP switch BEFORE powering on. Changing it while powered has no effect until reboot.

### Power Up Sequence

1. Set all DIP switches to correct board ID
2. Power up Master AA first — wait for LCD to show "Lucky-Lora AA"
3. Power up Slaves B1-B7 one at a time
4. Check LoRa online LEDs in Blynk (V150-V156) — all should be green within 30 seconds

### Antenna

- Use 433 MHz spring antenna on each SX1278 module
- Keep antenna vertical and as high as possible
- Typical range: 500m-2km open field (depends on terrain)

---

## 3. FIRST-TIME CONFIGURATION

### Step 1: Master WiFi Setup

1. Power on Master AA
2. On your phone/laptop, connect to WiFi network: **`CherryOne_Copy`**
3. Password: **`11111111`**
4. Open browser → go to `http://192.168.4.1`
5. Fill in:
   - **WiFi SSID:** Your home WiFi name
   - **WiFi Password:** Your home WiFi password
   - **Blynk Server:** `43.229.135.169`
   - **Blynk Port:** `8080`
   - **Blynk Token:** (get from Blynk Legacy app, see below)
   - **Telegram Bot Token:** (from @BotFather)
   - **Telegram Group ID:** (your Telegram group chat ID)
6. Click **Apply** → ESP32 reboots
7. LCD should now show an IP address (e.g., `192.168.1.50`)

### Step 2: Blynk App Setup

1. Install **Blynk Legacy** app (NOT Blynk 2.0)
2. Create new project → select **ESP32 Dev Board**
3. Connection type: **WiFi**
4. Copy the **Auth Token** (use this in Step 1)
5. Go to Settings → Server:
   - Server: `43.229.135.169`
   - Port: `8080`
6. Build dashboard using the widget table (see Section 4)

### Step 3: Telegram Setup (Optional)

1. Open Telegram → message `@BotFather`
2. Type: `/newbot` → follow prompts → copy token
3. Create a Telegram group → add your bot
4. Get group chat ID (send `/my_id` to `@RawDataBot`)
5. Enter bot token and group ID in web config (Step 1)
6. Restart Master AA

---

## 4. DAILY OPERATION

### Morning Routine (Before 09:00)

**Option A: Quick Start (Use Defaults)**
1. Open Blynk app
2. Press **V19 (SYNC ALL)** once
3. System runs all pumps and valves on default schedule

**Option B: Custom Schedule**
1. Open Blynk app → **Schedule Tab**
2. Set B1 pump (optional — defaults to 09:00-17:00, 50/10 cycle):
   - **V140** — Start time (default: 09:00)
   - **V141** — ON minutes (default: 50)
   - **V143** — OFF minutes (default: 10)
3. Set B2 pump (independent solar pump):
   - **V144** — Start time
   - **V145** — ON minutes (default: 50)
   - **V147** — OFF minutes (default: 10)
   - B2 stop is **automatic** = R2_Start − 15 min
   - ⚠️ **B2-B3 overlap protection:** If B2 start < R1 end + 15 min, Master auto-adjusts B2 to next 15-min boundary after R1. B2 NEVER runs during B3 valve windows.
4. Set Round 1 (first round of valves):
   - **V130** — Start time (default: 09:00)
   - **V131** — Duration per valve in minutes (default: 5)
5. Set Round 2 (second round of valves):
   - **V132** — Start time (default: 14:00)
   - **V133** — Duration per valve (default: 5)
6. Press **V19 (SYNC ALL)** — Master broadcasts to all slaves
   - 🔁 **Auto-retry:** If any slave misses the LoRa broadcast, Master auto-retries after 30 seconds (once)

### Monitoring During Operation

- **V60-V86 LEDs:** Show which relays are active in real-time
- **V150-V156 LEDs:** Show slave LoRa connection (green=online)
- **V11 Time Remaining:** Shows pump time left in current round
- **V12 Progress Bar:** Overall irrigation progress (0-100%)

### What NOT to Do

- ❌ Do NOT press V19 multiple times in quick succession
- ❌ Do NOT change schedule values while a round is active (wait for idle)
- ❌ Do NOT power off a slave during irrigation (use STOP ALL first)
- ⚠️ B2 will be auto-adjusted if it would overlap with B3's valve window
- 💡 B2/B3 solar status appears in Telegram (pause/resume alerts)

---

## 5. MANUAL OVERRIDE

Use manual mode to run a specific valve outside the schedule.

### How to Use Manual Mode

1. Go to Blynk **Schedule Tab**
2. **V134:** Set relay ID (1-20)
3. **V135:** Set duration (1-240 minutes)
4. **V136:** Press **START Manual**
5. Master sends LoRa command → valve opens + B3 pump turns on
6. Valve closes automatically after duration expires

### Manual Mode Rules

- B3 group pump turns on automatically with ANY manual command
- Manual mode overrides the scheduled program
- During manual mode, the schedule pauses for that board
- To return to schedule: let manual duration expire OR press **STOP ALL**
- B1 and B2 pumps are NOT targeted by manual mode (valve relays only)

---

## 6. EMERGENCY PROCEDURES

### STOP ALL (V137)

Press **V137** to immediately turn off ALL relays on ALL boards.

**When to use:**
- Pump running dry
- Pipe burst
- Electrical hazard
- Unexpected person/animal in area
- Any unsafe condition

**What happens:**
- All 20 relays turn OFF instantly
- Pumps stop, valves close
- Telegram alert: "⚠️ EMERGENCY STOP ACTIVATED"
- System stays stopped (persistent) — does NOT auto-restart

**To resume after STOP ALL:**
Press **V19 (SYNC ALL)** — Master rebroadcasts schedule, slaves resume.

### Master Offline

If Master AA loses power or WiFi:
- Slaves **continue working** from last received schedule
- No Blynk monitoring (LEDs frozen)
- No Telegram alerts
- When Master restarts, it reconnects automatically

### Slave Offline (LoRa Lost)

If a slave loses LoRa connection (>30 seconds):
- **Continues running** from NVM schedule + local RTC
- Blynk LED for that slave turns off (V150-V156)
- Telegram alert: "🔴 Bx Offline"
- When LoRa returns: auto-reconnects, Telegram: "🟢 Bx Online"

---

## 7. UNDERSTANDING THE SCHEDULE

### Daily Irrigation Timeline

```
09:00 ─────────────────────────────────────────────────────── 17:00
 │                                                              │
 │ B1: [50min ON][10min OFF]...[50min ON][10min OFF]            │
 │                                                              │
 │ R1: ┌──B4(1)──B4(2)──B4(3)──B4(4)──B5(5)──...──B7(20)┐    │
 │     └────── 20 valves, 1-min overlap × duration ──────┘    │
 │ B3: ═══════════════ Pump ON with valves ═══════════════     │
 │                                                  ~10:44     │
 │                                R1_end + 15min                │
 │                                         ↓                    │
 │ B2:                         ┌────[Auto-adj Start]──[50/10]─┐│
 │                             │ B2 NEVER overlaps with B3    ││
 │                             └─────────────┤   R2-15min     ││
 │                                       13:45 ← stop          ││
 │ R2: ←─ 14:00 start ────────────────────────────────→       │
 │ B3: ═══════════════ Pump ON with valves ═══════════════     │
 │                                                              │
17:00 ─────────────────────────────────────────────────────────
```

### B1 Pump Schedule (Independent)

- **Configurable via V140-V143 in Blynk**
- Default: 09:00-17:00, 50 min ON / 10 min OFF per cycle
- No solar dependency — runs regardless of sunlight
- No interaction with B2 or B3 pump

### B2 Pump Schedule (Independent + Solar)

- **Configurable via V144-V147 in Blynk**
- Start: V144 (Blynk TimeInput)
- Stop: **Auto-computed** as R2_Start − 15 minutes
- Cycle: V145 ON / V147 OFF per cycle
- **Solar gate:** Pump pauses when solar panel voltage < 12V (night/cloud)
- ⚠️ **B2-B3 overlap protection:**
  - Master auto-adjusts B2 start if it would overlap with R1 valve window
  - Slave blocks B2 pump during active B3 valve windows
  - If B2 start < R1 end + 15 min → auto-pushed to next 15-min boundary
- B2 does NOT run when B3 is pumping

### B3-B7 Group Schedule (Valve Rounds)

- **Round 1:** V130 start + V131 duration per valve
- **Round 2:** V132 start + V133 duration per valve
- 20 global relays sequenced with 1-minute overlap
- B3 pump runs whenever ANY valve is active
- B4-B7 valves run independently of solar

### Valve Overlap Logic

Each relay starts 1 minute before the previous relay ends:
```
Relay 1: [████████████]         (5 min, starts at 09:00)
Relay 2:     [████████████]     (5 min, starts at 09:04)
Relay 3:         [████████████] (5 min, starts at 09:08)
...
```

Number of relay changes per minute = exactly 1 valve transitions. Total Round 1 time = `20 × duration − 19` minutes.

---

## 8. TELEGRAM ALERTS

### Events That Trigger Alerts

| Event | Message | Trigger |
|-------|---------|---------|
| Slave online | "🟢 Bx Online" | First heartbeat received |
| Slave offline (>30s) | "🔴 Bx Offline" | Heartbeat lost |
| Emergency STOP | "⚠️ EMERGENCY STOP ACTIVATED" | V137 pressed |
| Manual mode | "🔧 Manual: Relay X, Y min" | V136 pressed |
| Solar Paused (B2/B3) | "🌑 Bx Solar Paused (night/cloud)" | Slave reports INA226 < 12V |
| Solar Resume (B2/B3) | "☀️ Bx Solar Resume (sun)" | Slave reports INA226 ≥ 12V |

### How Solar Alerts Work

The slave packs solar + fan status into every heartbeat (10-second interval).
Master detects changes in solar state and sends Telegram alerts:
- **bit 0** = solarOK (1 = sun, 0 = night/cloud)
- **bit 1** = fanOn (1 = running, 0 = off)

Only B2 and B3 report solar status. B1/B4-B7 always report "sun" (no solar sensor).

### Muting Alerts

Press **V138** in Blynk to toggle mute/unmute:
- **V138 LED = 0 (off):** Alerts active (default)
- **V138 LED = 255 (on):** Alerts muted

---

## 9. SOLAR & FAN SYSTEM

### How Solar Gating Works (B2, B3 only)

The INA226 sensor measures solar panel voltage:
- **≥ 12.0V** = Sun is shining → pump runs
- **≤ 11.0V** = Night or heavy cloud → pump pauses
- **1V hysteresis** prevents rapid on/off from passing clouds
- **3 consecutive readings** required to change state (15-second debounce)

**Safety Rule:** If INA226 sensor fails, pump is ALLOWED to run (fail-safe).

### Fan Control (B2, B3 only)

The DS18B20 temperature sensor monitors enclosure temperature:
- **≥ 40°C** → Fan turns ON
- **≤ 38°C** → Fan turns OFF
- Fan only runs when solar power is available (solar-gated)

---

## 10. TROUBLESHOOTING

| Symptom | Likely Cause | Action |
|---------|-------------|--------|
| Master LCD blank | Power issue | Check 5V supply, USB cable |
| Master shows "WiFi: ---" | WiFi config wrong | Reconnect to AP `192.168.4.1` and reconfigure |
| Blynk LEDs frozen | Master WiFi down or Blynk server unreachable | Check Master WiFi, check Blynk server |
| Slave LED shows offline (V150-V156) | LoRa range or power issue | Check antenna, check slave power, reduce distance |
| Pump not running but schedule says ON | Solar gating (B2/B3) or B2 blocked by B3 window | Check solar panel, wait for sun, check INA226, check Telegram for solar alerts |
| Pump not running, no solar issue | RTC time wrong on slave, schedule not received | Press V19 to re-sync. Master auto-retries after 30s |
| Valve skipping / wrong order | Schedule not broadcast | Press V19 (SYNC ALL) |
| Relay clicking rapidly | 1-min overlap working correctly | Normal operation — 1 valve transitions every minute |
| Telegram no alerts | Bot token/group wrong, or muted (V138) | Check web config, check V138 |

### Serial Debug (Advanced)

Connect USB to any board, open Serial Monitor (115200 baud).

**Slave commands (Wokwi/normal):**
```
status          → Show board state
```

**Normal Serial output shows:**
- `[LoRa] Init SX1278 ... OK`
- `[LoRa] Schedule: R1 09:00/5min, R2 14:00/5min, offDur=10 stop=13:45`
- `[LoRa] Heartbeat sent`
- `[Solar] SUN (Vbus=13.2V, I=500mA)` or `[Solar] NIGHT (Vbus=8.3V) — pausing pump`
- `[Fan] ON (42.3°C)` or `[Fan] OFF (37.5°C)`

**Master Serial output shows:**
- `[LoRa] B2 auto-adjusted to 10:45 (after R1+15)` — overlap protection
- `[LoRa] Retry broadcast (unacked slaves)` — schedule retry
- `[NTP] RTC synced: 1749286800` — time sync

---

## 11. LED STATUS REFERENCE

### Master AA LCD2004

```
Line 1: Lucky-Lora AA
Line 2: HH:MM:SS (current time)
Line 3: Online: X/7 (connected slaves)
Line 4: IP: 192.168.x.x
```

### Blynk LEDs

| LED | Color | Meaning |
|-----|-------|---------|
| V60 | Blue | Master AA online |
| V61 | Red | B1 pump relay ON |
| V62 | Red | B2 pump relay ON |
| V63 | Red | B3 pump relay ON |
| V66-V86 | Green | Individual valve relay ON |
| V150-V156 | Green | Slave board LoRa connected |
| V150-V156 | Off | Slave board offline |
| V138 | On | Telegram alerts MUTED |
| V138 | Off | Telegram alerts ACTIVE |

---

## APPENDIX A: Schedule Default Values

| Parameter | V-Pin | Default |
|-----------|-------|---------|
| B1 Pump Start | V140 | 09:00 |
| B1 Pump ON Duration | V141 | 50 min |
| B1 Pump OFF Duration | V143 | 10 min |
| B2 Pump Start | V144 | 09:00 |
| B2 Pump ON Duration | V145 | 50 min |
| B2 Pump OFF Duration | V147 | 10 min |
| B2 Pump Stop | — | Auto = R2 Start − 15 min |
| Round 1 Start | V130 | 09:00 |
| Round 1 Duration | V131 | 5 min/valve |
| Round 2 Start | V132 | 14:00 |
| Round 2 Duration | V133 | 5 min/valve |

## APPENDIX B: Blynk Widget Setup

When building the Blynk Legacy dashboard, use these widget types:

| Widget | Blynk Name | Used For |
|--------|-----------|----------|
| Button | Button (PUSH mode) | V19, V136, V137, V138 |
| LED | LED | V60-V86, V150-V156 |
| Value Display | Labeled Value | V1, V11 |
| Progress Bar | Level | V12 |
| Terminal | Terminal | V21 |
| Time Input | Time Input | V130, V132, V140, V144 |
| Number Input | Slider/Step | V131, V133, V134, V135, V141, V143, V145, V147 |

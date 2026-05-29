# CherryOne — ESP32 RS485 Pump Controller v2.0.0

## Status: PRODUCTION READY (2026-05-28)
All systems verified: No phantom relay toggling, Manual/Sch1/Sch2 all stable over 24h+ testing.

## Hardware
- **Master:** ESP32 38-pin (DOIT DevKit v1)
- **RTC:** DS1302 (CE=15, IO=4, CLK=5)
- **RS485:** Serial2 (RXD2=16, TXD2=17), 9600 baud
- **Relays:** 5× 8-channel RS485 Modbus RTU modules (Slave ID 1–5)
- **Total:** 21 Solenoid Valves + 1 Pump

## Platform
- **PlatformIO:** espressif32@3.5.0 (ESP32 Arduino Core 1.0.6)
- **Framework:** Arduino
- **Flash:** 83.9% used (1.10 MB / 1.31 MB)
- **RAM:** 14.0% used (45.9 KB / 320 KB)

## Key Libraries
| Library | Version | Purpose |
|---------|---------|---------|
| IotWebConf | 3.0.4 | WiFi Manager & Web Config |
| Blynk | 0.6.1 | Blynk Legacy (port 8080, no SSL) |
| UniversalTelegramBot | 1.3.0 | Telegram alerts |
| DFRobot_RTU | 1.0.3 | Modbus RTU Master |
| DS1302 | — | RTC timekeeping |
| NTPClient | 3.2.1 | Network time sync |
| ArduinoJson | 7.0.4 | JSON parsing |
| SimpleTimer | — | Timer callbacks |
| Bounce2 | 2.72 | Button debouncing |

## CHANGELOG v2.0.0 (2026-05-28)

### Critical Fix
- **Phantom relay toggling eliminated** — Replaced dangerous `writeHoldingRegister(slave, 0x00FF, random)` heartbeat with safe `readHoldingRegister(slave, 0x0001, &data, 1)`. The old code wrote random `millis()` values to an unvalidated register every 2 seconds, which cheap relay boards interpreted as coil commands.

### High-Impact Improvements
- **RS485 bus mutex** (`g_rs485Mutex`) — prevents concurrent Modbus access across Task3, Task6, and Blynk callbacks
- **Modbus write retry** — 3 attempts per write with 15 ms backoff, plus verification
- **State guard** reduced from 10 s → 2 s — corrects noise-induced glitches 5× faster
- **Task priority differentiation** — Relay control (3) > Stop/Blynk (2) > Web/Time (1)
- **Bus recovery** — automatic Serial2 flush after 10 consecutive Modbus timeouts

### Medium Improvements
- **Boot relay safety** — RS485 TX pin driven LOW before Serial2 initializes
- **Hardware WDT** — 60 s timeout with panic on expiry, proactive feed at 25 s
- **Error counters** — per-cycle Modbus error statistics for field diagnostics
- **Safe stop/verify** — `forceAllRelaysOffOnce()` now acquires RS485 mutex and retries

### Build Changes
- Removed TFT_eSPI dependency (not used)
- Pinned Blynk to Legacy 0.6.1 (prevents accidental IoT 1.3.x install)
- Removed `CONFIG_HEAP_POISONING_COMPREHENSIVE` (Core 1.0.6 incompatibility)
- `platformio.ini` now minimal: only `-DCORE_DEBUG_LEVEL=0`, loop stack, and production flag

## Features
- Web portal (IotWebConf) for WiFi / Blynk / Telegram config
- Blynk Legacy dashboard (47 virtual pins)
- Telegram notifications (start / stop / status)
- 3 operating modes: Period 1, Period 2, Manual (mutual exclusion)
- Manual single-relay mode with live switching
- RS485 Modbus watchdog (auto-shutoff on ESP32 power loss, 5 s timeout)
- EEPROM validation with auto-recovery from corruption
- OTA firmware update via Blynk
- Power-loss resume (< 1 minute lost)
- Hardware Watchdog (60 s) + Software Watchdog (proactive feed)

## Documentation
| File | Purpose |
|------|---------|
| `Requirement.txt` | Full requirements & architecture |
| `operation_manual.txt` | User operation guide |
| `Blynk_V-pins.txt` | Complete Blynk pin mapping |
| `User_config.h` | All credentials and defaults |
| `docs/RS485_HARDWARE_GUIDE.md` | RS485 termination, biasing & cabling |

## Backup
`CherryOne_Backup_20260520_2200` (v1.2.0 — last stable before v2.0)

---
*Built with PlatformIO + VS Code | ESP32 Core 1.0.6 | CherryOne v2.0.0*

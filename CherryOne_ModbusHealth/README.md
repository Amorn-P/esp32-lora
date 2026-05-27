# CherryOne - ESP32 RS485 Pump Controller v1.2.0

## Status: STABLE (2026-05-20)
All systems verified working: WiFi, Internet, Blynk, Telegram, Manual, Sch1, Sch2

## Hardware
- **Master:** ESP32 38-pin
- **RTC:** DS1302 (CE=15, IO=4, CLK=5)
- **RS485:** Serial2 (RXD2=16, TXD2=17), 9600 baud
- **Relays:** 5x 8-channel RS485 Modbus RTU modules (Slave ID 1-5)
- **Total:** 21 Solenoid Valves + 1 Pump

## Platform
- **PlatformIO:** espressif32@3.5.0 (ESP32 Arduino Core 1.0.6)
- **Framework:** Arduino
- **Flash:** ~85% used (1.1MB / 1.3MB)
- **RAM:** ~14% used (46KB / 320KB)

## Key Libraries
| Library | Version | Purpose |
|---------|---------|---------|
| IotWebConf | 3.0.4 | WiFi Manager & Web Config |
| Blynk | 0.6.1 | Blynk Legacy (port 8080, no SSL) |
| UniversalTelegramBot | 1.3.0 | Telegram alerts |
| DFRobot_RTU | 1.0.3 | Modbus RTU Master |
| DS1302 | - | RTC timekeeping |
| NTPClient | 3.2.1 | Network time sync |
| ArduinoJson | 7.0.4 | JSON parsing |
| SimpleTimer | - | Timer callbacks |

## Features
- Web portal (IotWebConf) for WiFi/Blynk/Telegram config
- Blynk Legacy dashboard (47 virtual pins)
- Telegram notifications (start/stop/status)
- 3 operating modes: Period 1, Period 2, Manual (mutual exclusion)
- Manual single-relay mode with live switching
- RS485 Modbus watchdog (auto-shutoff on ESP32 power loss)
- EEPROM validation with auto-recovery from corruption
- OTA firmware update via Blynk
- Power-loss resume (< 1 minute lost)
- Hardware + Software Watchdog

## Documentation
- `Requirement.txt` - Full requirements & architecture
- `operation_manual.txt` - User operation guide
- `Blynk_V-pins.txt` - Complete Blynk pin mapping
- `User_config.h` - All credentials and defaults

## Backup
`CherryOne_Backup_20260520_2200`

---
*Built with PlatformIO + VS Code | ESP32 Core 1.0.6*
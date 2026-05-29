# TEAM WORKFLOW: Papa + Sherry (Hermes Agent)

## 1. The Roles

| Role | Entity | Responsibility |
|:---|:---|:---|
| **Architect** | **Papa** | Concepts, requirements, hardware, final approval, field testing |
| **Engineer** | **Sherry** | Analysis, architecture, code review, stability, docs |

## 2. CommCore Framework (2026-05-20)

CherryOne is the **Master Template**. Its `CommCore/` folder is copied to all future projects:

| Project | Purpose | Status |
|:---|:---|:---|
| CherryOne | RS485 Pump Controller (21 SV + Pump) | **PRODUCTION v2.0.0** |
| ESP32_Solar_Control | Solar pump controller | Planned |
| ESP32_Tracking_Solar | Solar tracker | Planned |
| ESP32_Lora | LoRa communication | Planned |
| ESP32_Weather_Station | Weather monitoring | Planned |

## 3. Project Initiation Protocol

1. Copy `CommCore/` folder to new project
2. Copy template files: `User_config.h`, `12_List_Wf.h`, `01_Social_Handlers.h`
3. Create project-specific: `00_Blynk_App.h`, `Pins_config.h`, `Relay_config.h`
4. Define your FreeRTOS tasks
5. WiFi/Blynk/Telegram/OTA comes free with CommCore

## 4. Communication Rules

- **Sherry** handles all code, architecture, docs
- **Papa** confirms hardware tests, gives requirements
- All code fixes applied directly — explain only when asked

## 5. Key Technical Decisions

- Platform: `espressif32@3.5.0` (Core 1.0.6) — mandatory for Blynk Legacy
- Blynk: `43.229.135.169:8080` — no SSL, library pinned to 0.6.1 (NOT 1.3.x IoT)
- DHCP preferred over Static IP (DNS routing issues)
- NO `EEPROM.begin()` before `IotWebConf.init()` (Core 1.0.6 auto-inits)
- ALL Blynk writes use `BLYNK_WRITE_SAFE` mutex
- ALL Modbus operations acquire `g_rs485Mutex` before touching Serial2
- RS485 Modbus watchdog for power-loss safety (register 0x00FE, 5 s timeout)
- Slave heartbeat: safe READ-based (register 0x0001) — NEVER write to unknown registers
- Modbus writes: 3-attempt retry with 15 ms backoff
- State guard: re-assert all relay states every 2 seconds
- Task priorities: Relay (3) > Stop/Blynk (2) > Web/Time (1)
- Boot procedure: Serial.begin BEFORE hardware pin config (for crash visibility)

## 6. CherryOne v2.0.0 — Critical Bug Fix History

| Date | Version | Change |
|:---|:---|:---|
| 2026-05-28 | v2.0.0 | **Phantom relay fix**: replaced `writeHoldingRegister(0x00FF, random)` heartbeat with safe `readHoldingRegister(0x0001)`. Added RS485 mutex, Modbus retry, state guard 2 s, task priorities, bus recovery. |
| 2026-05-20 | v1.2.0 | Stable baseline: WiFi, Internet, Blynk, Telegram, Manual, Sch1, Sch2 all working |

## 7. Flash / Deploy Checklist

- [ ] `pio run` passes clean
- [ ] First flash: `pio run --target erase --upload-port COMx` then `pio run --target upload --upload-port COMx`
- [ ] Reconfigure WiFi via AP (CherryOne / 11111111 @ 192.168.4.1) if EEPROM was erased
- [ ] Verify Blynk dashboard connects (server 43.229.135.169:8080)
- [ ] Test Manual mode with 1 relay for 1 minute
- [ ] Test Sch1 with short duration
- [ ] Check V82-V86 all show alive
- [ ] Let run overnight, check for phantom toggles

---
*Updated: 2026-05-28 | CherryOne v2.0.0 — Production Ready*

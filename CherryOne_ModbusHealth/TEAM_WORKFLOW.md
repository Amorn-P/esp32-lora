# TEAM WORKFLOW: Papa + Sherry (Hermes Agent)

## 1. The Roles

| Role | Entity | Responsibility |
|:---|:---|:---|
| **Architect** | **Papa** | Concepts, requirements, hardware, final approval |
| **Engineer** | **Sherry** | Analysis, architecture, code review, stability, docs |

## 2. CommCore Framework (2026-05-20)

CherryOne v1.2.0 is the **Master Template**. Its `CommCore/` folder is copied to all future projects:

| Project | Purpose | Status |
|:---|:---|:---|
| CherryOne | RS485 Pump Controller (21 SV + Pump) | **COMPLETE** |
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
- Blynk: `43.229.135.169:8080` — no SSL
- DHCP preferred over Static IP (DNS routing issues)
- NO `EEPROM.begin()` before `IotWebConf.init()`
- ALL Blynk writes use `BLYNK_WRITE_SAFE` mutex
- RS485 Modbus watchdog for power-loss safety (0x00FE/0x00FF)

---
*Updated: 2026-05-20 | CherryOne v1.2.0 stable*

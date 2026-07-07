# NEXT_STEPS.md — ESP32 2-Axis Astronomical Solar Tracker

**Date:** 2026-06-08  |  **Current Status:** Code compiled, Wokwi simulation pending

---

## Current State

| Stage | Status | Notes |
|-------|:---:|-------|
| 1. Compile | ✅ | All envs build SUCCESS |
| 2. Wokwi Sim | 🔧 | Parts generated — DAD wires connections, test with stubs |
| 3. Bench Flash | ❌ | Physical board not yet flashed |
| 4. Field Test | ❌ | Not deployed to Uthai Thani |
| 5. Docs | 🔧 | SPEC ✅, BOM ✅, wiring ✅ — operation_manual needed |

---

## What DAD Needs to Do Next

### 1. Wokwi Simulation

- [ ] Open `diagram.json` in wokwi.com
- [ ] Wire all I2C devices (SDA 21, SCL 22) — MPU6050 at 0x69, DS3231 at 0x68, QMC5883L at 0x0D, LCD at 0x27
- [ ] Wire BTS7960 motor driver substitutes (4 LEDs on GPIO 25, 26, 27, 14)
- [ ] Wire RS485 wind sensor substitute (potentiometer to GPIO 34 for wind simulation)
- [ ] Upload firmware.bin
- [ ] Test scenarios:
  - `time 2026-06-08 12:00:00` — verify sun azimuth/elevation calculation
  - `wind 15` — verify stow mode triggers
  - `angle 30 180` — verify motor reacts to simulated MPU6050/QMC5883L
- [ ] Paste wired diagram.json back to VS Code

### 2. Bench Flash

- [ ] Wire physical ESP32 per `wiring.md`
- [ ] Run I2C scanner sketch — verify all 4 devices respond (0x68, 0x69, 0x0D, 0x27)
- [ ] Flash firmware via PlatformIO: `pio run -t upload`
- [ ] Verify LCD displays 4-line UI
- [ ] Test motor outputs with logic analyzer (before connecting BTS7960)
- [ ] Test RS485 communication with wind sensor (if available)

### 3. Calibration

- [ ] QMC5883L hard iron calibration at installation site (metal solar frame distorts compass)
- [ ] MPU6050 tilt calibration (flat reference)
- [ ] Verify sun position calculation against smartphone app at known times

### 4. Field Deployment — Uthai Thani Farm

- [ ] Mount panel frame with MPU6050 + QMC5883L
- [ ] Install DS3231 with correct time
- [ ] Wire BTS7960 to slew drive and linear actuator
- [ ] Install anemometer on pole near panel
- [ ] Run full-day tracking test
- [ ] Verify wind stow triggers at threshold
- [ ] Check for mechanical binding at extreme angles

---

## Known Risks

| Risk | Severity | Mitigation |
|------|----------|------------|
| I2C address conflict (MPU6050 vs DS3231 both 0x68) | High | AD0 pin MUST be 3.3V. Test with I2C scanner first. |
| QMC5883L distorted by metal frame | Medium | Needs hard iron calibration at install site |
| Wind sensor Modbus timeout (unplugged) | Low | Code implements fault-tolerant mode — ignores wind, keeps tracking |
| BTS7960 inrush current | Medium | Soft-start PWM ramp already in code |
| Mechanical limits (over-rotation) | Medium | Code limits: 0-90° elevation, 0-360° azimuth. Physical limit switches recommended. |

---

## Wokwi Simulation Trap Checklist

| # | Trap | SolarTracking Fix | Status |
|---|------|-------------------|:---:|
| 1 | I2C hangs if init but not wired | Need `#ifdef WOKWI` guards | ⬜ |
| 2 | LCD `lcd.begin()` fails silently | Use `lcd.init()` | ⬜ |
| 3 | Serial blank on fast boot | `delay(100); while(!Serial)` | ⬜ |
| 4 | UI task starved by motor tasks | Mutex timeout ≥ 1000ms | ⬜ |
| 5 | `snprintf` stack overflow | Stack ≥ 4096 bytes for LCD format tasks | ⬜ |

> These 5 traps were discovered during SolarTracking development and MUST be checked before Wokwi simulation.

---

## Documentation Backlog

- [ ] `operation_manual.md` — User guide with calibration steps, LCD display guide, error codes
- [ ] Final BOM verification after field deployment
- [ ] Zip project without `.pio` folder once all docs complete

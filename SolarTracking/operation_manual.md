# operation_manual.md — ESP32 2-Axis Astronomical Solar Tracker

**Version:** 1.0.0  |  **Date:** 2026-06-08  |  **Linked SPEC:** SPEC.md

---

## 1. SYSTEM OVERVIEW

The ESP32 Solar Tracker continuously aligns a solar panel to face the sun using astronomical calculations — no light sensors needed. It knows where the sun should be based on GPS coordinates (Uthai Thani farm: Lat 15.1856678°, Lon 99.6898316°) and real-time clock. An IMU and digital compass mounted on the panel frame provide position feedback. BTS7960 motor drivers power the pan (azimuth) and tilt (elevation) motors. An RS485 wind sensor triggers "stow mode" (flat horizontal) during high winds.

---

## 2. POWER-UP SEQUENCE

1. **Apply 12V/24V power** — buck converter powers ESP32 at 5V
2. **LCD backlight on** within 1 second
3. **Self-test display** (2 seconds):
   - "ESP32 Solar Tracker"
   - "v1.0.0 — Init..."
4. **I2C bus scan** — verifies all 4 devices respond:
   - DS3231 RTC (0x68), MPU6050 (0x69), QMC5883L (0x0D), LCD (0x27)
5. **If any device fails:** Display shows "ERR: [Device] MISSING" and system halts
6. **If all OK:** Tracking begins within 5 seconds

---

## 3. LCD DISPLAY GUIDE

The 2004 LCD shows 4 lines, updated every second:

```
Line 1: Sun: Az 180° El 45°     ← Target sun position (calculated)
Line 2: Pan: Az 178° El 44°     ← Actual panel position (measured)
Line 3: Wind: 12.5 km/h         ← Wind speed from anemometer
Line 4: Mode: TRACKING          ← Current operating mode
```

### Line 3 Variations

| Display | Meaning |
|---------|---------|
| `Wind: 12.5 km/h` | Normal — wind below stow threshold |
| `Wind: UNPLUGGED!` | RS485 anemometer not responding — tracking continues |
| `Wind: STOW! 45km/h` | Wind above threshold — stow mode active |

### Line 4 Modes

| Mode | Meaning |
|------|---------|
| `TRACKING` | Normal operation — following the sun |
| `WIND STOW` | Panel flat (0° elevation) due to high wind |
| `IDLE (Night)` | Sun below horizon — motors off |
| `CALIBRATE` | Calibration mode active |
| `ERROR: [detail]` | Fault — see troubleshooting |

---

## 4. OPERATING MODES

### 4.1 Normal Tracking (Default)

- Solar position recalculated every 1 second
- Position sensors (MPU6050 + QMC5883L) read at 10 Hz
- Motors activated when error exceeds 2° deadband
- Soft-start PWM ramp over 500ms to protect gearing
- Panel range: 0-90° elevation, 0-360° azimuth

### 4.2 Night Mode (Auto)

- Triggered when calculated sun elevation < 0° (below horizon)
- All motors OFF
- Panel holds last position
- Resumes tracking at sunrise

### 4.3 Wind Stow Mode (Safety)

- Triggered when wind speed exceeds `STOW_THRESHOLD` (default: 40 km/h)
- Panel immediately moves to **0° elevation** (flat horizontal) — safest position
- Azimuth stays at last position
- Display shows "WIND STOW"
- **Auto-recovery:** Returns to tracking when wind drops below threshold for 60 seconds (hysteresis prevents flapping)

### 4.4 Fault-Tolerant Mode

- If wind sensor unplugged/times out: `WIND_FAULT = TRUE`
- **Tracking continues normally** (ignores missing wind data)
- Display shows "UNPLUGGED!" on line 3
- No stow protection available — monitor weather manually

---

## 5. CALIBRATION PROCEDURE

### 5.1 QMC5883L Magnetometer (Hard Iron Calibration)

Required because the metal solar frame distorts the magnetic field.

**Procedure:**
1. Enter calibration mode (future: via Serial command `cal`)
2. Slowly rotate the panel frame 360° on the azimuth axis (takes ~2 minutes)
3. ESP32 records min/max X, Y, Z readings
4. Offsets calculated and stored in NVM
5. LCD confirms: "CAL: OK" or "CAL: FAIL (Retry)"

**After calibration:** Azimuth readings should be accurate within ±5°.

### 5.2 MPU6050 Tilt Calibration

- Place panel at known flat/horizontal position (use spirit level)
- Send Serial command: `cal tilt`
- System records pitch/roll offset
- Elevation readings should now match physical angle

### 5.3 Time Verification

- DS3231 RTC should be set before first use
- Verify against smartphone clock
- If time drifts, replace CR2032 backup battery on RTC module

---

## 6. WIND STOW CONFIGURATION

| Parameter | Default | Serial Command | Notes |
|-----------|---------|---------------|-------|
| Stow Threshold | 40 km/h | `wind stow 40` | Wind speed to trigger stow |
| Recovery Hysteresis | 5 km/h | `wind hyster 5` | Must drop this far below threshold before recovery |
| Recovery Delay | 60 sec | `wind delay 60` | Hold below threshold for this long before resuming |

---

## 7. ERROR CODES & TROUBLESHOOTING

| Error | LCD Display | Cause | Fix |
|-------|-------------|-------|-----|
| RTC Missing | `ERR: RTC MISSING` | DS3231 not on I2C bus | Check wiring, SDA/SCL connections |
| MPU Missing | `ERR: MPU MISSING` | MPU6050 not responding | Check AD0=3.3V, verify address 0x69 |
| QMC Missing | `ERR: QMC MISSING` | QMC5883L not responding | Check wiring, verify address 0x0D |
| LCD Missing | No display at all | LCD not on I2C bus | Check VCC=5V, SDA/SCL |
| No Movement | Mode=TRACKING but motors silent | Deadband too small, or BTS7960 fault | Check 12V power, PWM signals with logic analyzer |
| Wrong Direction | Panel moves opposite to sun | Motor polarity reversed | Swap M+/M- on BTS7960 |
| Wind UNPLUGGED | Persistent unplugged | Anemometer disconnected | Check RS485 A/B wiring, 120Ω termination, baud rate |
| Jittery Movement | Panel oscillates | Deadband too small or sensor noise | Increase deadband to 3°, check MPU6050 mounting |

---

## 8. MAINTENANCE

### Daily (Automatic)
- DS3231 time verification against solar position calculation
- I2C device health check on boot

### Weekly
- [ ] Inspect BTS7960 heatsinks — should be warm, not hot
- [ ] Check all cable connections for corrosion (farm environment)
- [ ] Verify LCD backlight brightness

### Monthly
- [ ] Clean MPU6050 and QMC5883L enclosures — dust affects readings
- [ ] Check slew drive grease
- [ ] Verify linear actuator full range of motion

### Every 6 Months
- [ ] Re-run QMC5883L hard iron calibration
- [ ] Check buck converter output (5V ±0.2V)
- [ ] Replace DS3231 CR2032 backup battery
- [ ] Inspect RS485 termination resistor (120Ω)

---

## 9. SERIAL COMMANDS (Debug/Maintenance)

| Command | Action |
|---------|--------|
| `status` | Print all sensor readings and system state |
| `cal mag` | Start QMC5883L calibration |
| `cal tilt` | Start MPU6050 calibration |
| `wind stow N` | Set stow threshold to N km/h |
| `wind delay N` | Set recovery delay to N seconds |
| `manual pan N` | Move pan to N degrees (for testing) |
| `manual tilt N` | Move tilt to N degrees (for testing) |
| `stop` | Emergency stop — all motors OFF |
| `reboot` | Software restart |
| `i2cscan` | Scan I2C bus for connected devices |

---

## 10. RS485 ANEMOMETER SETUP

| Parameter | Default | Notes |
|-----------|---------|-------|
| Baud Rate | 9600 | Must match sensor |
| Data Bits | 8 | |
| Stop Bits | 1 | |
| Parity | None | |
| Slave ID | 1 | Verify with sensor manual |
| Register | 0x0000 | Wind speed register (verify) |

**Termination:** Install 120Ω resistor between A(+) and B(-) at the anemometer end if cable exceeds 10 meters.

**Wiring:** Shielded twisted pair. Drain wire to GND at ESP32 end only (prevents ground loops).

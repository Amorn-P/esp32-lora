# ESP32_Lora — NEXT STEPS (2026-06-07)

## 📦 Project Location
`C:\Users\SAMBA\Documents\PlatformIO\Projects\Project_Preperation\ESP32_Lora`

## 🟢 Current Status: Code Complete, Awaiting Wokwi + Hardware

---

## STEP 1: Wokwi Wiring (DAD — Visual Editor)

### Open in Wokwi
1. Go to https://wokwi.com
2. Create new ESP32 project
3. Replace `diagram.json` with ours (parts generated, wiring empty)
4. Upload `firmware.bin` and `wokwi.toml`

### Components to Wire

#### Master AA (esp-master)
| From | To | Wire |
|------|----|------|
| ESP32 21 (SDA) | rtc-master SDA | Blue |
| ESP32 22 (SCL) | rtc-master SCL | Yellow |
| rtc-master VCC | 3.3V | Red |
| rtc-master GND | GND | Black |
| ESP32 21 (SDA) | lcd-master SDA | Blue |
| ESP32 22 (SCL) | lcd-master SCL | Yellow |
| lcd-master VCC | 5V / VIN | Red |
| lcd-master GND | GND | Black |
| ESP32 36 | dip-master pin1 | — |
| ESP32 39 | dip-master pin2 | — |
| ESP32 34 | dip-master pin3 | — |
| ESP32 35 | dip-master pin4 | — |
| DIP common pins | GND | — |

#### Slave B3 (esp-b3) — Pump + Solar
| From | To | Wire |
|------|----|------|
| ESP32 21 (SDA) | rtc-b3 SDA | Blue |
| ESP32 22 (SCL) | rtc-b3 SCL | Yellow |
| rtc-b3 VCC | 3.3V | Red |
| rtc-b3 GND | GND | Black |
| ESP32 13 | led-b3-relay1 anode (via 220Ω) | — |
| led-b3-relay1 cathode | GND | — |
| ESP32 34 (ADC) | pot-b3-solar wiper (middle) | — |
| pot-b3-solar pin1 | 3.3V | — |
| pot-b3-solar pin3 | GND | — |
| ESP32 36 | dip-b3 pin1 | — |
| ESP32 39 | dip-b3 pin2 | — |
| ESP32 34 | dip-b3 pin3 | **⚠️ GPIO34 shared: use pin2 only** |
| ESP32 35 | dip-b3 pin4 | — |

#### Slave B4 (esp-b4) — Valves
| From | To | Wire |
|------|----|------|
| ESP32 21 (SDA) | rtc-b4 SDA | Blue |
| ESP32 22 (SCL) | rtc-b4 SCL | Yellow |
| rtc-b4 VCC | 3.3V | Red |
| rtc-b4 GND | GND | Black |
| ESP32 13 | led-b4-relay1 (via 220Ω) | — |
| ESP32 14 | led-b4-relay2 (via 220Ω) | — |
| ESP32 27 | led-b4-relay3 (via 220Ω) | — |
| ESP32 26 | led-b4-relay4 (via 220Ω) | — |
| ESP32 36 | dip-b4 pin1 | — |
| ESP32 39 | dip-b4 pin2 | — |
| ESP32 34 | dip-b4 pin3 | — |
| ESP32 35 | dip-b4 pin4 | — |

### ⚠️ Wokwi Rules
- DIP switches: ON = pin connected to GND (INPUT_PULLUP reads LOW)
  - B3 ID = 3 → DIP 1=ON, 2=ON, 3=OFF, 4=OFF (0011)
  - B4 ID = 4 → DIP 1=OFF, 2=ON, 3=OFF, 4=OFF (0100)
- GPIO34 shared: B3 dip bit2 + solar ADC — OK because B3 tests dip to set ID, then 34 becomes ADC
- Paste back wired connections to `diagram.json`, save in VS Code

---

## STEP 2: Build for Wokwi

### Add WOKWI flag to platformio.ini
```
[env:slave]
build_flags = 
    -D BOARD_TYPE=1
    -D WOKWI    ← ADD THIS LINE for Wokwi build
```

### Build
```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run -e slave
```

This produces `firmware.bin` with LoRa and INA226 stubbed (no hangs).

---

## STEP 3: Wokwi Simulation Test

### Serial Commands to Type in Wokwi Monitor

**B3 (pump+valve test):**
```
time 2026-06-07 09:00:00          ← Set RTC to 09:00
sched 09:00,5,14:00,5,10,0,0     ← R1=09:00/5min R2=14:00/5min off=10 (B1/B2)
status                            ← Check: Mode=1 (R1 active)
```
- B3's red LED should turn ON (pump in valve window)
- Turn solar pot to 0V → LED should turn OFF (solar gate)
- Wait 10s, press `time 2026-06-07 10:44:00` → LED should turn OFF (R1 ends ~10:44)

**B4 (valve sequencing test):**
```
time 2026-06-07 09:00:00
sched 09:00,5,14:00,5,0,0,0     ← Same schedule
```
- B4's 4 green LEDs should sequence: R1 on→off, R2 on→off, R3 on→off, R4 on→off
- Each should light for `duration` minutes with 1-min overlap

**Manual test (B4):**
```
manual 4,10                       ← Relay 4, 10 minutes
```
- B4 relay 4 LED should turn ON for 10 min

**Stop test:**
```
stop                              ← All LEDs OFF
sched 09:00,5,14:00,5,0,0,0     ← Resume (stop cleared on new schedule)
```

---

## STEP 4: Bench Flash (Physical Hardware)

### Hardware Needed
- 1× Master AA (ESP32 + LoRa SX1278 + DS3231 + LCD2004)
- 1× Slave B3 (ESP32 + LoRa SX1278 + DS3231 + INA226 + DS18B20 + relay)
- 1× Slave B4 (ESP32 + LoRa SX1278 + DS3231 + 4 relays)
- 433 MHz spring antennas (3x)
- 5V power supplies (3x)

### Flash Each Board
```powershell
# Remove -D WOKWI from build_flags first!

# Master
pio run -e master --target upload --upload-port COM3

# B3 (ID=3 on dipswitch)
pio run -e slave --target upload --upload-port COM4

# B4 (ID=4 on dipswitch)
pio run -e slave --target upload --upload-port COM5
```

### Bench Test Checklist
- [ ] Master LCD shows "Lucky-Lora AA" + time + IP
- [ ] Master WiFi connects → Blynk LEDs show online (V60)
- [ ] B3 sends heartbeat → V152 LED green in Blynk
- [ ] B4 sends heartbeat → V153 LED green
- [ ] Press V19 (SYNC ALL) → B3/B4 relay LEDs respond per schedule
- [ ] Press V137 (STOP ALL) → all relays OFF
- [ ] Press V136 (manual) → valve opens + B3 pump ON
- [ ] Cover solar panel (B3) → pump pauses

---

## STEP 5: Field Test (Farm)

### Minimum Setup
- Master AA at house (WiFi range)
- B3 + solar panel at farm (LoRa range test)
- 1 valve on B4 (verify water flows)

### Test
1. Power Master, wait for Blynk connection
2. Power B3 + B4 at farm location
3. Check V152/V153 = green (LoRa OK)
4. Set schedule via Blynk → press V19
5. Verify pump + valve activate at scheduled time
6. Test offline: unplug Master → verify B3 continues from NVM
7. Test emergency: V137 → all stop

### Success Criteria
- [ ] LoRa RSSI at farm distance (should be > -120 dBm)
- [ ] Solar gating works with actual panel
- [ ] Schedule runs autonomously after Master offline
- [ ] Telegram alerts received for events

---

## REFERENCE: File Map

```
ESP32_Lora/
├── platformio.ini          ← Build config (master + slave envs)
├── diagram.json            ← Wokwi components (DAD wires)
├── wokwi.toml              ← Wokwi project config
├── ESP32_Lora.md           ← Full technical spec
├── operation_manual.md     ← User operation manual
├── Dipswitch.md            ← Dipswitch pin reference
├── Blynk_tab.txt           ← Blynk pin layout text
├── BOM.txt                 ← Bill of materials
├── Requirement_Lora.txt    ← Original requirements
└── src/
    ├── main.cpp            ← Unified firmware entry
    ├── Hardware_Map.h      ← GPIO definitions
    ├── Hardware_Service.*  ← GPIO/Relay/Dipswitch
    ├── Time_Manager.*      ← DS3231 RTC wrapper
    ├── Lora_Protocol.h     ← LuckyPacket (19 bytes)
    ├── Lora_Service.*      ← SX1278 RadioLib engine
    ├── Scheduler.*         ← B1-B7 schedule logic
    ├── INA226_Sensor.*     ← Solar + DS18B20 + Fan
    ├── 00_Blynk_App.h      ← Blynk V-pin callbacks
    ├── 01_Social_Handlers.h ← Web handlers
    ├── 12_List_Wf.h        ← IotWebConf params
    ├── CommCore_Globals.cpp ← Extern definitions
    ├── User_config.h       ← LoRa/TLS/mute settings
    └── wokwi_stubs.h       ← Wokwi simulation stubs
```

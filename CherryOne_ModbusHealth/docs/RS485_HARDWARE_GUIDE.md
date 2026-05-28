================================================================================
RS485 HARDWARE GUIDE — CherryOne Commercial Farm Installation
================================================================================
Version 2.0 | Required reading before field deployment

================================================================================
1. WHY RELAYS TOGGLE BY THEMSELVES (Root Causes)
================================================================================

Two root causes were identified in your system:

A) SOFTWARE (FIXED in v2.0):
   The heartbeat ping wrote random values to Modbus register 0x00FF every 2s.
   Cheap Chinese relay boards don't bounds-check register addresses. When the
   random value happened to be 256 (the ON command), it triggered a relay.
   This is now replaced with a safe READ-based heartbeat.

B) HARDWARE (You must fix this):
   RS485 bus noise from motors, pumps, VFDs, and long cable runs induces
   false Modbus frames. The slave interprets noise as a valid write command
   and toggles a relay. Without proper termination and biasing, the bus
   is electrically floating and susceptible to ANY nearby EMI.

================================================================================
2. RS485 TERMINATION (MANDATORY)
================================================================================

RS485 requires two 120Ω resistors — one at EACH END of the bus.

   [ESP32 Master] ----[120Ω]---[A]--------[B]---[120Ω]----[Last Slave]
                                  |          |
                              [Slave 2]  [Slave 3]

Installation:
  - 120Ω, 1/4W resistor between A(+) and B(-) at the ESP32 end
  - 120Ω, 1/4W resistor between A(+) and B(-) at the LAST slave (furthest)
  - Do NOT install termination at intermediate slaves
  - This prevents signal reflections that corrupt Modbus frames

Some RS485 modules have built-in termination jumpers. Enable ONLY at the
two ends, disable at all intermediate nodes.

================================================================================
3. FAIL-SAFE BIASING (STRONGLY RECOMMENDED)
================================================================================

When no device is transmitting, the A/B lines float to an undefined state.
Noise can push them into "valid signal" territory, causing false triggers.

Add fail-safe bias at the MASTER end:

   5V ----[680Ω]---- A(+)
   GND ---[680Ω]---- B(-)

This pulls A high and B low when the bus is idle, creating a known "idle"
state that noise cannot override.

Some RS485-to-TTL modules (MAX485, SP3485) have built-in bias. Check your
module's datasheet. If not, add these two resistors to your perfboard.

================================================================================
4. CABLE TYPE (CRITICAL FOR FARM ENVIRONMENTS)
================================================================================

Use ONLY twisted-pair shielded cable for RS485:

  RECOMMENDED: Belden 3105A, 9841, or equivalent
  MINIMUM:     22-24 AWG twisted pair with foil shield
  DO NOT USE:  Individual jumper wires, untwisted cable, CAT5 for long runs

The twisted pair rejects common-mode noise. The shield drains induced EMI.
Without both, your bus acts as an antenna for every pump motor in the farm.

Shield connection:
  - Connect shield to GND at the ESP32 end ONLY
  - Leave shield floating at all slave ends
  - Do NOT connect shield at both ends (creates ground loop)

================================================================================
5. WIRING TOPOLOGY (MUST BE DAISY-CHAIN)
================================================================================

WRONG (Star):                    CORRECT (Daisy-chain):
                                  ESP32 --- Slave1 --- Slave2 --- Slave3
    ESP32 ---+--- Slave1                   [120Ω]               [120Ω]
             |
             +--- Slave2             Do NOT use star/stub topology.
             |                       Each stub creates signal reflections
             +--- Slave3             that corrupt Modbus frames.

Keep stub length < 30cm if you must branch. Prefer daisy-chain.

================================================================================
6. MAXIMUM CABLE LENGTH
================================================================================

At 9600 baud with proper cable:
  - Reliable:  up to 1200 meters (4000 ft)
  - With noisy motors: keep under 300 meters (1000 ft)
  - If longer is needed: add an RS485 repeater every 1000m

================================================================================
7. POWER SUPPLY (ISOLATION)
================================================================================

Farm environments have dirty power. Motors and VFDs inject noise into
the DC power rail, which couples into RS485 signals.

  RECOMMENDED:
  - Use a quality 5V/3A power supply with adequate filtering
  - Add a 1000µF electrolytic + 0.1µF ceramic capacitor at the ESP32 Vin
  - Power the relay boards from a SEPARATE power supply if possible
  - Use a DC-DC isolated module if sharing power with heavy loads

  MINIMUM:
  - 1000µF capacitor at ESP32 Vin pin
  - TVS diode (5V) across Vin-GND for surge protection

================================================================================
8. RS485 TRANSCEIVER CHECK
================================================================================

Verify your ESP32's RS485 module has proper DE/RE pin control:
  - DE (Driver Enable) must be HIGH during transmit, LOW otherwise
  - RE (Receiver Enable) must be LOW during receive, HIGH during transmit
  - If both are floating, the transceiver is in an undefined state

The DFRobot_RTU library should handle this, but verify:
  - Find which GPIO controls DE/RE on your RS485 module
  - Check the library's pin configuration
  - If your module auto-negotiates (some do), verify it triggers correctly

================================================================================
9. FIELD DEPLOYMENT CHECKLIST
================================================================================

Before powering on in the farm, verify:

[ ] 120Ω termination at BOTH ends of the RS485 bus
[ ] 680Ω fail-safe bias at master end (A→5V, B→GND)
[ ] Cable is twisted-pair shielded, shield grounded at ESP32 only
[ ] Daisy-chain topology (no stars, no long stubs)
[ ] All slave dipswitch IDs are unique (1-5)
[ ] All slave baud rates match RS485_BAUD (9600)
[ ] Power supply has filtering capacitor
[ ] RS485 A/B wires are NOT swapped anywhere
[ ] ESP32 is in a dry, ventilated enclosure (farm humidity kills electronics)

================================================================================
10. TROUBLESHOOTING NOISY BUS
================================================================================

Symptom: Relays still toggle after v2.0 firmware upgrade

Step 1: Remove ALL slaves except Slave 1 (the pump board)
        -> If problem stops: noise is coming from a downstream slave

Step 2: Check termination resistors with multimeter
        -> Measure between A and B at the ends: should be ~60Ω (two 120Ω in parallel)

Step 3: Add a 1kΩ pull-up from A to 3.3V and 1kΩ pull-down from B to GND
        at the ESP32 directly. This provides strong bias.

Step 4: Shield the RS485 cable with metal conduit (EMT) if near large motors

Step 5: Add opto-isolated RS485 repeaters between long cable segments

Step 6: Increase Modbus timeout from the library default if slaves respond slowly

================================================================================
DOCUMENT VERSION: 2.0
Last updated: 2026-05-28
================================================================================

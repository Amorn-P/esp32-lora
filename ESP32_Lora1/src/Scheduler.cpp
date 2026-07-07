/**
 * Scheduler.cpp - Schedule Logic Implementation
 *
 * Board mapping:
 *   B1 (ID1): Independent pump, 1 relay. 50min ON / 10min OFF, 09:00-17:00.
 *   B2 (ID2): Independent pump, 1 relay. 50min ON / 10min OFF, Master-start to 13:50.
 *             Solar check via INA226 — pauses pump when current low.
 *   B3 (ID3): Group pump, 1 relay. ON during Round 1 & Round 2 valve windows.
 *             Solar check via INA226 — pauses pump when current low.
 *   B4 (ID4): Sequential valves, 4 relays (global 1-4)
 *   B5 (ID5): Sequential valves, 6 relays (global 5-10)
 *   B6 (ID6): Sequential valves, 4 relays (global 11-14)
 *   B7 (ID7): Sequential valves, 6 relays (global 15-20)
 *
 * Staggered overlap: 1 minute. relayStart = periodStart + i*(duration-1).
 * Round 1 and Round 2 have independent durations (duration1, duration2).
 */

#include "Scheduler.h"

#if BOARD_TYPE != 0
#include "INA226_Sensor.h"
#endif

uint8_t         Scheduler::boardID = 0;
Schedule        Scheduler::sched1 = {0,0,0};
Schedule        Scheduler::sched2 = {0,0,0};
uint8_t         Scheduler::currentMode = 0;
uint8_t         Scheduler::activeGlobalRelay = 0;
bool            Scheduler::isManualActive = false;
uint8_t         Scheduler::manualTargetRelay = 0;
unsigned long   Scheduler::manualEndTime = 0;
unsigned long   Scheduler::pumpCycleStartMs = 0;
bool            Scheduler::pumpIsOn = false;
bool            Scheduler::stopped = false;

static Preferences schedPrefs;

// Global relay offset per board (relays BEFORE this board in B4-B7 group)
static const uint8_t relayOffset[8] = {0, 0, 0, 0, 0, 4, 10, 14};
// Relay count per board
static const uint8_t relayCount[8]  = {0, 1, 1, 1, 4,  6,  4,  6};

// ============================================================
// INIT
// ============================================================
void Scheduler::init(uint8_t id) {
    boardID = id;
    loadFromNVM();

#if BOARD_TYPE != 0
    // INA226 only on B2 and B3 (pump boards with solar)
    if (boardID == 2 || boardID == 3) {
        INA226Sensor::init();
    }
#endif
}

// ============================================================
// RELAY MAPPING HELPERS
// ============================================================
int Scheduler::globalToLocal(uint8_t globalRelay) {
    if (boardID < 1 || boardID > 7) return -1;
    uint8_t offset = relayOffset[boardID];
    uint8_t count  = relayCount[boardID];
    if (globalRelay > offset && globalRelay <= offset + count) {
        return globalRelay - offset - 1; // 0-indexed local
    }
    return -1;
}

int Scheduler::getMaxRelays() {
    if (boardID < 1 || boardID > 7) return 0;
    return relayCount[boardID];
}

// ============================================================
// VALVE WINDOW CHECK (used by B3 and B4-B7)
// Returns true if ANY global relay 1-20 is active at currMins.
// Used by B3 to decide pump state.
// ============================================================
static bool valveActive[21] = {false}; // index 1-20, ignore 0

static void computeValveWindow(uint16_t currMins, uint16_t periodStart, uint8_t duration, bool clear) {
    if (duration == 0) return;
    uint8_t overlap = 1;
    for (uint8_t i = 0; i < 20; i++) {
        uint16_t rStart = periodStart + i * (duration > overlap ? duration - overlap : 1);
        uint16_t rEnd   = rStart + duration;
        if (currMins >= rStart && currMins < rEnd) {
            valveActive[i + 1] = true; // global relay 1-20
        }
    }
}

bool Scheduler::isInValveWindow(uint16_t currMins) {
    // Clear valve array
    for (int i = 1; i <= 20; i++) valveActive[i] = false;

    // Round 1
    uint16_t p1Start = sched1.startHr * 60 + sched1.startMin;
    computeValveWindow(currMins, p1Start, sched1.duration, false);

    // Round 2
    uint16_t p2Start = sched2.startHr * 60 + sched2.startMin;
    computeValveWindow(currMins, p2Start, sched2.duration, false);

    // Check if any relay active
    for (int i = 1; i <= 20; i++) {
        if (valveActive[i]) return true;
    }
    return false;
}

// ============================================================
// B1 PUMP LOGIC (50/10, 09:00-17:00)
// ============================================================
bool Scheduler::isB1Active(uint16_t currMins) {
    // Outside operating window
    if (currMins < B1_START_MIN || currMins >= B1_END_MIN) {
        pumpIsOn = false;
        return false;
    }

    // Inside window: 50 ON / 10 OFF cycle
    // Cycle = 60 minutes. Position in cycle = minutes since 09:00 % 60
    uint16_t elapsed   = currMins - B1_START_MIN;
    uint8_t  cyclePos  = elapsed % (PUMP_ON_MIN + PUMP_OFF_MIN); // % 60

    return (cyclePos < PUMP_ON_MIN);
}

// ============================================================
// B2 PUMP LOGIC (50/10, Master-start to 13:50, solar check)
// ============================================================
bool Scheduler::isB2Active(uint16_t currMins) {
    uint16_t b2Start = sched1.startHr * 60 + sched1.startMin;

    // Outside operating window
    if (currMins < b2Start || currMins >= B2_END_MIN) {
        pumpIsOn = false;
        return false;
    }

#if BOARD_TYPE != 0
    // Solar check: pause pump if sunlight low
    if (!INA226Sensor::isSolarOK()) {
        pumpIsOn = false;
        return false;
    }
#endif

    // Inside window: 50 ON / 10 OFF cycle
    uint16_t elapsed  = currMins - b2Start;
    uint8_t  cyclePos = elapsed % (PUMP_ON_MIN + PUMP_OFF_MIN);

    return (cyclePos < PUMP_ON_MIN);
}

// ============================================================
// RUN — Main scheduler tick (called every 1s from loop)
// ============================================================
void Scheduler::run() {
    if (boardID == 0) return; // Master has no scheduler

    // Emergency stop check — persistent, cleared only by new schedule
    if (stopped) return;

    DateTime now = TimeManager::now();
    uint16_t currMins = now.hour() * 60 + now.minute();
    bool shouldBeOn[TOTAL_RELAYS] = {false};
    currentMode = 0;

    // ---- Solar sensor update (B2, B3 only) ----
#if BOARD_TYPE != 0
    if (boardID == 2 || boardID == 3) {
        INA226Sensor::update();
    }
#endif

    // ==========================================================
    // MANUAL MODE (overrides schedule)
    // ==========================================================
    if (isManualActive) {
        if (millis() > manualEndTime) {
            isManualActive = false;
        } else {
            currentMode = 3;
            if (boardID == 3) {
                // B3 pump: on for any manual command
                shouldBeOn[0] = true;
            } else if (boardID >= 4 && boardID <= 7) {
                int localIdx = globalToLocal(manualTargetRelay);
                if (localIdx >= 0) shouldBeOn[localIdx] = true;
            }
            // B1/B2 not targeted by manual (relay IDs 1-20 are B4-B7)
        }
    } else {
        // ==========================================================
        // AUTONOMOUS SCHEDULE
        // ==========================================================
        switch (boardID) {

        case 1: // ---- B1: Independent pump ----
            if (isB1Active(currMins)) {
                shouldBeOn[0] = true;
                currentMode = 1;
            }
            break;

        case 2: // ---- B2: Independent pump with solar ----
            if (isB2Active(currMins)) {
                shouldBeOn[0] = true;
                currentMode = 1;
            }
            break;

        case 3: { // ---- B3: Group pump (solar check) ----
            // B3 runs during Round 1 or Round 2 valve windows
            if (isInValveWindow(currMins)) {
#if BOARD_TYPE != 0
                if (INA226Sensor::isSolarOK()) {
                    shouldBeOn[0] = true;
                    currentMode = 1;
                }
#else
                shouldBeOn[0] = true;
                currentMode = 1;
#endif
            }
            break;
        }

        case 4: case 5: case 6: case 7: { // ---- B4-B7: Sequential valves ----
            // Round 1
            if (sched1.duration > 0) {
                uint16_t p1Start = sched1.startHr * 60 + sched1.startMin;
                uint8_t overlap = 1;
                for (uint8_t i = 0; i < 20; i++) {
                    uint16_t rStart = p1Start + i * (sched1.duration > overlap ? sched1.duration - overlap : 1);
                    uint16_t rEnd   = rStart + sched1.duration;
                    if (currMins >= rStart && currMins < rEnd) {
                        int localIdx = globalToLocal(i + 1);
                        if (localIdx >= 0) {
                            shouldBeOn[localIdx] = true;
                            currentMode = 1;
                            activeGlobalRelay = i + 1;
                        }
                    }
                }
            }

            // Round 2
            if (sched2.duration > 0) {
                uint16_t p2Start = sched2.startHr * 60 + sched2.startMin;
                uint8_t overlap = 1;
                for (uint8_t i = 0; i < 20; i++) {
                    uint16_t rStart = p2Start + i * (sched2.duration > overlap ? sched2.duration - overlap : 1);
                    uint16_t rEnd   = rStart + sched2.duration;
                    if (currMins >= rStart && currMins < rEnd) {
                        int localIdx = globalToLocal(i + 1);
                        if (localIdx >= 0) {
                            shouldBeOn[localIdx] = true;
                            if (currentMode == 0) currentMode = 2;
                            activeGlobalRelay = i + 1;
                        }
                    }
                }
            }
            break;
        }

        } // switch boardID
    }

    // ==========================================================
    // APPLY RELAYS
    // ==========================================================
    for (int i = 0; i < TOTAL_RELAYS; i++) {
        HardwareService::setRelay(i, shouldBeOn[i]);
    }
}

// ============================================================
// SCHEDULE MANAGEMENT
// ============================================================
void Scheduler::setSchedule(Schedule s1, Schedule s2) {
    sched1 = s1;
    sched2 = s2;
    stopped = false;    // Resume on new schedule
    saveToNVM();
}

void Scheduler::setManual(uint8_t globalRelay, uint16_t durationMin) {
    manualTargetRelay = globalRelay;
    manualEndTime = millis() + (durationMin * 60000UL);
    isManualActive = (durationMin > 0);
}

void Scheduler::stopAll() {
    stopped = true;
    isManualActive = false;
    HardwareService::allRelaysOff();
    currentMode = 0;
}

void Scheduler::resume() {
    stopped = false;
    Serial.println("[Scheduler] Resumed");
}

void Scheduler::loadFromNVM() {
    schedPrefs.begin("lucky", true);
    sched1.startHr  = schedPrefs.getUChar("h1", 0);
    sched1.startMin = schedPrefs.getUChar("m1", 0);
    sched1.duration = schedPrefs.getUChar("d1", 0);
    sched2.startHr  = schedPrefs.getUChar("h2", 0);
    sched2.startMin = schedPrefs.getUChar("m2", 0);
    sched2.duration = schedPrefs.getUChar("d2", 0);
    schedPrefs.end();
}

void Scheduler::saveToNVM() {
    schedPrefs.begin("lucky", false);
    schedPrefs.putUChar("h1", sched1.startHr);
    schedPrefs.putUChar("m1", sched1.startMin);
    schedPrefs.putUChar("d1", sched1.duration);
    schedPrefs.putUChar("h2", sched2.startHr);
    schedPrefs.putUChar("m2", sched2.startMin);
    schedPrefs.putUChar("d2", sched2.duration);
    schedPrefs.end();
}

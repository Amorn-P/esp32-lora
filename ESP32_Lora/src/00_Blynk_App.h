/**
 * 00_Blynk_App.h - Blynk Virtual Pins for ESP32_Lora (Master Only)
 *
 * Pin Layout:
 *   V19   : SYNC ALL SLAVES
 *   V60   : Master LED   V61-V62  : B1/B2 Pump   V63: B3 Pump
 *   V66-V69 : B4-R1~R4   V70-V75  : B5-R1~R6
 *   V76-V79 : B6-R1~R4   V81-V86  : B7-R1~R6
 *   V130  : B3-B7 P1 Start        V131  : B3-B7 P1 Duration
 *   V132  : B3-B7 P2 Start        V133  : B3-B7 P2 Duration
 *   V134  : Manual Relay ID       V135  : Manual Duration
 *   V136  : Manual START          V137  : STOP ALL
 *   V140-V147 : B1/B2 (future)
 *
 * CommCore reserves: V90-V101, V118-V127
 */
#if BOARD_TYPE == 0

#ifndef BLYNK_APP_H
#define BLYNK_APP_H

#include "CommCore.h"
#include "Lora_Protocol.h"
#include "Lora_Service.h"
#include "Scheduler.h"

extern void broadcastSchedule();
extern void broadcastStop();
extern void broadcastManual(uint8_t relayID, uint16_t durationMin);
extern uint8_t relayToBoard(uint8_t globalRelay);

// Global Blynk schedule values (populated before broadcast)
uint8_t  blynkP1StartHr = 9, blynkP1StartMin = 0;
uint8_t  blynkP2StartHr = 14, blynkP2StartMin = 0;
uint16_t blynkP1Dur = 5, blynkP2Dur = 5;

// Manual mode buffer
uint8_t  manualRelayID = 0;
uint16_t manualDuration = 0;

// ============================================================
// B3-B7 GROUP SCHEDULE — P1 (V130, V131)
// ============================================================
BLYNK_WRITE(V130) {
    TimeInputParam t(param);
    if (t.getStartHour() >= 0)   blynkP1StartHr = t.getStartHour();
    if (t.getStartMinute() >= 0) blynkP1StartMin = t.getStartMinute();
}

BLYNK_WRITE(V131) {
    blynkP1Dur = param.asInt();
}

// ============================================================
// B3-B7 GROUP SCHEDULE — P2 (V132, V133)
// ============================================================
BLYNK_WRITE(V132) {
    TimeInputParam t(param);
    if (t.getStartHour() >= 0)   blynkP2StartHr = t.getStartHour();
    if (t.getStartMinute() >= 0) blynkP2StartMin = t.getStartMinute();
}

BLYNK_WRITE(V133) {
    blynkP2Dur = param.asInt();
}

// ============================================================
// SYNC ALL SLAVES (V19)
// ============================================================
BLYNK_WRITE(V19) {
    if (param.asInt()) {
        broadcastSchedule();
    }
}

// ============================================================
// STOP ALL (V137)
// ============================================================
BLYNK_WRITE(V137) {
    if (param.asInt()) {
        broadcastStop();
    }
}

// ============================================================
// MANUAL MODE (V134, V135, V136)
// ============================================================
BLYNK_WRITE(V134) {
    manualRelayID = param.asInt();
}

BLYNK_WRITE(V135) {
    manualDuration = param.asInt();
}

BLYNK_WRITE(V136) {
    if (param.asInt() && manualRelayID > 0 && manualRelayID <= 20) {
        broadcastManual(manualRelayID, manualDuration);
    }
}

// ============================================================
// B1 & B2 Independent (V140-V147) — future use
// ============================================================
BLYNK_WRITE(V140) {} // B1 P1 Start
BLYNK_WRITE(V141) {} // B1 P1 Duration
BLYNK_WRITE(V142) {} // B1 P2 Start
BLYNK_WRITE(V143) {} // B1 P2 Duration
BLYNK_WRITE(V144) {} // B2 P1 Start
BLYNK_WRITE(V145) {} // B2 P1 Duration
BLYNK_WRITE(V146) {} // B2 P2 Start
BLYNK_WRITE(V147) {} // B2 P2 Duration

#endif // BLYNK_APP_H
#endif // BOARD_TYPE == 0

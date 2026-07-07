/**
 * Scheduler.h - Schedule Logic for Slaves (B1-B7)
 * 
 * B1: Independent pump, 50min ON / 10min OFF, 09:00-17:00
 * B2: Independent pump, 50min ON / 10min OFF, Master-set start to 13:50, solar check
 * B3: Group pump, ON during Round 1 & Round 2 valve windows, solar check
 * B4-B7: Sequential valves, 1-min overlap, 20 global relays, separate R1/R2 duration
 *
 * Packet fields per target:
 *   B1 (targetId=1): startHr1/startMin1=9:00, duration1=50(ON), startHr2=10(OFF)
 *   B2 (targetId=2): startHr1/startMin1=computed start, duration1=50(ON), startHr2=10(OFF)
 *   B3-B7 (targetId=255): startHr1/duration1=Round1, startHr2/duration2=Round2
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <Arduino.h>
#include <Preferences.h>
#include "Time_Manager.h"
#include "Hardware_Service.h"

// ON/OFF cycle (minutes) for B1 and B2 pumps
#define PUMP_ON_MIN     50
#define PUMP_OFF_MIN    10

// B1 hard stop (minutes from midnight)
#define B1_START_MIN    (9 * 60)      // 09:00
#define B1_END_MIN      (17 * 60)     // 17:00

// B2 hard stop: 10 min before Round 2 start (default 14:00)
#define B2_END_MIN      (13 * 60 + 50) // 13:50

struct Schedule {
    uint8_t startHr;
    uint8_t startMin;
    uint8_t duration;  // Minutes
};

class Scheduler {
public:
    static void init(uint8_t boardID);
    static void run();
    static void loadFromNVM();
    static void saveToNVM();
    static void setSchedule(Schedule s1, Schedule s2);
    static void setManual(uint8_t globalRelay, uint16_t durationMin);
    static void stopAll();
    static void resume();           // Clear stop flag
    static bool isStopped() { return stopped; }

    // Accessors
    static Schedule getSched1() { return sched1; }
    static Schedule getSched2() { return sched2; }
    static uint8_t getCurrentMode() { return currentMode; }
    static uint8_t getActiveRelay() { return activeGlobalRelay; }

private:
    static uint8_t boardID;
    static Schedule sched1, sched2;
    static uint8_t currentMode;    // 0=Idle, 1=P1, 2=P2, 3=Manual
    static uint8_t activeGlobalRelay;

    // Manual mode
    static bool isManualActive;
    static uint8_t manualTargetRelay;
    static unsigned long manualEndTime;

    // Emergency stop
    static bool stopped;            // Persistent stop (CMD_STOP_ALL)

    // B1/B2 pump cycle tracking
    static unsigned long pumpCycleStartMs;  // when current ON/OFF cycle started
    static bool pumpIsOn;                   // true during ON phase

    // Helpers
    static int  globalToLocal(uint8_t globalRelay);
    static int  getMaxRelays();
    static bool isInValveWindow(uint16_t currMins);  // B3: any relay active?
    static bool isB1Active(uint16_t currMins);
    static bool isB2Active(uint16_t currMins);
};

#endif

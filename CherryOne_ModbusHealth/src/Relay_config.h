#ifndef RELAY_CONFIG_H
#define RELAY_CONFIG_H

#include <Arduino.h>
#include "Board_ID.h"

struct ValveMapping {
    uint8_t slaveId;
    uint8_t relayIndex; // 0-based index
};

/**
 * SYSTEM_VALVE_MAP
 * Maps logical ID (1-21) to physical Modbus Slave and Relay Index.
 * Index 0 is a placeholder (Pump is ID 1).
 */

/**
 * Relay Mapping Configuration
 * Simply set the number of Active relays (starting from Relay 0).
 * Any relay index above this number is automatically considered a Spare.
 */

// Board 0: Only Relay 0 is Active (Pump)
#define B0_ACTIVE_COUNT 1

// Board 1: Currently 4 Active (Relays 0-3)
#define B1_ACTIVE_COUNT 4

// Board 2, 3, 4 Configuration
#define B2_ACTIVE_COUNT 6  // Relays 0-5
#define B3_ACTIVE_COUNT 4  // Relays 0-3
#define B4_ACTIVE_COUNT 6  // Relays 0-5

const ValveMapping SYSTEM_VALVE_MAP[] = {
    {0, 0}, // Index 0 (Dummy)
    {BOARD_0_PUMP_ID, 0}, // ID 1 (Pump)
    
    // Board 1 (Zones 2-5)
    {BOARD_1_ZONE_A_ID, 0}, {BOARD_1_ZONE_A_ID, 1}, {BOARD_1_ZONE_A_ID, 2}, {BOARD_1_ZONE_A_ID, 3},
    
    // Board 2 (Zones 6-11)
    {BOARD_2_ZONE_B_ID, 0}, {BOARD_2_ZONE_B_ID, 1}, {BOARD_2_ZONE_B_ID, 2}, 
    {BOARD_2_ZONE_B_ID, 3}, {BOARD_2_ZONE_B_ID, 4}, {BOARD_2_ZONE_B_ID, 5},
    
    // Board 3 (Zones 12-15)
    {BOARD_3_ZONE_C_ID, 0}, {BOARD_3_ZONE_C_ID, 1}, {BOARD_3_ZONE_C_ID, 2}, {BOARD_3_ZONE_C_ID, 3},
    
    // Board 4 (Zones 16-21)
    {BOARD_4_ZONE_D_ID, 0}, {BOARD_4_ZONE_D_ID, 1}, {BOARD_4_ZONE_D_ID, 2}, 
    {BOARD_4_ZONE_D_ID, 3}, {BOARD_4_ZONE_D_ID, 4}, {BOARD_4_ZONE_D_ID, 5}
};

#endif
/**
 * Lora_Protocol.h - Lucky_Lora Packet Definition
 * Fixed-size 19-byte packet for Star Topology
 */

#ifndef LORA_PROTOCOL_H
#define LORA_PROTOCOL_H

#include <Arduino.h>

// Command Types
#define CMD_HEARTBEAT        0
#define CMD_MANUAL           1
#define CMD_STOP_ALL         2
#define CMD_TIME_SYNC        3
#define CMD_SET_SCHEDULE     4
#define CMD_ACK              5   // Slave → Master: schedule confirmed

struct LuckyPacket {
    uint8_t senderId;      // 0=Master, 1-7=Slave
    uint8_t targetId;      // 0=Master, 1-7=Specific, 255=Broadcast
    uint8_t cmdType;       // See CMD_* above

    // Status (Slave -> Master)
    uint8_t relayStatus;   // Bitmask for 7 relays
    uint8_t currentMode;   // 0=Idle, 1=Period1, 2=Period2, 3=Manual
    uint8_t battery;       // Reserved

    // Command (Master -> Slave)
    uint8_t activeRelay;   // Global Relay ID (1-20)

    // Schedule Data (cmdType == CMD_SET_SCHEDULE)
    uint8_t startHr1;
    uint8_t startMin1;
    uint8_t duration1;     // Minutes
    uint8_t startHr2;
    uint8_t startMin2;
    uint8_t duration2;     // Minutes

    uint8_t reserved[2];   // Future use
    uint16_t checksum;
} __attribute__((packed));

#endif // LORA_PROTOCOL_H

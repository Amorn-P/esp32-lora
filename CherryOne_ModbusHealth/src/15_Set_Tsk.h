// ==================================================
// FREERTOS TASK HANDLES DECLARATION
// ==================================================

#ifndef _15_SET_TSK_H
#define _15_SET_TSK_H

// Task1: IoT Web Configuration Handler (Core 0)
TaskHandle_t Task1;

// Task2: Date-Time NTP Server (Core 0)
TaskHandle_t Task2;

// Task3: ... (Core 1)
TaskHandle_t Task3;

// Task4: Blynk Communication Handler (Core 1)
TaskHandle_t Task4;

// Task5: HTTP Data Sender (Core 1)
TaskHandle_t Task5;

// Task6: ... (Core 1)
TaskHandle_t Task6;

// Task8: Software Watchdog Timer & Stack Monitor (Core 0)
TaskHandle_t Task8;

#endif // _15_SET_TSK_H

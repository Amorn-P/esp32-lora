#ifndef ASTRO_TASK_H
#define ASTRO_TASK_H

#include <Arduino.h>

/**
 * @brief FreeRTOS task that reads RTC time and computes solar position
 *        (target elevation and azimuth) using astronomical algorithms.
 *        Updates sysState.target_elevation and sysState.target_azimuth under mutex.
 * @param pvParameters Standard FreeRTOS parameter pointer.
 */
void astro_task(void *pvParameters);

#endif // ASTRO_TASK_H

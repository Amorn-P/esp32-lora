#ifndef SAFETY_TASK_H
#define SAFETY_TASK_H

#include <Arduino.h>

/**
 * @brief Initialize the RS485 interface (Serial2) for Modbus wind sensor communication.
 */
void init_wind_sensor();

/**
 * @brief FreeRTOS task that polls the Modbus wind speed sensor via RS485.
 *        Sets sysState.wind_alarm if wind exceeds STOW_WIND_SPEED_KPH.
 *        Sets sysState.wind_sensor_fault if sensor is unresponsive.
 * @param pvParameters Standard FreeRTOS parameter pointer.
 */
void safety_task(void *pvParameters);

#endif // SAFETY_TASK_H

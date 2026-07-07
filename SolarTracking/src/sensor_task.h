#ifndef SENSOR_TASK_H
#define SENSOR_TASK_H

#include <Arduino.h>

/**
 * @brief Initialize the I2C bus and the sensors (MPU6050, QMC5883L, DS3231).
 * @return true if all critical sensors initialized successfully.
 */
bool init_sensors();

/**
 * @brief FreeRTOS task that continuously reads MPU6050 (tilt), QMC5883L (heading),
 *        and DS3231 (time). It updates global/shared variables for the tracking task.
 * @param pvParameters Standard FreeRTOS parameter pointer.
 */
void sensor_task(void *pvParameters);

#endif // SENSOR_TASK_H
#ifndef MOTOR_TASK_H
#define MOTOR_TASK_H

#include <Arduino.h>

/**
 * @brief Configure LEDC PWM channels and attach to motor driver pins.
 */
void init_motor_drivers();

/**
 * @brief Immediately stop all motor PWM output.
 */
void stop_all_motors();

/**
 * @brief FreeRTOS task that implements the motor control loop.
 *        Priority: 1) Wind stow, 2) Track sun position with dead-band.
 * @param pvParameters Standard FreeRTOS parameter pointer.
 */
void motor_task(void *pvParameters);

#endif // MOTOR_TASK_H

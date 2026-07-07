#ifndef UI_TASK_H
#define UI_TASK_H

#include <Arduino.h>

/**
 * @brief FreeRTOS task that refreshes the LCD2004 display with:
 *        Target Az/El, Current Az/El, Wind Speed, System Mode.
 * @param pvParameters Standard FreeRTOS parameter pointer.
 */
void ui_task(void *pvParameters);

#endif // UI_TASK_H

#ifndef __BUTTON_H
#define __BUTTON_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Button event types
 */
typedef enum
{
    BUTTON_EVENT_NONE = 0,
    BUTTON_EVENT_SINGLE_CLICK,
    BUTTON_EVENT_DOUBLE_CLICK,
    BUTTON_EVENT_LONG_PRESS,
} ButtonEvent_t;

/**
 * @brief Initialize the button hardware and internal state.
 *
 * This function configures PA3 as input with pull-up and resets internal state.
 * Call it once after HAL initialization.
 */
void Button_Init(void);

/**
 * @brief Poll the button state and update internal debouncing / click state machine.
 *
 * This should be called periodically (e.g. every 5..20 ms) from the main loop
 * or a timer callback.
 */
void Button_Task(void);

/**
 * @brief Retrieve the latest button event.
 *
 * If there are multiple events, only the first one is returned and cleared.
 *
 * @return ButtonEvent_t one of BUTTON_EVENT_* values.
 */
ButtonEvent_t Button_GetEvent(void);

#ifdef __cplusplus
}
#endif

#endif // __BUTTON_H

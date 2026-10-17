#include "button-task/button-task.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "platform/platform.h"
#include "systime/systime.h"

#define BUTTON_DEBOUNCE_MS 20
#define BUTTON_DEBOUNCE_US (BUTTON_DEBOUNCE_MS * 1000)

typedef enum
{
    BUTTON_STATE_RELEASED,
    BUTTON_STATE_PRESSING,
    BUTTON_STATE_PRESSED,
    BUTTON_STATE_RELEASING,
} button_state_t;

static button_state_t button_state = BUTTON_STATE_RELEASED;
static uint64_t transition_us = 0;
static uint32_t press_count = 0;
static button_task_callback_t button_on_press = NULL;

void button_task_init(button_task_callback_t on_press)
{
    button_state = BUTTON_STATE_RELEASED;
    transition_us = systime_us();
    press_count = 0;
    button_on_press = on_press;
}

void button_task_handle(void)
{
    bool pressed = platform_button_read();
    uint64_t now_us = systime_us();

    switch (button_state)
    {
        case BUTTON_STATE_RELEASED:
            if (pressed)
            {
                transition_us = now_us;
                button_state = BUTTON_STATE_PRESSING;
            }
            break;

        case BUTTON_STATE_PRESSING:
            if (!pressed)
            {
                button_state = BUTTON_STATE_RELEASED;
            }
            else if (now_us - transition_us >= BUTTON_DEBOUNCE_US)
            {
                button_state = BUTTON_STATE_PRESSED;
                press_count++;

                if (button_on_press != NULL)
                {
                    button_on_press();
                }
            }
            break;

        case BUTTON_STATE_PRESSED:
            if (!pressed)
            {
                transition_us = now_us;
                button_state = BUTTON_STATE_RELEASING;
            }
            break;

        case BUTTON_STATE_RELEASING:
            if (pressed)
            {
                button_state = BUTTON_STATE_PRESSED;
            }
            else if (now_us - transition_us >= BUTTON_DEBOUNCE_US)
            {
                button_state = BUTTON_STATE_RELEASED;
            }
            break;
    }
}

bool button_task_is_pressed(void)
{
    return button_state == BUTTON_STATE_PRESSED ||
           button_state == BUTTON_STATE_RELEASING;
}

uint32_t button_task_get_press_count(void)
{
    return press_count;
}

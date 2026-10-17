#include "button-task/button-task.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "platform/platform.h"
#include "systime/systime.h"

#define BUTTON_DEBOUNCE_US (20 * 1000)
#define BUTTON_LONG_PRESS_US 1000000

typedef enum
{
    BUTTON_STATE_RELEASED,
    BUTTON_STATE_PRESSING,
    BUTTON_STATE_PRESSED,
    BUTTON_STATE_RELEASING,
} button_state_t;

typedef struct
{
    button_state_t state;

    uint64_t transition_us;
    uint64_t pressed_us;

    uint32_t press_count;

    bool long_press_reported;

    button_task_callback_t on_press;
    button_task_callback_t on_long_press;
} button_task_data_t;

static button_task_data_t buttons[PLATFORM_BUTTON_COUNT];

static bool button_valid(platform_button_t button)
{
    return button == PLATFORM_BUTTON_LED ||
           button == PLATFORM_BUTTON_PERIOD;
}

static void button_handle_one(
    platform_button_t button,
    uint64_t now_us)
{
    button_task_data_t *data = &buttons[button];
    bool pressed = platform_button_read(button);

    switch (data->state)
    {
        case BUTTON_STATE_RELEASED:
            if (pressed)
            {
                data->transition_us = now_us;
                data->state = BUTTON_STATE_PRESSING;
            }
            break;

        case BUTTON_STATE_PRESSING:
            if (!pressed)
            {
                data->state = BUTTON_STATE_RELEASED;
            }
            else if (now_us - data->transition_us >=
                     BUTTON_DEBOUNCE_US)
            {
                data->state = BUTTON_STATE_PRESSED;
                data->pressed_us = now_us;
                data->long_press_reported = false;
                data->press_count++;

                /*
                 * Если долгого действия у кнопки нет,
                 * короткое нажатие можно сообщить сразу.
                 */
                if (data->on_long_press == NULL &&
                    data->on_press != NULL)
                {
                    data->on_press();
                }
            }
            break;

        case BUTTON_STATE_PRESSED:
            if (!pressed)
            {
                data->transition_us = now_us;
                data->state = BUTTON_STATE_RELEASING;
            }
            else if (data->on_long_press != NULL &&
                     !data->long_press_reported &&
                     now_us - data->pressed_us >=
                         BUTTON_LONG_PRESS_US)
            {
                /*
                 * Долгое нажатие сообщаем ровно один раз,
                 * даже если при отпускании будет дребезг.
                 */
                data->long_press_reported = true;
                data->on_long_press();
            }
            break;

        case BUTTON_STATE_RELEASING:
            if (pressed)
            {
                data->state = BUTTON_STATE_PRESSED;
            }
            else if (now_us - data->transition_us >=
                     BUTTON_DEBOUNCE_US)
            {
                data->state = BUTTON_STATE_RELEASED;

                /*
                 * Для кнопки с долгим действием короткое
                 * нажатие становится известно только
                 * после подтверждённого отпускания.
                 */
                if (data->on_long_press != NULL &&
                    !data->long_press_reported &&
                    data->on_press != NULL)
                {
                    data->on_press();
                }
            }
            break;
    }
}

void button_task_init(
    platform_button_t button,
    button_task_callback_t on_press,
    button_task_callback_t on_long_press)
{
    if (!button_valid(button))
    {
        return;
    }

    buttons[button].state = BUTTON_STATE_RELEASED;
    buttons[button].transition_us = systime_us();
    buttons[button].pressed_us = 0;
    buttons[button].press_count = 0;
    buttons[button].long_press_reported = false;
    buttons[button].on_press = on_press;
    buttons[button].on_long_press = on_long_press;
}

void button_task_handle(void)
{
    uint64_t now_us = systime_us();

    for (uint32_t i = 0;
         i < PLATFORM_BUTTON_COUNT;
         i++)
    {
        button_handle_one(
            (platform_button_t)i,
            now_us);
    }
}

bool button_task_is_pressed(platform_button_t button)
{
    if (!button_valid(button))
    {
        return false;
    }

    return buttons[button].state == BUTTON_STATE_PRESSED ||
           buttons[button].state == BUTTON_STATE_RELEASING;
}

uint32_t button_task_get_press_count(platform_button_t button)
{
    if (!button_valid(button))
    {
        return 0;
    }

    return buttons[button].press_count;
}

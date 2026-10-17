#include "button-task.h"

#include <stdbool.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "led-task.h"
#include "systime/systime.h"

#define BUTTON_PIN 15
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

static bool button_read(void)
{
    // Кнопка замыкает GP15 на землю:
    // ноль на выводе означает "нажата".
    return gpio_get(BUTTON_PIN) == 0;
}

void button_task_init(void)
{
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    button_state = BUTTON_STATE_RELEASED;
    transition_us = systime_us();
    press_count = 0;
}

void button_task_handle(void)
{
    bool pressed = button_read();
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
                // Уровень не продержался 20 мс — это дребезг.
                button_state = BUTTON_STATE_RELEASED;
            }
            else if (now_us - transition_us >= BUTTON_DEBOUNCE_US)
            {
                button_state = BUTTON_STATE_PRESSED;
                press_count++;
                led_task_next_state();
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
                // Отпускание не подтвердилось — кнопка всё ещё нажата.
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

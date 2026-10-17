#include "led-task.h"

#include <stdint.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"

static const uint32_t LED_PIN = 25;
static const uint32_t DEFAULT_PERIOD_MS = 1000;

static led_state_t led_state = LED_STATE_BLINK;
static uint32_t led_period_ms = DEFAULT_PERIOD_MS;
static bool led_on = false;
static uint64_t last_toggle_us = 0;

static void led_set(bool on)
{
    led_on = on;
    gpio_put(LED_PIN, on);
}

static void led_toggle(void)
{
    led_set(!led_on);
}

// true, когда период прошёл.
// Следующее срабатывание отсчитываем от прошлого, поэтому ошибка времени
// одного вызова не сдвигает всю сетку последующих переключений.
static bool period_elapsed(uint64_t *last_us, uint64_t period_us)
{
    uint64_t now_us = time_us_64();

    if (now_us - *last_us < period_us)
    {
        return false;
    }

    *last_us += period_us;

    // Если отстали больше чем на целый период, пропущенные события
    // не догоняем, а начинаем новую сетку от текущего момента.
    if (now_us - *last_us >= period_us)
    {
        *last_us = now_us;
    }

    return true;
}

void led_task_init(void)
{
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    led_period_ms = DEFAULT_PERIOD_MS;
    led_on = false;
    led_set(false);

    led_task_set_state(LED_STATE_BLINK);
}

void led_task_set_state(led_state_t state)
{
    led_state = state;

    // Действие при входе в состояние.
    switch (led_state)
    {
        case LED_STATE_OFF:
            led_set(false);
            break;

        case LED_STATE_ON:
            led_set(true);
            break;

        case LED_STATE_BLINK:
            // Первое переключение выполняется сразу.
            led_toggle();
            last_toggle_us = time_us_64();
            break;
    }
}

led_state_t led_task_get_state(void)
{
    return led_state;
}

bool led_task_set_period_ms(uint32_t period_ms)
{
    if (period_ms == 0)
    {
        return false;
    }

    led_period_ms = period_ms;
    return true;
}

uint32_t led_task_get_period_ms(void)
{
    return led_period_ms;
}

void led_task_handle(void)
{
    switch (led_state)
    {
        case LED_STATE_OFF:
        case LED_STATE_ON:
            break;

        case LED_STATE_BLINK:
        {
            uint64_t half_period_us =
                ((uint64_t)led_period_ms * 1000) / 2;

            if (period_elapsed(&last_toggle_us, half_period_us))
            {
                led_toggle();
            }

            break;
        }
    }
}

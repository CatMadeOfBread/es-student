#include "platform/platform.h"

#include "pico/stdlib.h"

#define LED_PIN 25

#define BUTTON_LED_PIN 15
#define BUTTON_PERIOD_PIN 14

static uint32_t button_pin(platform_button_t button)
{
    switch (button)
    {
        case PLATFORM_BUTTON_LED:
            return BUTTON_LED_PIN;

        case PLATFORM_BUTTON_PERIOD:
            return BUTTON_PERIOD_PIN;

        case PLATFORM_BUTTON_COUNT:
            break;
    }

    return BUTTON_LED_PIN;
}

void platform_init(void)
{
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, false);

    gpio_init(BUTTON_LED_PIN);
    gpio_set_dir(BUTTON_LED_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_LED_PIN);

    gpio_init(BUTTON_PERIOD_PIN);
    gpio_set_dir(BUTTON_PERIOD_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PERIOD_PIN);
}

void platform_led_set(bool on)
{
    gpio_put(LED_PIN, on);
}

bool platform_button_read(platform_button_t button)
{
    // Обе кнопки замыкают соответствующий GPIO на землю.
    return gpio_get(button_pin(button)) == 0;
}

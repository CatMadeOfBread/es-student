#include "pico/stdlib.h"

#include "api.h"
#include "led.h"
#include "profiling.h"
#include "stdio-text-protocol.h"

static const uint BLINK_HALF_PERIOD_MS = 500;

static uint64_t last_toggle_us = 0;

static void blink(void)
{
    uint64_t now_us = time_us_64();

    if (now_us - last_toggle_us >= BLINK_HALF_PERIOD_MS * 1000)
    {
        last_toggle_us = now_us;
        led_toggle();
    }
}

int main(void)
{
    stdio_init_all();
    led_init();

    stdio_text_protocol_init();
    profiling_init();

    while (1)
    {
        profiling_iteration();

        blink();

        // принятая строка становится командой, команда — действием прибора
        const command_t *command = stdio_text_protocol_handle();

        if (command != NULL)
        {
            api_handle(command);
        }
    }
}

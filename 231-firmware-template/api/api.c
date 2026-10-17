#include "api.h"

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"

#include "button-task.h"
#include "device.h"
#include "led-task.h"
#include "pi-task.h"
#include "profiling.h"

// обработчик команды: получает имя и аргументы
typedef void (*api_callback_t)(const command_t *command);

// команда прибора: имя, справка для help и обработчик
typedef struct
{
    const char *name;
    const char *help;
    api_callback_t callback;
} api_command_t;

static bool parse_u32(const char *text, uint32_t *value)
{
    // strtoul приняла бы и знак, поэтому первый символ проверяем сами.
    if (text == NULL || !isdigit((unsigned char)text[0]))
    {
        return false;
    }

    char *end;
    errno = 0;
    unsigned long number = strtoul(text, &end, 10);

    if (*end != '\0' || errno == ERANGE || number > UINT32_MAX)
    {
        return false;
    }

    *value = (uint32_t)number;
    return true;
}

static const char *led_state_name(led_state_t state)
{
    switch (state)
    {
        case LED_STATE_OFF:
            return "off";

        case LED_STATE_ON:
            return "on";

        case LED_STATE_BLINK:
            return "blink";
    }

    return "unknown";
}

static void print_led_state(void)
{
    printf("led: %s, period %u ms\r\n",
           led_state_name(led_task_get_state()),
           (unsigned)led_task_get_period_ms());
}

static void cmd_info(const command_t *command)
{
    (void)command;

    device_info_t info;
    device_get_info(&info);

    printf("name: %s\r\n", DEVICE_NAME);
    printf("version: %s\r\n", FIRMWARE_VERSION);
    printf("project: %s\r\n", DEVICE_PROJECT);
    printf("repo: %s\r\n", DEVICE_REPO);
    printf("board: %s\r\n", info.board);
    printf("serial: %s\r\n", info.serial);
    printf("chip: manufacturer 0x%03x, part 0x%04x, revision %u\r\n",
           (unsigned)info.manufacturer,
           (unsigned)info.part,
           (unsigned)info.revision);
    printf("pico-sdk: %s\r\n", info.sdk_version);
}

static void cmd_uptime(const command_t *command)
{
    (void)command;

    printf("uptime: %llu ms\r\n",
           (unsigned long long)(time_us_64() / 1000));
}

static void cmd_led_enable(const command_t *command)
{
    (void)command;

    led_task_set_state(LED_STATE_ON);
    print_led_state();
}

static void cmd_led_disable(const command_t *command)
{
    (void)command;

    led_task_set_state(LED_STATE_OFF);
    print_led_state();
}

static void cmd_led_blink(const command_t *command)
{
    (void)command;

    led_task_set_state(LED_STATE_BLINK);
    print_led_state();
}

static void cmd_led_period(const command_t *command)
{
    uint32_t period_ms;

    if (command->argc != 1 ||
        !parse_u32(command->argv[0], &period_ms))
    {
        printf("error: usage led_period <period_ms>\r\n");
        return;
    }

    if (!led_task_set_period_ms(period_ms))
    {
        printf("error: period_ms must be greater than 0\r\n");
        return;
    }

    print_led_state();
}

static void cmd_button(const command_t *command)
{
    (void)command;

    printf("button: %s, presses %u\r\n",
           button_task_is_pressed() ? "pressed" : "released",
           (unsigned)button_task_get_press_count());
}

static void cmd_pi_start(const command_t *command)
{
    const uint32_t default_terms = 1000000;
    uint32_t terms = default_terms;

    if (command->argc > 1)
    {
        printf("error: usage pi_start [<terms>]\r\n");
        return;
    }

    if (command->argc == 1)
    {
        if (!parse_u32(command->argv[0], &terms))
        {
            printf("error: usage pi_start [<terms>]\r\n");
            return;
        }

        if (terms == 0)
        {
            printf("error: terms must be greater than 0\r\n");
            return;
        }
    }

    if (!pi_task_start(terms))
    {
        printf("error: pi is busy\r\n");
        return;
    }

    printf("pi: started, %u terms\r\n", (unsigned)terms);
}

static void cmd_pi(const command_t *command)
{
    (void)command;

    switch (pi_task_get_state())
    {
        case PI_TASK_NOT_STARTED:
            printf("pi: not started\r\n");
            break;

        case PI_TASK_RUNNING:
            printf("pi: running\r\n");
            break;

        case PI_TASK_READY:
        {
            pi_task_result_t result;

            if (pi_task_get_result(&result))
            {
                printf("pi: %.9f in %llu ms\r\n",
                       result.value,
                       (unsigned long long)result.time_ms);
            }

            break;
        }
    }
}

static void cmd_profiling(const command_t *command)
{
    (void)command;

    for (uint32_t id = 0;
         id < PROFILING_STOPWATCH_COUNT;
         id++)
    {
        profiling_result_t result;

        if (!profiling_get(id, &result))
        {
            continue;
        }

        printf("%-10s count %-10u mean %.2f us  max %u us\r\n",
               result.name,
               (unsigned)result.count,
               result.mean_us,
               (unsigned)result.max_us);
    }
}

static void cmd_profiling_reset(const command_t *command)
{
    (void)command;

    profiling_reset();
    printf("profiling reset\r\n");
}

static const api_command_t api_commands[] = {
    {
        "info",
        "device passport",
        cmd_info,
    },
    {
        "uptime",
        "time since reset",
        cmd_uptime,
    },
    {
        "led_enable",
        "turn led on",
        cmd_led_enable,
    },
    {
        "led_disable",
        "turn led off",
        cmd_led_disable,
    },
    {
        "led_blink",
        "blink led",
        cmd_led_blink,
    },
    {
        "led_period",
        "set blink period: led_period <period_ms>",
        cmd_led_period,
    },
    {
        "button",
        "button state and press count",
        cmd_button,
    },
    {
        "pi_start",
        "start pi calculation: pi_start [<terms>]",
        cmd_pi_start,
    },
    {
        "pi",
        "pi calculation result or state",
        cmd_pi,
    },
    {
        "profiling",
        "profiling results",
        cmd_profiling,
    },
    {
        "profiling_reset",
        "reset profiling results",
        cmd_profiling_reset,
    },
};

static const uint32_t api_command_count =
    sizeof(api_commands) / sizeof(api_commands[0]);

static void print_help(void)
{
    printf("%-16s %s\r\n",
           "help",
           "list of commands");

    for (uint32_t i = 0; i < api_command_count; i++)
    {
        printf("%-16s %s\r\n",
               api_commands[i].name,
               api_commands[i].help);
    }
}

void api_handle(const command_t *command)
{
    if (command == NULL || command->name == NULL)
    {
        return;
    }

    if (command->truncated)
    {
        printf("error: command longer than 63 characters or 4 arguments\r\n");
        return;
    }

    if (strcmp(command->name, "help") == 0)
    {
        print_help();
        return;
    }

    for (uint32_t i = 0; i < api_command_count; i++)
    {
        if (strcmp(command->name, api_commands[i].name) == 0)
        {
            api_commands[i].callback(command);
            return;
        }
    }

    printf("error: unknown command '%s', try help\r\n",
           command->name);
}

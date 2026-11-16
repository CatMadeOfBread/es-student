#include <stddef.h>

#include "api/api.h"
#include "block-task/block-task.h"
#include "button-task/button-task.h"
#include "firmware.h"
#include "led-task/led-task.h"
#include "logging/log.h"
#include "pi-task/pi-task.h"
#include "platform/platform.h"
#include "profiling/profiling.h"
#include "stdio-text-protocol/stdio-text-protocol.h"
#include "stream-task/stream-task.h"
#include "telemetry-task/telemetry-task.h"

typedef enum
{
    STOPWATCH_LOOP,
    STOPWATCH_COMMAND,
    STOPWATCH_LED,
    STOPWATCH_BUTTON,
    STOPWATCH_PI,
    STOPWATCH_TELEMETRY,
    STOPWATCH_BLOCK,
    STOPWATCH_STREAM,
} stopwatch_id_t;

int main(void)
{
    platform_init();

    LOG_INF("%s %s\n",
            FIRMWARE_NAME,
            FIRMWARE_VERSION);

    led_task_init();

    // Связь кнопки со светодиодом задаёт прибор,
    // а не сама задача кнопки.
    button_task_init(led_task_next_state);

    pi_task_init();
    telemetry_task_init(api_print_telemetry);
    stream_task_init(api_send_frame);
    stdio_text_protocol_init();

    profiling_stopwatch_init(
        STOPWATCH_LOOP, "loop");
    profiling_stopwatch_init(
        STOPWATCH_COMMAND, "command");
    profiling_stopwatch_init(
        STOPWATCH_LED, "led");
    profiling_stopwatch_init(
        STOPWATCH_BUTTON, "button");
    profiling_stopwatch_init(
        STOPWATCH_PI, "pi");
    profiling_stopwatch_init(
        STOPWATCH_TELEMETRY, "telemetry");
    profiling_stopwatch_init(
        STOPWATCH_BLOCK, "block");
    profiling_stopwatch_init(
        STOPWATCH_STREAM, "stream");

    while (1)
    {
        profiling_start(STOPWATCH_LOOP);

        profiling_start(STOPWATCH_LED);
        led_task_handle();
        profiling_stop(STOPWATCH_LED);

        profiling_start(STOPWATCH_BUTTON);
        button_task_handle();
        profiling_stop(STOPWATCH_BUTTON);

        profiling_start(STOPWATCH_PI);
        pi_task_handle();
        profiling_stop(STOPWATCH_PI);

        profiling_start(STOPWATCH_TELEMETRY);
        telemetry_task_handle();
        profiling_stop(STOPWATCH_TELEMETRY);

        profiling_start(STOPWATCH_BLOCK);
        block_task_handle();
        profiling_stop(STOPWATCH_BLOCK);

        profiling_start(STOPWATCH_STREAM);
        stream_task_handle();
        profiling_stop(STOPWATCH_STREAM);

        profiling_start(STOPWATCH_COMMAND);

        const command_t *command =
            stdio_text_protocol_handle();

        if (command != NULL)
        {
            api_handle(command);
        }

        profiling_stop(STOPWATCH_COMMAND);

        // Все задачи вернули управление — суперцикл жив.
        platform_watchdog_feed();

        profiling_stop(STOPWATCH_LOOP);
    }
}

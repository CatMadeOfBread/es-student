#include "telemetry-task/telemetry-task.h"

#include <stddef.h>

#include "platform/platform.h"
#include "systime/systime.h"

static telemetry_state_t state;
static telemetry_task_callback_t callback;
static uint32_t sample_period_ms;
static uint64_t last_us;

void telemetry_task_init(telemetry_task_callback_t on_record)
{
    callback = on_record;
    sample_period_ms = 100;
    last_us = 0;
    state = TELEMETRY_STATE_IDLE;
}

bool telemetry_task_start(uint32_t period_ms)
{
    if (period_ms < 10)
    {
        return false;
    }

    sample_period_ms = period_ms;

    // Первая запись — на ближайшей итерации суперцикла.
    last_us = systime_us() - (uint64_t)period_ms * 1000;
    state = TELEMETRY_STATE_RUN;
    return true;
}

void telemetry_task_stop(void)
{
    state = TELEMETRY_STATE_IDLE;
}

void telemetry_task_handle(void)
{
    switch (state)
    {
        case TELEMETRY_STATE_IDLE:
            return;

        case TELEMETRY_STATE_RUN:
        {
            if (!systime_period_elapsed(
                    &last_us, (uint64_t)sample_period_ms * 1000))
            {
                return;
            }

            telemetry_record_t record;
            record.time_us = systime_us();
            record.voltage_v = platform_voltage_read();
            record.temp_c = platform_temp_read();

            if (callback != NULL)
            {
                callback(&record);
            }
            return;
        }
    }
}

telemetry_state_t telemetry_task_get_state(void)
{
    return state;
}

uint32_t telemetry_task_get_period_ms(void)
{
    return sample_period_ms;
}

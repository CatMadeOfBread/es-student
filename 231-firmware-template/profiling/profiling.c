#include "profiling.h"

#include <stdbool.h>
#include <stdint.h>

#include "pico/stdlib.h"

typedef struct
{
    bool initialized;
    const char *name;

    uint32_t start_us;

    uint32_t count;
    uint32_t sum_us;
    uint32_t max_us;
} stopwatch_t;

static stopwatch_t stopwatches[PROFILING_STOPWATCH_COUNT];

void profiling_stopwatch_init(uint32_t id, const char *name)
{
    if (id >= PROFILING_STOPWATCH_COUNT)
    {
        return;
    }

    stopwatches[id].initialized = true;
    stopwatches[id].name = name;
    stopwatches[id].start_us = 0;
    stopwatches[id].count = 0;
    stopwatches[id].sum_us = 0;
    stopwatches[id].max_us = 0;
}

void profiling_start(uint32_t id)
{
    if (id >= PROFILING_STOPWATCH_COUNT ||
        !stopwatches[id].initialized)
    {
        return;
    }

    stopwatches[id].start_us = time_us_32();
}

void profiling_stop(uint32_t id)
{
    if (id >= PROFILING_STOPWATCH_COUNT ||
        !stopwatches[id].initialized)
    {
        return;
    }

    uint32_t elapsed_us =
        time_us_32() - stopwatches[id].start_us;

    stopwatches[id].count++;
    stopwatches[id].sum_us += elapsed_us;

    if (elapsed_us > stopwatches[id].max_us)
    {
        stopwatches[id].max_us = elapsed_us;
    }
}

bool profiling_get(uint32_t id, profiling_result_t *result)
{
    if (id >= PROFILING_STOPWATCH_COUNT ||
        !stopwatches[id].initialized ||
        result == NULL)
    {
        return false;
    }

    result->name = stopwatches[id].name;
    result->count = stopwatches[id].count;
    result->max_us = stopwatches[id].max_us;

    if (stopwatches[id].count == 0)
    {
        result->mean_us = 0.0f;
    }
    else
    {
        result->mean_us =
            (float)stopwatches[id].sum_us /
            (float)stopwatches[id].count;
    }

    return true;
}

void profiling_reset(void)
{
    for (uint32_t i = 0;
         i < PROFILING_STOPWATCH_COUNT;
         i++)
    {
        if (!stopwatches[i].initialized)
        {
            continue;
        }

        // profiling_reset() может быть вызван прямо внутри
        // измеряемой команды. Поэтому начало текущего измерения
        // переносим на момент сброса, а не обнуляем.
        stopwatches[i].start_us = time_us_32();
        stopwatches[i].count = 0;
        stopwatches[i].sum_us = 0;
        stopwatches[i].max_us = 0;
    }
}

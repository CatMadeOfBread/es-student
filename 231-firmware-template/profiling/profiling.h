#pragma once

#include <stdbool.h>
#include <stdint.h>

#define PROFILING_STOPWATCH_COUNT 5

typedef struct
{
    const char *name;
    uint32_t count;
    float mean_us;
    uint32_t max_us;
} profiling_result_t;

// Сумма длительностей хранится в 32 битах.
// 2^32 микросекунд — около 71 минуты непрерывно накопленного времени,
// после чего сумма переполнится.
void profiling_stopwatch_init(uint32_t id, const char *name);

void profiling_start(uint32_t id);
void profiling_stop(uint32_t id);

bool profiling_get(uint32_t id, profiling_result_t *result);
void profiling_reset(void);

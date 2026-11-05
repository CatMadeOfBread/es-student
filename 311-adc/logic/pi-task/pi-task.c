#include "pi-task/pi-task.h"

#include <stdint.h>

#include "systime/systime.h"

// Один член ряда занимал около 4,7 мкс.
// 100 членов занимают около 470 мкс, то есть меньше 0,5 мс.
#define PI_TERMS_PER_CALL 100

static pi_task_state_t pi_state = PI_TASK_NOT_STARTED;

static uint32_t pi_terms = 0;
static uint32_t pi_k = 0;

static double pi_sum = 0.0;
static double pi_sign = 1.0;
static double pi_value = 0.0;

static uint64_t pi_start_us = 0;
static uint64_t pi_time_ms = 0;

void pi_task_init(void)
{
    pi_state = PI_TASK_NOT_STARTED;

    pi_terms = 0;
    pi_k = 0;

    pi_sum = 0.0;
    pi_sign = 1.0;
    pi_value = 0.0;

    pi_start_us = 0;
    pi_time_ms = 0;
}

bool pi_task_start(uint32_t terms)
{
    if (pi_state == PI_TASK_RUNNING)
    {
        return false;
    }

    pi_terms = terms;
    pi_k = 0;

    pi_sum = 0.0;
    pi_sign = 1.0;
    pi_value = 0.0;

    pi_time_ms = 0;
    pi_start_us = systime_us();

    pi_state = PI_TASK_RUNNING;

    return true;
}

void pi_task_handle(void)
{
    if (pi_state != PI_TASK_RUNNING)
    {
        return;
    }

    // Вычисляем не больше PI_TERMS_PER_CALL членов.
    // Разность не переполняется даже при pi_terms == UINT32_MAX.
    uint32_t remaining = pi_terms - pi_k;
    uint32_t count = remaining;

    if (count > PI_TERMS_PER_CALL)
    {
        count = PI_TERMS_PER_CALL;
    }

    for (uint32_t i = 0; i < count; i++)
    {
        pi_sum += pi_sign / (2.0 * pi_k + 1.0);
        pi_sign = -pi_sign;
        pi_k++;
    }

    if (pi_k >= pi_terms)
    {
        pi_value = pi_sum * 4.0;
        pi_time_ms = (systime_us() - pi_start_us) / 1000;
        pi_state = PI_TASK_READY;
    }
}

pi_task_state_t pi_task_get_state(void)
{
    return pi_state;
}

bool pi_task_get_result(pi_task_result_t *result)
{
    if (pi_state != PI_TASK_READY)
    {
        return false;
    }

    result->value = pi_value;
    result->time_ms = pi_time_ms;

    return true;
}

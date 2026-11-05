#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    PI_TASK_NOT_STARTED,
    PI_TASK_RUNNING,
    PI_TASK_READY,
} pi_task_state_t;

typedef struct
{
    double value;
    uint64_t time_ms;
} pi_task_result_t;

void pi_task_init(void);
bool pi_task_start(uint32_t terms);
void pi_task_handle(void);

pi_task_state_t pi_task_get_state(void);
bool pi_task_get_result(pi_task_result_t *result);

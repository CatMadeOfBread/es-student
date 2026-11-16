#include "stream-task/stream-task.h"

#include <stddef.h>

#include "platform/platform.h"

static stream_task_callback_t frame_callback;
static stream_state_t state;
static uint8_t sample_bits;
static uint32_t frames_sent;
static uint32_t samples_sent;

void stream_task_init(stream_task_callback_t on_frame)
{
    frame_callback = on_frame;
    state = STREAM_STATE_IDLE;
}

bool stream_task_start(uint8_t bits)
{
    if (state == STREAM_STATE_RUN || frame_callback == NULL ||
        !platform_stream_start(bits))
    {
        return false;
    }

    sample_bits = bits;
    frames_sent = 0;
    samples_sent = 0;
    state = STREAM_STATE_RUN;
    return true;
}

void stream_task_stop(void)
{
    if (state == STREAM_STATE_RUN)
    {
        platform_stream_stop();
        state = STREAM_STATE_IDLE;
    }
}

void stream_task_handle(void)
{
    if (state != STREAM_STATE_RUN)
    {
        return;
    }

    uint32_t count;
    const uint8_t *data = platform_stream_next(&count);
    if (data == NULL)
    {
        return;
    }

    const stream_header_t header = {
        .sync_word = STREAM_SYNC_WORD,
        .number = (uint16_t)frames_sent,
        .count = (uint16_t)count,
    };

    frame_callback(&header, data, count * (sample_bits / 8u));
    platform_stream_release();

    frames_sent++;
    samples_sent += count;
}

stream_state_t stream_task_get_state(void)
{
    return state;
}

uint8_t stream_task_get_bits(void)
{
    return sample_bits;
}

uint32_t stream_task_get_frames(void)
{
    return frames_sent;
}

uint32_t stream_task_get_samples(void)
{
    return samples_sent;
}

uint32_t stream_task_get_overruns(void)
{
    return platform_stream_overruns();
}

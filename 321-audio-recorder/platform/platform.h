#pragma once

#include <stdbool.h>
#include <stdint.h>

// Запускает стандартный ввод-вывод и настраивает выводы прибора.
void platform_init(void);

void platform_led_set(bool on);

// Мигание по будильнику независимо от суперцикла.
// false — период вне диапазона 10..60000 мс или будильник не создан.
bool platform_blink_start(uint32_t period_ms);

// Останавливает мигание, гасит светодиод и возвращает число переключений.
uint32_t platform_blink_stop(void);

// Текущее физическое состояние кнопки без подавления дребезга.
bool platform_button_read(void);

// Напряжение на GP26 в вольтах.
float platform_voltage_read(void);

// Температура кристалла в градусах Цельсия.
float platform_temp_read(void);

// Включает заряд RC-цепи через GP22 или соединяет её с землёй.
void platform_charge_set(bool on);

// Занят ли АЦП захватом.
bool platform_adc_busy(void);

// Захват count отсчётов GP26 по будильнику с периодом period_us.
#define PLATFORM_CAPTURE_MAX 2000
bool platform_capture_start(uint32_t count, uint32_t period_us);
bool platform_capture_done(void);

// Отсчёты, разброс интервалов и доля времени внутри обработчика.
typedef struct {
    const uint16_t *samples;
    uint32_t count;
    uint32_t period_us;
    uint32_t min_interval_us;
    uint32_t max_interval_us;
    float handler_percent;
} platform_capture_t;

void platform_capture_get(platform_capture_t *capture);

// Частота отсчётов АЦП в блоке и потоке.
#define PLATFORM_SAMPLE_RATE_HZ 44100

// Захват блока отсчётов GP26 через DMA.
// false — АЦП занят или число отсчётов вне диапазона 1..8192.
#define PLATFORM_BLOCK_MAX 8192
bool platform_block_start(uint32_t count);
bool platform_block_done(void);
const uint16_t *platform_block_samples(void);

// Время от запуска АЦП до завершения захвата блока.
uint32_t platform_block_time_us(void);

// Поток: два буфера по 1024 байта. Пока поддерживается 16 бит.
#define PLATFORM_STREAM_BYTES 1024
bool platform_stream_start(uint8_t bits);
void platform_stream_stop(void);

// Следующий готовый буфер или NULL. В samples записывается число отсчётов.
const uint8_t *platform_stream_next(uint32_t *samples);

// Освобождает буфер после отправки.
void platform_stream_release(void);

// Сколько раз DMA застал буфер неотправленным.
uint32_t platform_stream_overruns(void);

// Отправляет двоичные данные без преобразования переводов строки.
void platform_write(const void *data, uint32_t bytes);

// Тон на GP20: 20..20000 Гц, 0 выключает тон.
bool platform_tone_set(uint32_t freq_hz);

// false, если таймаут вне диапазона 1..8388 мс.
bool platform_watchdog_enable(uint32_t timeout_ms);
void platform_watchdog_feed(void);

// Причина последнего сброса и число сбросов по watchdog подряд.
bool platform_watchdog_caused_reset(uint32_t *restarts);

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

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

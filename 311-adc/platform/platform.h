#pragma once

#include <stdbool.h>

// Запускает стандартный ввод-вывод и настраивает выводы прибора.
void platform_init(void);

void platform_led_set(bool on);

// Текущее физическое состояние кнопки без подавления дребезга.
bool platform_button_read(void);

// Напряжение на GP26 в вольтах.
float platform_voltage_read(void);

// Температура кристалла в градусах Цельсия.
float platform_temp_read(void);

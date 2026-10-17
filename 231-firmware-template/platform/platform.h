#pragma once

#include <stdbool.h>

// Запускает стандартный ввод-вывод и настраивает выводы прибора.
void platform_init(void);

void platform_led_set(bool on);

// Текущее физическое состояние кнопки без подавления дребезга.
bool platform_button_read(void);

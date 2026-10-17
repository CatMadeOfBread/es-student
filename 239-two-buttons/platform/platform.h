#pragma once

#include <stdbool.h>

typedef enum
{
    PLATFORM_BUTTON_LED,
    PLATFORM_BUTTON_PERIOD,
    PLATFORM_BUTTON_COUNT,
} platform_button_t;

// Запускает стандартный ввод-вывод и настраивает выводы прибора.
void platform_init(void);

void platform_led_set(bool on);

// Текущее физическое состояние выбранной кнопки без подавления дребезга.
bool platform_button_read(platform_button_t button);

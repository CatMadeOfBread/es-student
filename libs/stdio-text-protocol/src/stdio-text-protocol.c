#include "stdio-text-protocol/stdio-text-protocol.h"

#include <stdio.h>

#include "pico/stdlib.h"

static char line[STDIO_TEXT_PROTOCOL_LINE_SIZE];
static uint32_t line_length = 0;
static bool line_truncated = false;
static command_t parsed_command;

static const command_t *parse_line(void)
{
    bool truncated = line_truncated;

    line[line_length] = '\0';

    parsed_command.name = NULL;
    parsed_command.argc = 0;
    parsed_command.truncated = truncated;

    for (uint32_t i = 0; i < STDIO_TEXT_PROTOCOL_MAX_ARGS; i++)
    {
        parsed_command.argv[i] = NULL;
    }

    char *cursor = line;

    while (*cursor != '\0')
    {
        // Пропускаем пробелы и одновременно превращаем их в концы строк.
        while (*cursor == ' ')
        {
            *cursor = '\0';
            cursor++;
        }

        if (*cursor == '\0')
        {
            break;
        }

        char *word = cursor;

        while (*cursor != '\0' && *cursor != ' ')
        {
            cursor++;
        }

        if (parsed_command.name == NULL)
        {
            parsed_command.name = word;
        }
        else if (parsed_command.argc < STDIO_TEXT_PROTOCOL_MAX_ARGS)
        {
            parsed_command.argv[parsed_command.argc] = word;
            parsed_command.argc++;
        }
        else
        {
            parsed_command.truncated = true;
        }
    }

    line_length = 0;
    line_truncated = false;

    if (parsed_command.name == NULL)
    {
        return NULL;
    }

    return &parsed_command;
}

void stdio_text_protocol_init(void)
{
    line_length = 0;
    line_truncated = false;

    parsed_command.name = NULL;
    parsed_command.argc = 0;
    parsed_command.truncated = false;
}

const command_t *stdio_text_protocol_handle(void)
{
    int symbol = getchar_timeout_us(0);

    if (symbol == PICO_ERROR_TIMEOUT)
    {
        return NULL;
    }

    if (symbol == '\r' || symbol == '\n')
    {
        if (line_length == 0)
        {
            return NULL;
        }

        printf("\r\n");
        return parse_line();
    }

    if (symbol == 0x08 || symbol == 0x7f)
    {
        if (line_length > 0)
        {
            line_length--;
            line_truncated = false;
            printf("\b \b");
        }

        return NULL;
    }

    if (symbol >= 0x20 && symbol <= 0x7e)
    {
        if (line_length + 1 < STDIO_TEXT_PROTOCOL_LINE_SIZE)
        {
            line[line_length] = (char)symbol;
            line_length++;
            putchar(symbol);
        }
        else
        {
            line_truncated = true;
        }
    }

    return NULL;
}

#include "memory.h"
#include "command.h"
#include "device.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "hardware/regs/addressmap.h"
#include "pico/stdlib.h"

int main(void);

extern char __flash_binary_start;
extern char __flash_binary_end;
extern char __boot2_start__;
extern char __boot2_end__;
extern char __etext;
extern char __data_start__;
extern char __data_end__;
extern char __bss_start__;
extern char __bss_end__;
extern char __HeapLimit;
extern char __StackBottom;
extern char __StackTop;

uint32_t data_variable = 100;
uint32_t bss_variable;

static void row(const char *name, uintptr_t start, uintptr_t end)
{
    printf("%-10s 0x%08x 0x%08x %8u\n",
           name,
           (unsigned)start,
           (unsigned)end,
           (unsigned)(end - start));
}

void mem_info(void)
{
    uintptr_t flash_start = XIP_BASE;
    uintptr_t flash_end = XIP_BASE + PICO_FLASH_SIZE_BYTES;

    uintptr_t sram_start = SRAM_BASE;
    uintptr_t sram_end = SRAM_BASE + 264 * 1024;

    uintptr_t rom_start = ROM_BASE;
    uintptr_t rom_end = ROM_BASE + 16 * 1024;

    uintptr_t image_start = (uintptr_t)&__flash_binary_start;
    uintptr_t image_end = (uintptr_t)&__flash_binary_end;

    uintptr_t boot2_start = (uintptr_t)&__boot2_start__;
    uintptr_t boot2_end = (uintptr_t)&__boot2_end__;

    uintptr_t text_start = boot2_end;
    uintptr_t text_end = (uintptr_t)&__etext;

    uintptr_t data_ram_start = (uintptr_t)&__data_start__;
    uintptr_t data_ram_end = (uintptr_t)&__data_end__;
    uintptr_t data_size = data_ram_end - data_ram_start;

    uintptr_t data_flash_start = (uintptr_t)&__etext;
    uintptr_t data_flash_end = data_flash_start + data_size;

    uintptr_t bss_start = (uintptr_t)&__bss_start__;
    uintptr_t bss_end = (uintptr_t)&__bss_end__;

    uintptr_t heap_start = bss_end;
    uintptr_t heap_end = (uintptr_t)&__HeapLimit;

    uintptr_t stack_start = (uintptr_t)&__StackBottom;
    uintptr_t stack_end = (uintptr_t)&__StackTop;

    printf("area       start      end        size\n");

    row("flash", flash_start, flash_end);
    row("sram", sram_start, sram_end);
    row("rom", rom_start, rom_end);

    row("image", image_start, image_end);
    row("free", image_end, flash_end);
    row("boot2", boot2_start, boot2_end);
    row("text", text_start, text_end);

    row("data flash", data_flash_start, data_flash_end);
    row("data ram", data_ram_start, data_ram_end);
    row("bss", bss_start, bss_end);
    row("heap", heap_start, heap_end);
    row("stack", stack_start, stack_end);

    unsigned image_size = (unsigned)(image_end - image_start);
    unsigned boot2_size = (unsigned)(boot2_end - boot2_start);
    unsigned text_size = (unsigned)(text_end - text_start);
    unsigned data_size_u = (unsigned)data_size;
    unsigned flash_free = (unsigned)(flash_end - image_end);

    unsigned bss_size = (unsigned)(bss_end - bss_start);
    unsigned ram_used = data_size_u + bss_size;
    unsigned heap_size = (unsigned)(heap_end - heap_start);
    unsigned stack_size = (unsigned)(stack_end - stack_start);

    printf("\ntotal\n");
    printf("  flash image %8u = boot2 %u + text %u + data %u\n",
           image_size, boot2_size, text_size, data_size_u);
    printf("  flash free  %8u of %u\n",
           flash_free, (unsigned)PICO_FLASH_SIZE_BYTES);
    printf("  ram used    %8u = data %u + bss %u\n",
           ram_used, data_size_u, bss_size);
    printf("  ram free    %8u for heap and %u for stack\n",
           heap_size, stack_size);
}


void fw_info(void)
{
    data_variable++;
    bss_variable++;

    uint16_t *main_code = (uint16_t *)((uintptr_t)main & ~1u);
    uint16_t *fw_info_code = (uint16_t *)((uintptr_t)fw_info & ~1u);

    uint32_t stack_variable = 1946;
    uint32_t *heap_variable = malloc(sizeof(uint32_t));

    printf("object          address     value\n");

    printf("main            0x%08x  0x%04x\n",
           (uint)(uintptr_t)main,
           *main_code);

    printf("fw_info         0x%08x  0x%04x\n",
           (uint)(uintptr_t)fw_info,
           *fw_info_code);

    printf("commands        0x%08x\n",
           (uint)(uintptr_t)commands);

    for (uint i = 0; i < command_count; i++)
    {
        printf("- %-11s 0x%08x\n",
               commands[i].name,
               (uint)(uintptr_t)commands[i].handler);
    }

    printf("DEVICE_PROJECT  0x%08x  %s\n",
           (uint)(uintptr_t)DEVICE_PROJECT,
           DEVICE_PROJECT);

    printf("DEVICE_BOARD    0x%08x  %s\n",
           (uint)(uintptr_t)DEVICE_BOARD,
           DEVICE_BOARD);

    printf("data_variable   0x%08x  %lu\n",
           (uint)(uintptr_t)&data_variable,
           (unsigned long)data_variable);

    printf("bss_variable    0x%08x  %lu\n",
           (uint)(uintptr_t)&bss_variable,
           (unsigned long)bss_variable);

    printf("stack_variable  0x%08x  %lu\n",
           (uint)(uintptr_t)&stack_variable,
           (unsigned long)stack_variable);

    if (heap_variable != NULL)
    {
        *heap_variable = 1951;

        printf("heap_variable   0x%08x  %lu\n",
               (uint)(uintptr_t)heap_variable,
               (unsigned long)*heap_variable);
    }
    else
    {
        printf("heap_variable   malloc failed\n");
    }

    free(heap_variable);
}

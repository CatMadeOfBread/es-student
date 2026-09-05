cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(blink C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(blink
    main.c
)

target_link_libraries(blink
    pico_stdlib
    hardware_gpio
)

pico_set_linker_script(blink ${CMAKE_CURRENT_LIST_DIR}/memmap_rp2040.ld)

pico_add_extra_outputs(blink)

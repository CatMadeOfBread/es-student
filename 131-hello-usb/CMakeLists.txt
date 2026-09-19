cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(131_hello_usb)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(${PROJECT_NAME}
    main.c
)

target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    hardware_gpio
)

pico_set_linker_script(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/memmap_rp2040.ld)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})

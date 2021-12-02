cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(psion_recreate C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(psion_recreate
psion_recreate.c
display.c
)

pico_enable_stdio_usb(psion_recreate 1)
pico_enable_stdio_uart(psion_recreate 1)
pico_add_extra_outputs(psion_recreate)
target_link_libraries(psion_recreate pico_stdlib)

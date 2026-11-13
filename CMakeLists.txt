cmake_minimum_required(VERSION 3.13...3.27)

set(PICO_BOARD pico)              # обычная Pico на RP2040
include(pico_sdk_import.cmake)    # подтягивает SDK по PICO_SDK_PATH

project(pulsar_onboard C CXX ASM)
set(CMAKE_C_STANDARD 11)

pico_sdk_init()

# Исходники прошивки. Новый модуль — новая строка здесь:
#   max30102.c  fir.c  ssf.c  hrv.c  link.c
add_executable(pulsar_onboard
    src/main.c
    src/max30102.c
    src/fir.c
    src/ssf.c
    src/hrv.c
)
target_include_directories(pulsar_onboard PRIVATE include)
target_link_libraries(pulsar_onboard pico_stdlib hardware_i2c)
target_compile_options(pulsar_onboard PRIVATE -Wall -Wextra -Wsign-conversion)

pico_enable_stdio_usb(pulsar_onboard 1)   # printf и заливка без кнопки
pico_enable_stdio_uart(pulsar_onboard 0)  # UART понадобится для CH9143 — через hardware_uart

pico_add_extra_outputs(pulsar_onboard)

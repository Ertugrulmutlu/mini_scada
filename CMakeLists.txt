cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)
project(mini_scada C CXX ASM)

pico_sdk_init()
add_executable(mini_scada
    main.cpp
    utils.cpp
)
target_link_libraries(mini_scada
    pico_stdlib
    hardware_adc
)

pico_enable_stdio_usb(mini_scada 1)
pico_enable_stdio_uart(mini_scada 0)

pico_add_extra_outputs(mini_scada)

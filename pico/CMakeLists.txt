cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(my_project)

set(PICO_BOARD pico)
# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

include(brotli/interfaceLibForPicoSDK.cmake)

# rest of your project
add_executable(main
    main.cpp
)

pico_generate_pio_header(main ${CMAKE_CURRENT_LIST_DIR}/ws2812_parallel.pio)

target_link_libraries(main
    pico_stdlib
    pico_multicore
    hardware_irq
    hardware_dma
    hardware_pio
    brotli
)

pico_enable_stdio_usb(main 0)
pico_enable_stdio_uart(main 1)

pico_add_extra_outputs(main)
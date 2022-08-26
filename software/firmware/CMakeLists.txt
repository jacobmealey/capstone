cmake_minimum_required(VERSION 3.23)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

add_executable(midi_firmware
        main.c
    )

project(midi_firmware C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)



# pull in common dependencies
pico_sdk_init()


# usb output
pico_enable_stdio_usb(midi_firmware 1)
pico_enable_stdio_uart(midi_firmware 0)
pico_add_extra_outputs(midi_firmware)

# create map/bin/hex file etc.
pico_add_extra_outputs(midi_firmware)


target_link_libraries(midi_firmware pico_stdlib)

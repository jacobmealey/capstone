cmake_minimum_required(VERSION 3.23)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include($ENV{PICO_SDK_PATH}/lib/tinyusb/hw/bsp/rp2040/family.cmake)

include_directories($ENV{PICO_SDK_PATH}/tinyusb/src)

add_executable(midi_firmware
        main.c
        usb_descriptors.c
    )

project(midi_firmware C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)



# pull in common dependencies
pico_sdk_init()


# usb output
pico_enable_stdio_usb(midi_firmware 0)
pico_enable_stdio_uart(midi_firmware 0)
pico_add_extra_outputs(midi_firmware)

target_include_directories(midi_firmware PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(midi_firmware PUBLIC pico_stdlib tinyusb_device tinyusb_board)

# create map/bin/hex file etc.
pico_add_extra_outputs(midi_firmware)
# example_auto_set_url(midi_firmware)

cmake_minimum_required(VERSION 3.12)

include(pico-sdk/pico_sdk_init.cmake)

project(rp2040_ov2640)

pico_sdk_init()

add_executable(firmware
	ov2640.c
	main.c
)

pico_generate_pio_header(firmware ${CMAKE_CURRENT_LIST_DIR}/image.pio)


pico_enable_stdio_usb(firmware 1)
pico_enable_stdio_uart(firmware 0)  # <— turn off UART if not used


target_link_libraries(firmware
    pico_stdlib
    # hardware_uart
    hardware_pio
    hardware_dma
    hardware_i2c
	hardware_pwm
)

pico_add_extra_outputs(firmware)

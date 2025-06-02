#include <stdio.h>
#include "ov2640.h"
#include "ov2640_init.h"
#include "hardware/dma.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "image.pio.h"

static const uint8_t OV2640_ADDR = 0x60 >> 1;

void ov2640_init(struct ov2640_config *config) {
	printf("first line OV init\n");
	// XCLK generation (~20.83 MHz)
	gpio_set_function(config->pin_xclk, GPIO_FUNC_PWM);
	uint slice_num = pwm_gpio_to_slice_num(config->pin_xclk);

	printf("second line OV init\n");
	// 6 cycles (0 to 5), 125 MHz / 6 = ~20.83 MHz wrap rate
	pwm_set_wrap(slice_num, 5);
	pwm_set_gpio_level(config->pin_xclk, 3);
	pwm_set_enabled(slice_num, true);

	printf("third line OV init - SCCB\n");
	// SCCB I2C @ 100 kHz
	gpio_set_function(config->pin_sioc, GPIO_FUNC_I2C);
	gpio_set_function(config->pin_siod, GPIO_FUNC_I2C);
	gpio_pull_up(config->pin_sioc);
	gpio_pull_up(config->pin_siod);
	i2c_init(config->sccb, 10 * 1000);
	// uint yada = i2c_init(config->sccb, 100 * 1000);
	// printf("i2c_init return %d\n", yada);
	// sleep_ms(10);

	printf("fourth line OV init - init reset pin\n");
	// Initialise reset pin
	gpio_init(config->pin_resetb);
	gpio_set_dir(config->pin_resetb, GPIO_OUT);

	printf("fifth line OV init\n");
	// Reset camera, and give it some time to wake back up
	gpio_put(config->pin_resetb, 0);
	sleep_ms(100);
	gpio_put(config->pin_resetb, 1);
	sleep_ms(100);

	printf("sixth line OV init\n");
	// Initialise the camera itself over SCCB
	ov2640_regs_write(config, ov2640_vga);
	printf("sixth 1 line OV init\n");
	ov2640_regs_write(config, ov2640_uxga_cif);
	printf("sixth 2 line OV init\n");

	printf("seventh line OV init\n");
	// Set RGB565 output mode
	ov2640_reg_write(config, 0xff, 0x00);
	printf("seventh1 line OV init\n");
	ov2640_reg_write(config, 0xDA, (ov2640_reg_read(config, 0xDA) & 0xC) | 0x8);
	printf("seventh2 line OV init\n");

	printf("eighth line OV init\n");
	// Enable image RX PIO
	uint offset = pio_add_program(config->pio, &image_program);
	printf("offset = %d\n", offset);
	printf("base = %d\n", config->pin_y2_pio_base);
	image_program_init(config->pio, config->pio_sm, offset, config->pin_y2_pio_base);
}

void ov2640_capture_frame(struct ov2640_config *config) {
	printf("capture frame start\n");
	dma_channel_config c = dma_channel_get_default_config(config->dma_channel);
	channel_config_set_read_increment(&c, false);
	channel_config_set_write_increment(&c, true);
	channel_config_set_dreq(&c, pio_get_dreq(config->pio, config->pio_sm, false));
	channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
	
	printf("dma_channel_configure\n");
	dma_channel_configure(
		config->dma_channel, &c,
		config->image_buf,
		&config->pio->rxf[config->pio_sm],
		config->image_buf_size,
		false
	);

	printf("wait for vsync rising edge to start frame\n");
	// Wait for vsync rising edge to start frame
	while (gpio_get(config->pin_vsync) == true);
	printf("vsync low, waiting for high\n");
	while (gpio_get(config->pin_vsync) == false);
	printf("got vsync rising edge\n");

	printf("dma_channel_start\n");
	dma_channel_start(config->dma_channel);
	printf("dma_channel_wait_for_finish_blocking\n");
	dma_channel_wait_for_finish_blocking(config->dma_channel);
	printf("capture frame end\n");
}

void ov2640_reg_write(struct ov2640_config *config, uint8_t reg, uint8_t value) {
	uint8_t data[] = {reg, value};
	i2c_write_blocking(config->sccb, OV2640_ADDR, data, sizeof(data), false);
}

uint8_t ov2640_reg_read(struct ov2640_config *config, uint8_t reg) {
	i2c_write_blocking(config->sccb, OV2640_ADDR, &reg, 1, false);

	uint8_t value;
	int ret = i2c_read_blocking(config->sccb, OV2640_ADDR, &value, 1, false);
	if (ret < 0) {
		printf("NACK on write (addr phase)\n");
		return 0xFF;
	}

	return value;
}

void ov2640_regs_write(struct ov2640_config *config, const uint8_t (*regs_list)[2]) {
	while (1) {
		uint8_t reg = (*regs_list)[0];
		uint8_t value = (*regs_list)[1];
		
		// printf("regs_list %d\n", (*regs_list)[0]);
		if (reg == 0x00 && value == 0x00) {
			break;
		}

		ov2640_reg_write(config, reg, value);

		regs_list++;
	}
}

#include <stdio.h>
#include "pico/stdlib.h"
#include "ov2640.h"

// const int PIN_LED = 25;

// const int PIN_CAM_SIOC = 5; // I2C0 SCL
// const int PIN_CAM_SIOD = 4; // I2C0 SDA
// const int PIN_CAM_RESETB = 2;
// const int PIN_CAM_XCLK = 3;
// const int PIN_CAM_VSYNC = 16;
// const int PIN_CAM_Y2_PIO_BASE = 6;

const int PIN_CAM_SIOC = 15;   // I2C0 SCL
const int PIN_CAM_SIOD = 14;   // I2C0 SDA
const int PIN_CAM_RESETB = 13 ;
const int PIN_CAM_XCLK = 8;
const int PIN_CAM_VSYNC = 12;
const int PIN_CAM_Y2_PIO_BASE = 0;  // DATA2 is GPIO0, and DATA0–DATA7 are non-contiguous but we'll map in software


const uint8_t CMD_REG_WRITE = 0xAA;
const uint8_t CMD_REG_READ = 0xBB;
const uint8_t CMD_CAPTURE = 0xCC;

uint8_t image_buf[352*288*2];
int main() {
	stdio_init_all();

    for (int i = 0; i < 5; ++i) {
        printf("Hello, world!%d\n", i);
        sleep_ms(1000);
    }
    printf("Booted!\n");

	gpio_init(26);
	gpio_set_dir(26, GPIO_OUT);
	gpio_put(26, 1);  // set GPIO26 HIGH
	
	sleep_ms(1000);

    struct ov2640_config config;
    config.sccb = i2c1;
    config.pin_sioc = PIN_CAM_SIOC;
    config.pin_siod = PIN_CAM_SIOD;

    config.pin_resetb = PIN_CAM_RESETB;
    config.pin_xclk = PIN_CAM_XCLK;
    config.pin_vsync = PIN_CAM_VSYNC;
    config.pin_y2_pio_base = PIN_CAM_Y2_PIO_BASE;

    config.pio = pio0;
    config.pio_sm = 0;

    config.dma_channel = 0;
    config.image_buf = image_buf;
    config.image_buf_size = sizeof(image_buf);

    ov2640_init(&config);
	printf("post OV init !\n");

    ov2640_reg_write(&config, 0xff, 0x01);
    uint8_t midh = ov2640_reg_read(&config, 0x1C);
    uint8_t midl = ov2640_reg_read(&config, 0x1D);
    printf("MIDH = 0x%02x, MIDL = 0x%02x\n", midh, midl);


    // Reading register
    sleep_ms(1000);
    printf("reading register...\n");
    int reg = 0x1C;
    uint8_t value = ov2640_reg_read(&config, (uint8_t)reg);
    printf("reg = 0x%02x, value = 0x%02x\n", reg, value);


    // Reading camera frame
    sleep_ms(1000);

    // printf("capturing frame OV2640\n");
    ov2640_capture_frame(&config);
    // config.image_buf_size
    printf("frame got! size = %d\n", config.image_buf_size);

    for (size_t i = 0; i < config.image_buf_size; ++i) {
        putchar(config.image_buf[i]);
    }



    // while (true) {
    //     int cmd = getchar();  // wait for 1 byte from USB serial

    //     if (cmd == EOF) continue;

    //     gpio_put(PIN_LED, !gpio_get(PIN_LED));

    //     if (cmd == CMD_REG_WRITE) {
    //         int reg = getchar();
    //         int value = getchar();
    //         if (reg == EOF || value == EOF) continue;

    //         ov2640_reg_write(&config, (uint8_t)reg, (uint8_t)value);

    //     } else if (cmd == CMD_REG_READ) {
    //         int reg = getchar();
    //         if (reg == EOF) continue;

    //         uint8_t value = ov2640_reg_read(&config, (uint8_t)reg);
    //         putchar(value);

    //     } else if (cmd == CMD_CAPTURE) {
    //         ov2640_capture_frame(&config);

    //         for (size_t i = 0; i < config.image_buf_size; ++i) {
    //             putchar(config.image_buf[i]);
    //         }
    //     }
    // }

    return 0;
}

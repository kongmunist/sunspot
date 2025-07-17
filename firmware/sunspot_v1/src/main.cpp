#include <stdio.h>
#include "pico/stdlib.h"
#include "ov2640.h"
#include <RadioLib.h>
#include "lora.h"
#include "config.h"

SX1262 radio = new Module(SX1262_NSS, SX1262_DIO1, SX1262_NRST, SX1262_BUSY);
int state;

void setup() {
    state = radio.begin(915.0);
    if (state != RADIOLIB_ERR_NONE) {
        Serial.println("Failed to initialize radio" + state);
        delay(1000);
    }
    // lora.init();

    // config.sccb = i2c0;
    // config.pin_sioc = CAMERA_SCL2;
    // config.pin_siod = CAMERA_SDA2;
    // config.pin_resetb = CAMERA_RST;
    // config.pin_xclk = CAMERA_XCLK;
    // config.pin_vsync = CAMERA_VSYNC;
    // config.pin_y2_pio_base = CAMERA_D0;

    // config.pio = pio0;
    // config.pio_sm = 0;
    // config.dma_channel = 0;
    // config.image_buf = image_buf;
    // config.image_buf_size = sizeof(image_buf);

    // ov2640_init(&config);
}

void loop() {
    Serial.println(state);
    delay(1000);
    // ov2640_capture_frame(&config);
    // lora.send(image_buf, sizeof(image_buf));
    // delay(1000);
}

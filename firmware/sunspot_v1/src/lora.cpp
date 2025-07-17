#include "lora.h"
#include <RadioLib.h>

void Lora::init() {
    const int state = radio.begin(915.0);
    if (state != RADIOLIB_ERR_NONE) {
        Serial.println("Failed to initialize radio" + state);
        while (1);
    }
}

int Lora::send(uint8_t* data, size_t len) {
    return radio.transmit(data, len);
}



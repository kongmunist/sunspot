#ifndef LORA_H
#define LORA_H

#include <Arduino.h>
#include <RadioLib.h>
#include "config.h"

class Lora {
public:
    void init();
    int send(uint8_t* data, size_t len);
private:
    SX1262 radio = new Module(SX1262_NSS, SX1262_DIO1, SX1262_NRST, SX1262_BUSY);
};



#endif

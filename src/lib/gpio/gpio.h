#ifndef GPIO_H
#define GPIO_H

#include <Arduino.h>

class GPIOPin {
    public:
    GPIOPin(int num, int mode) : pin(num), mode(mode) {
        pinMode(pin, mode);
    };
    private:
        int pin;
        uint8_t mode;
};

#endif // GPIO_H
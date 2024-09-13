#ifndef GPIO_H
#define GPIO_H

#include <Arduino.h>

class GPIOPin {
    public:
    GPIOPin(uint8_t num, uint8_t mode) : pin(num), mode(mode) {
        pinMode(pin, mode);
    };

    void write(int val);

    // Getters
    uint8_t get_pin();
    uint8_t get_mode();

    private:
        uint8_t pin;
        uint8_t mode;
};

#endif // GPIO_H
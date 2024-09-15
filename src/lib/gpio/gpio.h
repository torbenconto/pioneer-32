#ifndef GPIO_H
#define GPIO_H

#include <Arduino.h>

class GPIOPin {
    public:
    GPIOPin(uint8_t pin, uint8_t mode) : _pin(pin), _mode(mode) {
        pinMode(pin, mode);
    };

    void writeAnalog(int val);
    void writeDigital(uint8_t val);

    private:
        uint8_t _pin;
        uint8_t _mode;
};

#endif // GPIO_H
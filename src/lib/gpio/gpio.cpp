#include "gpio.h"
#include <Arduino.h>

// writeAnalog analogWrites int val to GPIOPin::_pin
void GPIOPin::writeAnalog(int val) {
    analogWrite(_pin, val);
};

// writeDigital digitalWrites uint8_t val to GPIOPin::_pin
void GPIOPin::writeDigital(uint8_t val) {
    digitalWrite(_pin, val);
}
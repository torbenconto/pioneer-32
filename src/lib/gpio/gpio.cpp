#include "gpio.h"

// write analogWrites int val to GPIOPin::pin
void GPIOPin::write(int val) {
    analogWrite(GPIOPin::pin, val);
};
#ifndef MOTOR_H
#define MOTOR_H

#include "gpio.h"

class Motor {
    public: 
        Motor(GPIOPin *pin) : pin(pin) {};
    private:
        GPIOPin *pin;

}

#endif // MOTOR_H
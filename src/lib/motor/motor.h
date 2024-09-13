#ifndef MOTOR_H
#define MOTOR_H

#include "gpio.h"

class Motor {
    public: 
        Motor(GPIOPin *pin) : pin(pin) {};

        enum direction {
            clockwise,
            counterclockwise
        };
        
        void drive(direction d, int speed);
    private:
        GPIOPin *pin;

}

#endif // MOTOR_H
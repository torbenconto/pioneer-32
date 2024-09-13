#ifndef MOTOR_H
#define MOTOR_H

#include "gpio.h"

class Motor {
    public: 
        Motor(GPIOPin *pinA, GPIOPin *pinB, GPIOPin *pwm ) : pinA(pinA), pinB(pinB), pwm(pwm) {};

        enum direction {
            clockwise,
            counterclockwise
        };
        
        void drive(direction d, int speed);
    private:
        GPIOPin *pinA;
        GPIOPin *pinB;
        GPIOPin *pwm;

}

#endif // MOTOR_H
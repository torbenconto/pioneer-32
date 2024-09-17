#ifndef MOTOR_H
#define MOTOR_H

#include "gpio.h"

enum direction {
    clockwise,
    counterclockwise
};

class Motor {
    public: 
        Motor(GPIOPin* pinA, GPIOPin* pinB, GPIOPin* pwm) : _pinA(pinA), _pinB(pinB), _pwm(pwm) {};
        
        void drive(direction turnDirection, int speed);
        void stop();
        void brake();
    private:
        // Store references because its a class not like a uint8_t or something that can be copied without a large performance impact
        GPIOPin* _pinA;
        GPIOPin* _pinB;
        GPIOPin* _pwm;

};

#endif // MOTOR_H
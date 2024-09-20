#ifndef MOTOR_H
#define MOTOR_H

#include "gpio.h"
#include <Arduino.h>

enum direction {
    clockwise,
    counterclockwise
};

class Motor {
    public: 
        Motor(uint8_t pinA, uint8_t pinB, uint8_t PWM, uint8_t STBY) : _pinA(pinA, OUTPUT), _pinB(pinB, OUTPUT), _PWM(PWM, OUTPUT), _STBY(STBY, OUTPUT) {
            _STBY.writeDigital(HIGH);
        };
        
        void drive(direction turnDirection, int speed);
        void stop();
        void brake();
    private:
        // Store references because its a class not like a uint8_t or something that can be copied without a large performance impact
        GPIOPin _pinA;
        GPIOPin _pinB;
        GPIOPin _PWM;
        GPIOPin _STBY;

};

#endif // MOTOR_H
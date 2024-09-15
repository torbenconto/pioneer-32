#include "motor.h"

/*
  This Drives a motor in a specific direction, at a specified speed:
    - turnDirection: member of "direction" enum ---> direction.clockwise or direction.counterclockwise
    - speed: 0 to 255 ---> 0 = stop / 255 = fast
*/
void Motor::drive(direction turnDirection, int speed) {
    // Clamp speed value
    speed = max(0, min(255, speed));  

    // Control direction based on turnDirection (obviously. Also why is this a switch? There are only two turn directions)
    switch (turnDirection) {
        case direction::clockwise:
            _pinA->writeDigital(HIGH);
            _pinB->writeDigital(LOW);
            break;

        case direction::counterclockwise:
            _pinA->writeDigital(LOW);
            _pinB->writeDigital(HIGH);
            break;

        default:
            // This should never ever be reached. Why is it here? I don't know.
            return;
    }

    // Set motor speed with PWM
    _pwm->writeAnalog(speed);
}

// Turns off both PinA and PinB to stop the motor
void Motor::stop(){
    _pinA->writeDigital(LOW);
    _pinB->writeDigital(LOW);
}
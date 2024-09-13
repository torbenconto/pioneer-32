#include "motor.h"

/*
  This Drives a motor in a specific direction, at a specified speed:
    - turnDirection: member of "direction" enum ---> direction.clockwise or direction.counterclockwise
    - speed: 0 to 255 ---> 0 = stop / 255 = fast
*/
void Motor::drive(direction turnDirection, int speed) {
    // TODO: fix this horrific code
    if (turnDirection == direction::clockwise) {
        Motor::pinA->write(HIGH);
        Motor::pinB->write(LOW);
    }

    if (turnDirection == direction::counterclockwise) {
        Motor::pinA->write(LOW);
        Motor::pinB->write(HIGH);
    }

    Motor::pwm->write(speed);
}
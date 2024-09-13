#include <Arduino.h>
#include "gpio.h"
#include "motor.h"
#include "pins.h"


/*
  Pin initializations, see lib/gpio for GPIOPin class
  Definitons are handled in include/pins.h
h
  Driver used: https://www.sparkfun.com/products/14450
  Pinout: https://learn.sparkfun.com/tutorials/tb6612fng-hookup-guide/all
*/

// Directonal control pins for motor A
GPIOPin p_AIN1(AIN1, OUTPUT);
GPIOPin p_AIN2(AIN2, OUTPUT);

// PWM signal pin for speed control on motor A
GPIOPin p_PWMA(PWMA, OUTPUT);

// Directonal control pins for motor B
GPIOPin p_BIN1(BIN1, OUTPUT);
GPIOPin p_BIN2(BIN2, OUTPUT);

// PWM signal pin for speed control on motor B
GPIOPin p_PWMB(PWMB, OUTPUT);

/*
  Standby pin
  Allows the H-bridges to work when high (has a pulldown resistor so it must actively pulled high)
  Pull STBY on HIGH during normal operation
*/
GPIOPin p_STBY(STBY, OUTPUT);


void setup() {
}

void loop() {
}
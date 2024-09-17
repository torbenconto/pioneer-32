#include <Arduino.h>
#include "gpio.h"
#include "motor.h"
#include "pins.h"
#include "controller.h"

/*
  Pin initializations, see lib/gpio for GPIOPin class
  Definitons are handled in include/pins.h

  Driver module used: https://www.sparkfun.com/products/14450
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

// MotorA instance
Motor motorA(&p_AIN1, &p_AIN2, &p_PWMA);
Motor motorB(&p_BIN1, &p_BIN2, &p_PWMB);

// Controller params
const float JOYSTICK_MIDDLE = 0.5;
const float DEAD_ZONE = 0.05; 

// Controller instance
Controller xboxController("f4:6a:d7:12:31:be");

void setup() {
  // Disable standby
  p_STBY.writeDigital(HIGH);

  xboxController.begin();
}

void loop() {
  xboxController.onLoop();

  if (xboxController.isConnected()) {
    if (xboxController.isWaitingForFirstNotification()) {
      uint16_t joystickMax = XboxControllerNotificationParser::maxJoy; // Maximum possible joystick value
      float joyLVert = (float)xboxController.xboxNotif.joyLVert / joystickMax; // Normalize joystick vertical position to [0, 1]

      if (joyLVert > JOYSTICK_MIDDLE + DEAD_ZONE) {
          // Joystick is pushed beyond the positive dead zone
          // Map joystick value to the motor speed range
          // joyLVert - (JOYSTICK_MIDDLE + DEAD_ZONE) calculates how far the joystick is beyond the positive dead zone
          // 1.0 - JOYSTICK_MIDDLE - DEAD_ZONE is the maximum range where the joystick can still affect the motor
          float speed = (joyLVert - (JOYSTICK_MIDDLE + DEAD_ZONE)) / (1.0 - JOYSTICK_MIDDLE - DEAD_ZONE);
          // Scale speed to motor range (0 to 255)
          motorA.drive(clockwise, 255 * speed);
      } else if (joyLVert < JOYSTICK_MIDDLE - DEAD_ZONE) {
          // Joystick is pushed beyond the negative dead zone
          // Map joystick value to the motor speed range
          // (JOYSTICK_MIDDLE - DEAD_ZONE) - joyLVert calculates how far the joystick is below the negative dead zone
          // JOYSTICK_MIDDLE - DEAD_ZONE is the maximum range where the joystick can still affect the motor
          float speed = ((JOYSTICK_MIDDLE - DEAD_ZONE) - joyLVert) / (JOYSTICK_MIDDLE - DEAD_ZONE);
          // Scale speed to motor range (0 to 255)
          motorA.drive(counterclockwise, 255 * speed);
      } else {
          // Joystick is within the dead zone
          // No significant movement detected, so brake the motor
          motorA.brake();
      }
    }
  } else {
    if (xboxController.getCountFailedConnection() > 2) {
      ESP.restart();
    }
  }
}
#include <Arduino.h>
#include "gpio.h"
#include "motor.h"
#include "pins.h"
#include "controller.h"

/*
  Pin initializations, see lib/gpio for GPIOPin class
  Definitions are handled in include/pins.h

  Driver module used: https://www.sparkfun.com/products/14450
  Pinout: https://learn.sparkfun.com/tutorials/tb6612fng-hookup-guide/all
*/

// Directional control pins for motor A
GPIOPin p_AIN1(AIN1, OUTPUT);
GPIOPin p_AIN2(AIN2, OUTPUT);

// PWM signal pin for speed control on motor A
GPIOPin p_PWMA(PWMA, OUTPUT);

// Directional control pins for motor B
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

// MotorA and MotorB instances
Motor motorA(&p_AIN1, &p_AIN2, &p_PWMA);
Motor motorB(&p_BIN1, &p_BIN2, &p_PWMB);

// Controller parameters
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
    if (!xboxController.isWaitingForFirstNotification()) {
      uint16_t joystickMax = XboxControllerNotificationParser::maxJoy; // Maximum possible joystick value

      // Left joystick vertical position for motor A
      float joyLVert = (float)xboxController.xboxNotif.joyLVert / joystickMax; // Normalize joystick value to [0, 1]

      // Right joystick vertical position for motor B
      float joyRVert = (float)xboxController.xboxNotif.joyRVert / joystickMax; // Normalize joystick value to [0, 1]
      
      // Motor A control logic (left joystick)
      if (joyLVert > JOYSTICK_MIDDLE + DEAD_ZONE) {
        float speed = (joyLVert - (JOYSTICK_MIDDLE + DEAD_ZONE)) / (1.0 - JOYSTICK_MIDDLE - DEAD_ZONE);
        motorA.drive(clockwise, 255 * speed);
      } else if (joyLVert < JOYSTICK_MIDDLE - DEAD_ZONE) {
        float speed = ((JOYSTICK_MIDDLE - DEAD_ZONE) - joyLVert) / (JOYSTICK_MIDDLE - DEAD_ZONE);
        motorA.drive(counterclockwise, 255 * speed);
      } else {
        motorA.brake();
      }

      // Motor B control logic (right joystick)
      if (joyRVert > JOYSTICK_MIDDLE + DEAD_ZONE) {
        float speed = (joyRVert - (JOYSTICK_MIDDLE + DEAD_ZONE)) / (1.0 - JOYSTICK_MIDDLE - DEAD_ZONE);
        motorB.drive(clockwise, 255 * speed);
      } else if (joyRVert < JOYSTICK_MIDDLE - DEAD_ZONE) {
        float speed = ((JOYSTICK_MIDDLE - DEAD_ZONE) - joyRVert) / (JOYSTICK_MIDDLE - DEAD_ZONE);
        motorB.drive(counterclockwise, 255 * speed);
      } else {
        motorB.brake();
      }
    }
  } else {
    if (xboxController.getCountFailedConnection() > 2) {
      ESP.restart();
    }
  }
  
  // DO NOT REMOVE THIS LITERALLY EVER!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
  delay(10);
}

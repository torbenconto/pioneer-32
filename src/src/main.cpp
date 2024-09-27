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

Motor WheelMotorA(MotorControllerA.AIN1, MotorControllerA.AIN2, MotorControllerA.PWMA, MotorControllerA.STBY);
Motor WheelMotorB(MotorControllerA.BIN1, MotorControllerA.BIN2, MotorControllerA.PWMB, MotorControllerA.STBY);

Motor ClawMotorA(MotorControllerB.AIN1, MotorControllerB.AIN2, MotorControllerB.PWMA, MotorControllerB.STBY);
Motor ClawMotorB(MotorControllerB.BIN1, MotorControllerB.BIN2, MotorControllerB.PWMB, MotorControllerB.STBY);

// Controller parameters
const float JOYSTICK_MIDDLE = 0.5;
const float DEAD_ZONE = 0.05; 

// Controller instance
Controller xboxController("f4:6a:d7:12:31:be");

void setup() {
  xboxController.begin();
}

void loop() {
  xboxController.onLoop();

  if (xboxController.isConnected()) {
    if (!xboxController.isWaitingForFirstNotification()) {
      uint16_t joystickMax = XboxControllerNotificationParser::maxJoy; // Maximum possible joystick value

      float joyLVert = (float)xboxController.xboxNotif.joyLVert / joystickMax; // Normalize joystick value to [0, 1]

      float joyRVert = (float)xboxController.xboxNotif.joyRVert / joystickMax; // Normalize joystick value to [0, 1]

      float rTrigger = (float)xboxController.xboxNotif.trigRT / 1024; // Normalize trigger value to [0, 1]

      float lTrigger = (float)xboxController.xboxNotif.trigLT / 1024; // Normalize trigger value to [0, 1]

      bool rb = xboxController.xboxNotif.btnRB;
      bool lb = xboxController.xboxNotif.btnLB;
      
      // Motor A control logic (left joystick)
      if (joyLVert > JOYSTICK_MIDDLE + DEAD_ZONE) {
        float speed = (joyLVert - (JOYSTICK_MIDDLE + DEAD_ZONE)) / (1.0 - JOYSTICK_MIDDLE - DEAD_ZONE);
        WheelMotorA.drive(clockwise, 255 * speed);
      } else if (joyLVert < JOYSTICK_MIDDLE - DEAD_ZONE) {
        float speed = ((JOYSTICK_MIDDLE - DEAD_ZONE) - joyLVert) / (JOYSTICK_MIDDLE - DEAD_ZONE);
        WheelMotorA.drive(counterclockwise, 255 * speed);
      } else {
        WheelMotorA.brake();
      }

      // Motor B control logic (right joystick)
      if (joyRVert > JOYSTICK_MIDDLE + DEAD_ZONE) {
        float speed = (joyRVert - (JOYSTICK_MIDDLE + DEAD_ZONE)) / (1.0 - JOYSTICK_MIDDLE - DEAD_ZONE);
        WheelMotorB.drive(clockwise, 255 * speed);
      } else if (joyRVert < JOYSTICK_MIDDLE - DEAD_ZONE) {
        float speed = ((JOYSTICK_MIDDLE - DEAD_ZONE) - joyRVert) / (JOYSTICK_MIDDLE - DEAD_ZONE);
        WheelMotorB.drive(counterclockwise, 255 * speed);
      } else {
        WheelMotorB.brake();
      }

      // Triggers control ClawMotorA (move arm up and down)
      if (rTrigger > 0) {
          // Rtrigger moves up
          ClawMotorA.drive(clockwise, 255 * rTrigger);
      } else if (lTrigger > 0) {
          ClawMotorA.drive(counterclockwise, 255 * lTrigger);
      } else {
          ClawMotorA.brake();
      }

      // Bumpers control ClawMotorB (open and close jaws)
      if (rb) {
        ClawMotorB.drive(clockwise, 255);
      } else if (lb) {
        ClawMotorB.drive(counterclockwise, 255);
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

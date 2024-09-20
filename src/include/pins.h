#pragma once

struct MotorController {
    int AIN1;
    int AIN2;
    int PWMA;

    int BIN1;
    int BIN2;
    int PWMB;

    int STBY;
};

MotorController MotorControllerA{
    .AIN1 = 14,
    .AIN2 = 27,
    .PWMA = 12,

    .BIN1 = 25,
    .BIN2 = 33,
    .PWMB = 32,

    .STBY = 26,
};

MotorController MotorControllerB{
    .AIN1 = 4,
    .AIN2 = 2,
    .PWMA = 15,

    .BIN1 = 18,
    .BIN2 = 19,
    .PWMB = 21,

    .STBY = 5,
};

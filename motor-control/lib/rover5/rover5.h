#pragma once

#include "Arduino.h"
#include "motor.h"

const int NUM_WHEELS = 4;

enum wheel_type
{
    WHEEL_FR = 0,
    WHEEL_FL,
    WHEEL_RR,
    WHEEL_RL,
};

struct wheel_config
{
    uint8_t pwm_pin;
    uint8_t dir_pin;
    uint8_t enc_pin;
};

struct rover_config
{
    wheel_config wheels[NUM_WHEELS];
};

class rover5
{
public:
    rover5(const rover_config *config);
    void update();
private:
    const rover_config *_config;
    motor _motors[NUM_WHEELS];
    static void enc0_isr();
    static void enc1_isr();
    static void enc2_isr();
    static void enc3_isr();
};

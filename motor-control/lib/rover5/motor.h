#pragma once
#include "Arduino.h"

enum motor_direction
{
    DIR_FWD = 0,
    DIR_REV = 1,
};

class motor
{
public:
    motor() { }
    int init(int id, uint8_t pwm_pin, uint8_t dir_pin);
    void set_direction(motor_direction dir);
    void set_speed(uint8_t speed);
    void handle_enc_interrupt();
    void update();
private:
    void set_power(int value);
    uint8_t _pwm_pin, _dir_pin;
    volatile int _enc_count;
    int _prev_count;
    uint32_t _prev_us;
    //volatile int led;
    int _id;
    uint8_t _set_speed;
};

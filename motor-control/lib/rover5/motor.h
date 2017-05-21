#pragma once
#include "Arduino.h"
#include "PID_v1.h"

enum motor_direction
{
    DIR_FWD = 0,
    DIR_REV = 1,
};

class motor
{
public:
    motor();
    int init(int id, uint8_t pwm_pin, uint8_t dir_pin);
    void set_direction(motor_direction dir);
    void set_speed(float rpm);
    void handle_enc_interrupt();
    void update();
private:
    void print_state();
    uint8_t _pwm_pin, _dir_pin;
    volatile uint32_t _enc_count;
    uint32_t _prev_count;
    uint32_t _prev_us;
    uint16_t _last_print_ms;
    int _id;
    double _cps_setpoint;
    double _cps_input;
    double _pwm_output;
    double _kp, _ki, _kd;
    float _update_rate;
    PID _ctrl;
};

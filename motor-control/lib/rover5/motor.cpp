#include "motor.h"

/* 1000 pulses per 3 revolutions => 1rpm = 1000/(3*60) */
const float CPS_PER_RPM = 5.555555f;

int motor::init(int id, uint8_t pwm_pin, uint8_t dir_pin)
{
    _id = id;
    _pwm_pin = pwm_pin;
    _dir_pin = dir_pin;
    pinMode(_pwm_pin, OUTPUT);
    pinMode(_dir_pin, OUTPUT);
    _enc_count = 0;
    return 0;
}

void motor::set_direction(motor_direction dir)
{
    digitalWrite(_dir_pin, dir);
}

void motor::set_power(int value)
{
    analogWrite(_pwm_pin, value);
}

void motor::set_speed(uint8_t speed)
{
    _set_speed = speed;
}


void motor::update()
{
    uint32_t cur_us = micros();
    if (_prev_count != _enc_count) {
        float dt = (cur_us - _prev_us) / 1000000.0f;
        float cps = (_enc_count - _prev_count) / dt;
        Serial.print("Motor ");
        Serial.print(_id);
        Serial.print(": count = ");
        Serial.print(_enc_count);
        Serial.print(", cps = ");
        Serial.print(cps);
        Serial.print("\n");
        _prev_count = _enc_count;

    }
    _prev_us = cur_us;
    //set_power(250);
}

void motor::handle_enc_interrupt()
{
    _enc_count++;
}

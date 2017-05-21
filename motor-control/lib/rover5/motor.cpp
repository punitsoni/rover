#include "motor.h"

/* 1000 pulses per 3 revolutions => 1rpm = 1000/(3*60) cps */
const float CPS_PER_RPM = 5.555555f;

const float DEFAULT_KP = 0.84039;
const float DEFAULT_KI = 3.0;
const float DEFAULT_KD = 0.2;

const uint32_t SAMPLE_PERIOD = 50000;

inline float count_to_rpm(word count)
{
    return (count * 1000) / (180 * SAMPLE_PERIOD);
}

motor::motor():
    _kp(DEFAULT_KP),
    _ki(DEFAULT_KI),
    _kd(DEFAULT_KD),
    _ctrl(&_cps_input, &_pwm_output, &_cps_setpoint, _kp, _ki, _kd, DIRECT)
{
}

int motor::init(int id, uint8_t pwm_pin, uint8_t dir_pin)
{
    _id = id;
    _pwm_pin = pwm_pin;
    _dir_pin = dir_pin;
    pinMode(_pwm_pin, OUTPUT);
    pinMode(_dir_pin, OUTPUT);
    _enc_count = 0;
    _cps_setpoint = 0;
    _ctrl.SetSampleTime(50);
    _ctrl.SetMode(AUTOMATIC);
    return 0;
}

void motor::set_direction(motor_direction dir)
{
    digitalWrite(_dir_pin, dir);
}

void motor::set_speed(float cps)
{
    _cps_setpoint = cps;
}

void motor::update()
{
    if (_id != 0) {
        return;
    }
    uint32_t cur_us = micros();
    if (cur_us - _prev_us >= SAMPLE_PERIOD) {
        uint32_t dt = cur_us - _prev_us;
        noInterrupts();
        uint32_t cur_count = _enc_count;
        _enc_count = 0;
        interrupts();
        _cps_input = (1000000.0f * cur_count) / dt;
        _ctrl.Compute();
        #if 0
        if (cur_us < 1000000) {
            _pwm_output = 0;
        } else if (cur_us < 6000000) {
            _pwm_output = 255;
        } else {
            _pwm_output = 0;
        }
        #endif
        analogWrite(_pwm_pin, _pwm_output);
        //_update_rate = 1000000.0/dt;
        _prev_count = cur_count;
        _prev_us = cur_us;
        #if 0
        Serial.print(cur_us);
        Serial.print(",");
        Serial.print(_pwm_output);
        Serial.print(",");
        Serial.print(_cps_input);
        Serial.print("\n");
        #endif
    }

    if (millis() - _last_print_ms > 500)
    {
        //print_state();
    }
}

void motor::print_state()
{
    #if 1
    //Serial.print("Motor ");
    //Serial.print(_id);
    //Serial.print(": cps_setpoint = ");
    //Serial.print(_cps_setpoint);
    Serial.print("cps = ");
    Serial.print(_cps_input);
    //Serial.print(", rpm = ");
    //Serial.print(_cps_input * (180.0/1000));
    //Serial.print(", N = ");
    //Serial.print(_prev_count);
    Serial.print(", pwm = ");
    Serial.print(_pwm_output);
    #endif
    Serial.print("\n");
}

void motor::handle_enc_interrupt()
{
    _enc_count++;
}

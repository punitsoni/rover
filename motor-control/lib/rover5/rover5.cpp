#include "rover5.h"
//#define LIBCALL_ENABLEINTERRUPT
#include "EnableInterrupt.h"

static rover5 *inst;

rover5::rover5(const rover_config *config):
    _config(config)
{
    for (int i=0; i<NUM_WHEELS; i++) {
        _motors[i].init(i, _config->wheels[i].pwm_pin,
                        _config->wheels[i].dir_pin);
        pinMode(_config->wheels[i].enc_pin, INPUT);
    }
    enableInterrupt(_config->wheels[0].enc_pin, &rover5::enc0_isr, CHANGE);
    enableInterrupt(_config->wheels[1].enc_pin, &rover5::enc1_isr, CHANGE);
    enableInterrupt(_config->wheels[2].enc_pin, &rover5::enc2_isr, CHANGE);
    enableInterrupt(_config->wheels[3].enc_pin, &rover5::enc3_isr, CHANGE);
    inst = this;
    _motors[0].set_speed(300);
}

void rover5::update()
{
    for (int i=0; i<NUM_WHEELS; i++) {
        _motors[i].update();
    }
}

void rover5::enc0_isr()
{
    inst->_motors[0].handle_enc_interrupt();
}

void rover5::enc1_isr()
{
    inst->_motors[1].handle_enc_interrupt();
}

void rover5::enc2_isr()
{
    inst->_motors[2].handle_enc_interrupt();
}

void rover5::enc3_isr()
{
    inst->_motors[3].handle_enc_interrupt();
}

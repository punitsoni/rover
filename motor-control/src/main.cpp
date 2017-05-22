#include "Arduino.h"
#include "board_config.h"
#include "rover5.h"

#include "CmdMessenger.h"

static const rover_config config = {
    {
        {
            PIN_MOTOR_FR_PWM,
            PIN_MOTOR_FR_DIR,
            PIN_ENC_FR_INT
        },
        {
            PIN_MOTOR_FR_PWM,
            PIN_MOTOR_FR_DIR,
            PIN_ENC_FL_INT
        },
        {
            PIN_MOTOR_FR_PWM,
            PIN_MOTOR_FR_DIR,
            PIN_ENC_RR_INT
        },
        {
            PIN_MOTOR_FR_PWM,
            PIN_MOTOR_FR_DIR,
            PIN_ENC_RL_INT
        },
    }
};

rover5 rover(&config);

CmdMessenger msgr = CmdMessenger(Serial);

enum command_type
{
    CMD_ACK,
    CMD_MSG,
    CMD_ERROR,
    CMD_PING,
    CMD_SET_SPEED,
};

void on_unknown_cmd()
{
    msgr.sendCmd(CMD_ERROR, "Invalid Command");
    Serial.print("ERROR\n");
}

void on_ping()
{
    msgr.sendCmd(CMD_ACK, CMD_PING);
}

void on_set_speed()
{
    int speed = msgr.readInt16Arg();
    msgr.sendCmd(CMD_ACK, CMD_SET_SPEED);
    msgr.sendCmd(CMD_MSG, "speed set to " + String(speed));
}

void msgr_attach_callbacks()
{
    msgr.attach(on_unknown_cmd);
    msgr.attach(CMD_PING, on_ping);
    msgr.attach(CMD_SET_SPEED, on_set_speed);
}

void serialEvent()
{
    msgr.feedinSerialData();
}

void setup()
{
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
    msgr.printLfCr();
    msgr_attach_callbacks();
    msgr.sendCmd(CMD_MSG, "__ready_for_business__");
}

void loop()
{
    //rover.update();
    //msgr.feedinSerialData();
}

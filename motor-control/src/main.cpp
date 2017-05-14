/**
 * Blink
 *
 * Turns on an LED on for one second,
 * then off for one second, repeatedly.
 */
#include "Arduino.h"

#include "motor.h"
#include "board_config.h"
#include "rover5.h"
//#include "EnableInterrupt.h"

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

uint8_t pwm_m1 = 3;

rover5 rover(&config);

void setup()
{
  // initialize LED digital pin as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  Serial.print("-- Rover5 Controller --\n");
}

void loop()
{
    rover.update();
    delay(50);
    #if 0
    // turn the LED on (HIGH is the voltage level)
    digitalWrite(LED_BUILTIN, HIGH);

    analogWrite(pwm_m1, 50);

    // wait for a second
    delay(1000);

    // turn the LED off by making the voltage LOW
    digitalWrite(LED_BUILTIN, LOW);

    analogWrite(pwm_m1, 200);

    // wait for a second
delay(1000);
  #endif
}

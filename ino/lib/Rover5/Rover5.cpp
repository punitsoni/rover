#include "Rover5.h"
#include "PinChangeInt.h"

#define SERIAL_OUT_PERIOD_MS       500
#define ENC_MEASURE_PULSE_COUNT    20
#define MAX_SPEED_ENC_FREQ         310
#define ENC_STALL_DETECT_TIME_MS   1000


Rover *rSingleton;

Rover::Rover(R5BoardConfig bCfg)
{
    int i;
    this->bConfig = bCfg;
    for(i=0; i<NUM_MOTORS; i++) {
        mPid[i] = new PID(&pidInfo[i].input, &pidInfo[i].output,
            &pidInfo[i].setPoint, pidInfo[i].Kp, pidInfo[i].Ki,
                pidInfo[i].Kd, DIRECT);
        mPid[i]->SetMode(AUTOMATIC);
    }
    initMotors();
    initEncoders();
    rSingleton = this;
}

void Rover::initMotors()
{
    for(int i=0; i<4; i++) {
        pinMode(bConfig.pinPwm[i], OUTPUT);
        pinMode(bConfig.pinDir[i], OUTPUT);
    }
}

void Rover::initEncoders()
{
    for(int i=0; i<NUM_ENCODERS; i++) {
        pinMode(bConfig.pinEnc[i], INPUT);
    }
    PCintPort::attachInterrupt(bConfig.pinEnc[0], &Rover::enc0isr, CHANGE);
    PCintPort::attachInterrupt(bConfig.pinEnc[1], &Rover::enc1isr, CHANGE);
    PCintPort::attachInterrupt(bConfig.pinEnc[2], &Rover::enc2isr, CHANGE);
    PCintPort::attachInterrupt(bConfig.pinEnc[3], &Rover::enc3isr, CHANGE);
}

void Rover::handleEncoder(int i)
{
	unsigned long t1 = micros();
	if (eInfo[i].count == ENC_MEASURE_PULSE_COUNT) {
		eInfo[i].curDiff = t1 - eInfo[i].t0;
		eInfo[i].t0 = t1;
		eInfo[i].count = 0;
	}
	eInfo[i].lastChange = t1;
	eInfo[i].count ++;
}

void Rover::enc0isr()
{
    rSingleton->handleEncoder(0);
}

void Rover::enc1isr()
{
    rSingleton->handleEncoder(1);
}

void Rover::enc2isr()
{
    rSingleton->handleEncoder(2);
}
void Rover::enc3isr()
{
    rSingleton->handleEncoder(3);
}

void Rover::setSpeed(int *s) {
	for(int i=0; i<4; i++) {
		curDir[i] = (s[i] > 0) ? DIR_FWD : DIR_BACK;
		reqSpeed[i] = (s[i] > 0) ? s[i] : -s[i];
		digitalWrite(bConfig.pinDir[i], curDir[i]);
	}
}

void Rover::update()
{
    unsigned long curFreq;
    for(int i=0; i<4; i++) {
        pidInfo[i].setPoint = abs(reqSpeed[i]);
        if(micros() - eInfo[i].lastChange > ENC_STALL_DETECT_TIME_MS*1000 ||
            running[i] == 0) {
            curSpeed[i] = 0;
            running[i] = 0;
        } else {
            noInterrupts();
            curFreq = (1000000 * ENC_MEASURE_PULSE_COUNT)/eInfo[i].curDiff;
            interrupts();
            curSpeed[i] = (curFreq * 100) / MAX_SPEED_ENC_FREQ;
        }

        pidInfo[i].input = curSpeed[i];
        if (pidInfo[i].input > 1000)
            pidInfo[i].input  = 0;
        if(pidInfo[i].setPoint == 0)
            pidInfo[i].input  = 0;
        else
            mPid[i]->Compute();
        running[i] = ((int)pidInfo[i].output > 0);
        //pid_output[i] = (abs(req_speed[i]) * 255) / 100;
        analogWrite(bConfig.pinPwm[i], (int) pidInfo[i].output);
    }
}

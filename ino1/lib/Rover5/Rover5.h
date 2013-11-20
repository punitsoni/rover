#ifndef ROVER5_H
#define ROVER5_H

#include "Arduino.h"
#include "PID_v1.h"

#define NUM_MOTORS 		4
#define NUM_ENCODERS 	4

#define DIR_FWD   1
#define DIR_BACK  0

template<class T> inline Print &operator <<(Print &obj, T arg)
{
	obj.print(arg);
	return obj;
}

typedef struct _EncoderInfo {
	unsigned long curDiff;
	unsigned long t0;
	unsigned long lastChange;
	unsigned int count;
} EncoderInfo;

typedef struct _R5BoardConfig {
	uint8_t pinPwm[NUM_MOTORS];
	uint8_t pinDir[NUM_MOTORS];
	uint8_t pinEnc[NUM_ENCODERS];
	//uint8_t chLeft[2];
	//uint8_t chRight[2];
	uint8_t pinDirLeft;
	uint8_t pinDirRight;
} R5BoardConfig;

typedef struct _PIDInfo {
	double input;
	double output;
	double setPoint;
	double Kp, Ki, Kd;
} PIDInfo;

typedef struct _RoverStatus {
	int curSpeed[4];
	PIDInfo *pidInfo;
} RoverStatus;

class Rover
{
	private:
		PIDInfo pidInfo[NUM_MOTORS];
		PID *mPid[NUM_MOTORS];
		EncoderInfo eInfo[NUM_ENCODERS];
		R5BoardConfig bConfig;
		int reqSpeed[4];
		uint8_t running[4];
		int curSpeed[4];
		uint8_t curDir[4];

		void initMotors();
		void initEncoders();
		void handleEncoder(int i);
		static void enc0isr();
		static void enc1isr();
		static void enc2isr();
		static void enc3isr();
		void updateControlLoop();

	public:
		Rover(R5BoardConfig);
		void update();
		void setSpeed(int *);
		inline void getCurSpeed(int *s) {
			for(int i=0; i<4; i++)
				s[i] = (curDir[i] == DIR_FWD) ? curSpeed[i] : -curSpeed[i];
		}
		inline RoverStatus getStatus() {
			RoverStatus r;
			getCurSpeed(r.curSpeed);
			r.pidInfo = this->pidInfo;
			return r;
		}
};

#endif
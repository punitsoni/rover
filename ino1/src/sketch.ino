#include "Rover5.h"
#include "Timer.h"

R5BoardConfig bCfg = {
	{9, 6, 5, 3},
	{7, 4, 7, 4},
	{A0, A1, A2, A3},
	4, 7,
};

Timer t;
Rover rover(bCfg);
int speed[2] = {0, 0};

void setup() {
	pinMode(8, OUTPUT);
	Serial.begin(9600);
	t.every(100, heartBeat, 0);
	t.every(100, roverUpdate, 0);
}

void loop() {
	speed[0] = (analogRead(A7) * 100l) / 1023;
	speed[1] = speed[0];
	rover.setSpeed(speed);
	t.update();
}

void roverUpdate(void *arg)
{
	rover.update();
}

String getRoverStatus()
{
	RoverStatus rs = rover.getStatus();
	String s = String("");
	s = s + " speed = [ " + rs.curSpeed[0] + " " + rs.curSpeed[1] + " " +
		rs.curSpeed[2] + " " + rs.curSpeed[3] + " ]";
	return s;
}

void heartBeat(void *arg) {
	Serial.print("heartbeat: ");
	Serial.print(millis());
	Serial.print(getRoverStatus());
	Serial.println("");
	//digitalWrite(8, HIGH);
	//delay(100);
	//digitalWrite(8, LOW);
}
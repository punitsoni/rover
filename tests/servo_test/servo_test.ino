#include <Servo.h>

Servo servo1;
Servo servo2;

int hAngle=90, vAngle=90;
int hPulse=1500, vPulse=1500;
int pMax=2400, pMin=500;

int hServoPin = 9;
int vServoPin = 10;

void setup() {

  pinMode(hServoPin, OUTPUT);
  pinMode(vServoPin, OUTPUT);

  servo1.attach(hServoPin, pMin, pMax);
  servo2.attach(vServoPin, pMin, pMax);

  //servo1.setMaximumPulse(2000);
  //servo1.setMinimumPulse(700);

  //servo2.attach(15); //analog pin 1
  Serial.begin(9600);
  Serial.println("Ready");
}

void loop()
{
  //sweepUpdate();
  commandUpdate();
  servoUpdate();
}

void printStatus()
{
  
  Serial.print("H = ");
  Serial.print(hAngle);
  Serial.print(", V = ");
  Serial.println(vAngle);
}

void servoUpdate()
{
  //servo1.writeMicroseconds(hPulse);
  servo1.write(hAngle);
  servo2.write(vAngle);
}

void sweepUpdate()
{
  static int dir=1;
  if(hPulse > 2300)
    dir = -1;
  if (hPulse < 400)
    dir = 1;
  
  hPulse += 50*dir;
  printStatus();
  servo1.writeMicroseconds(hPulse);
  //servo2.writeMicros(vPulse);
  delay(100);
}

void commandUpdate() {
  
  while(Serial.available() > 0) {
    switch(Serial.read()) {
      case 'w':
        vAngle -= 10;
        vPulse -= 50;
        break;
      case 's':
        vAngle += 10;
        vPulse += 50;
        break;
      case 'a':
        hAngle += 10;
        hPulse += 50;
        break;
      case 'd':
        hAngle -= 10;
        hPulse -= 50;
        break;
      default:
        break;
    }
    hAngle = constrain(hAngle, 0, 180);
    vAngle = constrain(vAngle, 0, 180);
    printStatus();
  }
} 


#include <Wire.h>

#define SLAVE_ADDRESS 0x11

int number = 0;
int state = 0;

/* state variables */
int lspeed = 0;
int rspeed = 0;

/* commands and parameters */
int CMD_SET = 0;
int CMD_GET = 0;

int P_RV_SPEED = 0;
int P_SERVO_POS = 1;

unsigned long reply = 0;
int reply_idx = 0;

void setup() {
    pinMode(13, OUTPUT);
    Serial.begin(9600);
    // initialize i2c as slave
    Wire.begin(SLAVE_ADDRESS);

    // define callbacks for i2c communication
    Wire.onReceive(receiveDataOnI2C);
    Wire.onRequest(sendDataToI2C);

    Serial.println("Ready!");
}

void loop() {
    delay(100);
}

// callback for received data
void receiveDataOnI2C(int byteCount) {
  int data = 0;
  int i = 0;
  int b;
  digitalWrite(13, HIGH);
    while(Wire.available()) {
        b = Wire.read();
        Serial.print("data received: ");
        Serial.println(b);
        //if(i < 4)
        //  data += b << 8*i;
        data = b;
        i++;
     }
     reply = data;
     digitalWrite(13, LOW);
}

// callback for sending data
void sendDataToI2C() {
  
    //int b = (reply >> (reply_idx*8)) & 0xff;
    int b = reply;
    Wire.write(b);
    reply_idx ++;
    if(reply_idx >= 4)
      reply_idx = 0;
}


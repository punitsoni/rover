#include <Wire.h>

#define SLAVE_ADDRESS 0x11

/* state variables */
int lspeed = 0;
int rspeed = 0;

/* commands and parameters */
enum {
 CMD_SET_LSPEED = 0,
 CMD_SET_RSPEED = 1,
} cmd_t;

int t_step = 0;
int t_cmd = -1;
int reply = -1;

void setup() {
    pinMode(13, OUTPUT);
    Serial.begin(9600);
    Wire.begin(SLAVE_ADDRESS);
    Wire.onReceive(receiveDataOnI2C);
    Wire.onRequest(sendDataToI2C);
    Serial.println("RV Controller Interface: Ready");
}

void loop() {
    delay(100);
}

int processCommand(int cmd, int data) {
  int ret = 255;
  switch(cmd) {
    case CMD_SET_LSPEED:
      lspeed = data;
      Serial.print("lspeed = ");
      Serial.println(lspeed);
      break;
    case CMD_SET_RSPEED:
      rspeed = data;
      Serial.print("rspeed = ");
      Serial.println(rspeed);       
      break;
    default:
      Serial.print("invalid command ");
      Serial.println(cmd);
      return 0;
  }
  return ret;
}

// callback for received data
void receiveDataOnI2C(int byteCount) {
  int cmd, data;
  digitalWrite(13, HIGH);
    while(Wire.available()) {
        int b = Wire.read();
        switch(t_step) {
          case 0:
            cmd = b;
            Serial.print("cmd: ");
            Serial.println(cmd);
            t_step = 1;
            t_cmd = cmd;
            break;
          case 1:
            data = b;
            Serial.print("data: ");
            Serial.println(data);
            reply = processCommand(t_cmd, data);
            t_cmd = -1;
            t_step = 2;
            break;
          default:
            Serial.println("protocol error on receive");
            return;
        }
     }
     digitalWrite(13, LOW);
}

// callback for sending data
void sendDataToI2C() {
    if(t_step != 2) {
      Serial.println("protocol error on send");
      return;
    }
    Serial.print("response: ");
    Serial.println(reply);
    Wire.write(reply);
    t_step = 0;
}


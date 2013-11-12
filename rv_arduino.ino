#include <Timer.h>
#include <PinChangeInt.h>
#include <Wire.h>

#define SLAVE_ADDRESS 0x11

#define sign(x) ((x > 0) ? 1 : -1)

#define DIR_FWD   1
#define DIR_BACK  0

#define LEFT  0
#define RIGHT 1

#define PIN_ENC0_INT A0
#define PIN_ENC1_INT A1
#define PIN_ENC2_INT A2
#define PIN_ENC3_INT A3

#define SPEED_UPDATE_INTERVAL 40
#define SPEED_MAX_PPS         300
#define SPEED_TOLERANCE       5

#define CH_LEFT0   1
#define CH_LEFT1   3
#define CH_RIGHT0  0
#define CH_RIGHT1  2


Timer t;
int lspeed = 0, rspeed = 0;
int prev_lspeed = 0, prev_rspeed = 0;

volatile int pulse_count[4] = {0, 0, 0, 0};
volatile unsigned long change_time[4];
volatile unsigned long pulse_width[4] = {0, 0, 0, 0};
volatile unsigned long pulse_width_h1[4] = {0, 0, 0, 0};
volatile unsigned long pulse_width_h2[4] = {0, 0, 0, 0};


unsigned long  cur_speed[4] = {0, 0, 0, 0};
volatile int req_speed[4] = {0, 0, 0, 0};
volatile unsigned char cur_pwm[4] = {0, 0, 0, 0};
volatile int cur_err[4];

byte enc_old, enc_new;
unsigned long mtime;

/* motor channels 1, 2, 3, 4
 * left channels 2, 4
 * right channels 1, 3
 */
int pins_pwm[] = {9, 6, 5, 3};
int pins_dir[] = {7, 4, 7, 4};
int pins_left_pwm[] = {6, 3};
int pins_right_pwm[] = {9, 5};
int pin_left_dir = 4;
int pin_right_dir = 7;


/* commands and parameters */
enum {
 CMD_SET_LSPEED = 0,
 CMD_SET_RSPEED = 1,
} cmd_t;

int t_step = 0;
int t_cmd = -1;
int reply = -1;

unsigned long dtime = 0;
unsigned long ptime = 0;

unsigned int cur_pps[4];
void setup() {
    pinMode(13, OUTPUT);
    Serial.begin(9600);
    //Wire.begin(SLAVE_ADDRESS);
    //Wire.onReceive(receiveDataOnI2C);
    //Wire.onRequest(sendDataToI2C);
    init_motors();
    init_encoders();
    //t.every(SPEED_UPDATE_INTERVAL, measure_speed, (void*) 0);
    //t.every(500, display_serial, (void*) 0);
    attachInterrupt(0, isr0, CHANGE);
    Serial.println("RV Controller Interface: Ready");
    
    //digitalWrite(pins_pwm[0], HIGH);
    req_speed[0] = 20;
    req_speed[1] = 20;
    req_speed[2] = 20;
    req_speed[3] = 20;

}

void loop() {
  update_encoders();
  //updateMovement();
  update_motors();
  if(micros() - dtime > 499999) {
    dtime = micros();
    display_serial(0);
  }
}


void update_motors() {
  int i, err;
  unsigned long req_width;
  if(micros() - mtime > 2999) {
    mtime = micros();
    
    for(i=0; i<4; i++) {
      digitalWrite(pins_dir[i], req_speed[i]>0);
      if(mtime - change_time[i] > 50000ul && req_speed[i] != 0) {
        cur_pwm[i] += 5;
      } 
      /*req_width = 1000000 / ((abs(req_speed[i]) * 300)/100);
      if (req_width > pulse_width[i] && cur_pwm[i] < 255)
        cur_pwm[i]++;
      else if (req_width > pulse_width[i] && cur_pwm[i] > 0)
        cur_pwm[i]--;*/
      if(req_speed[i] > 0)
        cur_err[i] = req_speed[i]*10  - cur_speed[i];
      else
        cur_err[i] = (-req_speed[i]*10)  - cur_speed[i];
      if(cur_err[i] > 0 && cur_pwm[i] < 255) {
        cur_pwm[i]++;
      } else if(cur_err[i] < 0 && cur_pwm[i] > 0) {
        cur_pwm[i]--;
      }
      analogWrite(pins_pwm[i], cur_pwm[i]);
    }
  }
}

void update_encoders() {
  int i;
  unsigned long w;
  enc_new = PINC & B00001111;
  unsigned long etime = micros();
  for (i=0; i<4; i++) {
    if((enc_old & (1 << i)) < (enc_new & (1 << i)))
    {
      w = (unsigned long)(((etime - change_time[i])));
      pulse_width[i] = (w + pulse_width_h1[i] + pulse_width_h2[i])/3;
      pulse_width_h2[i] = pulse_width_h1[i];
      pulse_width_h1[i] = pulse_width[i];
      change_time[i]=etime;
      pulse_count[i]++;
      cur_speed[i] = (3200000ul / pulse_width[i]);
    }
  }
  enc_old=enc_new;
  /* accumulate pulses over time to measure speed */
  /*if(micros() - ptime > 49999) {
    ptime = micros();
    for (i=0; i<4; i++) {
      cur_pps[i] = pulse_count[i];
      pulse_count[i] = 0;
      //cur_speed[i] = (cur_pps[i]*100)/48;
    }
  }*/
}

void isr0() {
  int i;
  /* encoder monitor */
  //enc_new = PINC & B00001111;
  enc_new = ~enc_old;
  unsigned long etime = micros();
  for (i=0; i<4; i++) {
    if((enc_old & (1 << i)) != (enc_new & (1 << i)))
    {
      pulse_width[i]=(unsigned long)((etime - change_time[i]));
      change_time[i]=etime;
      pulse_count[i]++;
    }
  }
  enc_old=enc_new;
}

void display_serial(void *arg) {
/*  Serial.print("cur_speed, cur_pwm, cur_err: ");
  Serial.print(cur_speed[0]);
  Serial.print(" ");
  Serial.print(cur_pwm[0]);
  Serial.print(" ");
  Serial.print(cur_err[0]);
  Serial.println(" ");
 */
  Serial.print("pwm, count, speed : ");
  Serial.print(pulse_width[0]);
  Serial.print(" ");    
  Serial.print(cur_pwm[0]);
  Serial.print(" ");
  Serial.print(cur_err[0]);
  Serial.print(" ");  
  Serial.print(cur_speed[0]);
  Serial.println(" ");
}
void measure_speed(void *arg) {
  int i, pps;
  for(i=0; i< 1; i++) {
    pps = pulse_count[i] * (1000 / SPEED_UPDATE_INTERVAL);
    cur_speed[i] = (pps * 100) / SPEED_MAX_PPS;
    pulse_count[i] = 0;
  }
}

void correct_speed() {
  int i, err;
  for(i=0; i<1; i++) {
    cur_err[i] = abs(cur_speed[i]) - abs(req_speed[i]);
    if(abs(cur_err[i]) > SPEED_TOLERANCE) {
      cur_pwm[i] = cur_pwm[i] + sign(err)*1;
      analogWrite(pins_pwm[0], cur_pwm[i]);
    }
  }
}


void init_encoders()
{
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  PCintPort::attachInterrupt(PIN_ENC0_INT, &enc0isr, RISING);
  PCintPort::attachInterrupt(PIN_ENC1_INT, &enc1isr, RISING);
  PCintPort::attachInterrupt(PIN_ENC2_INT, &enc2isr, RISING);
  PCintPort::attachInterrupt(PIN_ENC3_INT, &enc3isr, RISING);
}

void enc0isr() {
  pulse_count[0]++;
}

void enc1isr() {
  pulse_count[1]++;
}

void enc2isr() {
  pulse_count[2]++;
}

void enc3isr() {
  pulse_count[3]++;
}

void init_motors()
{
  int i;
  for(i=0; i<4; i++) {
    pinMode(pins_pwm[i], OUTPUT); 
  }
  pinMode(pin_left_dir, OUTPUT);
  pinMode(pin_right_dir, OUTPUT);
}

/* updates rover left and right speed */
void updateMovement()
{
  int val, dir;
  if(lspeed != prev_lspeed) {
    val = (lspeed * 255)/100;
    if(lspeed < 0) {
      val = -val;
      dir = DIR_BACK;
    } else {
      dir = DIR_FWD;
    }
    analogWrite(pins_left_pwm[0], val & 0xff);
    analogWrite(pins_left_pwm[1], val & 0xff);
    digitalWrite(pin_left_dir, dir);
    prev_lspeed = lspeed;
  }
  if(rspeed != prev_rspeed) {
    val = (rspeed * 255)/100;
    if(rspeed < 0) {
      val = -val;
      dir = DIR_BACK;
    } else {
      dir = DIR_FWD;
    }
    analogWrite(pins_right_pwm[0], val & 0xff);
    analogWrite(pins_right_pwm[1], val & 0xff);
    digitalWrite(pin_right_dir, dir);
    prev_rspeed = rspeed;
  }
}

int processCommand(int cmd, unsigned int data) {
  int ret = 255;
  switch(cmd) {
    case CMD_SET_LSPEED:
      lspeed = (signed char) data;
      req_speed[CH_LEFT0] = lspeed;
      req_speed[CH_LEFT1] = lspeed;
      Serial.print("lspeed = ");
      Serial.println(lspeed);
      break;
    case CMD_SET_RSPEED:
      rspeed = (signed char) data;
      req_speed[CH_RIGHT0] = rspeed;
      req_speed[CH_RIGHT1] = rspeed;      
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


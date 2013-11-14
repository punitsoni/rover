#include <Timer.h>
#include <PinChangeInt.h>
#include <Wire.h>
#include <PID_v1.h>


#define SLAVE_ADDRESS 0x11

#define DIR_FWD   1
#define DIR_BACK  0

#define LEFT  0
#define RIGHT 1

#define PIN_ENC0_INT A0
#define PIN_ENC1_INT A1
#define PIN_ENC2_INT A2
#define PIN_ENC3_INT A3

#define SERIAL_OUT_PERIOD_MS       500
#define ENC_MEASURE_PULSE_COUNT    20
#define MAX_SPEED_ENC_FREQ         310
#define ENC_STALL_DETECT_TIME_MS   1000


#define CH_LEFT0   1
#define CH_LEFT1   3
#define CH_RIGHT0  0
#define CH_RIGHT1  2


Timer t;
int lspeed = 0, rspeed = 0;

volatile unsigned long enc_time[4] = {0, 0, 0, 0};
volatile unsigned long enc_time_t0[4] = {0, 0, 0, 0};
volatile unsigned long last_change[4] = {0, 0, 0, 0};
volatile unsigned int enc_count[4] = {0, 0, 0, 0};


unsigned int  cur_speed[4] = {0, 0, 0, 0};
volatile int req_speed[4] = {0, 0, 0, 0};

int running[4] = {0, 0, 0, 0};

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

double kp=0.2, ki=1.5, kd=0;
double pid_input[4], pid_output[4], pid_setpoint[4];

PID m_pid[4] = {
  PID(&pid_input[0], &pid_output[0], &pid_setpoint[0],
        kp, ki, kd, DIRECT),
  PID(&pid_input[1], &pid_output[1], &pid_setpoint[1],
        kp, ki, kd, DIRECT),
  PID(&pid_input[2], &pid_output[2], &pid_setpoint[2],
        kp, ki, kd, DIRECT),
  PID(&pid_input[3], &pid_output[3], &pid_setpoint[3],
        kp, ki, kd, DIRECT)        
};

void init_encoders()
{
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  PCintPort::attachInterrupt(PIN_ENC0_INT, &enc0isr, CHANGE);
  PCintPort::attachInterrupt(PIN_ENC1_INT, &enc1isr, CHANGE);
  PCintPort::attachInterrupt(PIN_ENC2_INT, &enc2isr, CHANGE);
  PCintPort::attachInterrupt(PIN_ENC3_INT, &enc3isr, CHANGE);
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

void display_serial(void *arg) {
  Serial.print("req_speed: ");
  Serial.print(req_speed[2]);
  Serial.print(" tuning: ");    
  Serial.print(kp); Serial.print(",");
  Serial.print(ki); Serial.print(",");
  Serial.print(kd); Serial.print(",");
  Serial.print(" cur_speed: ");
  Serial.print(cur_speed[1]);
  Serial.print(" pid_output: ");  
  Serial.print(pid_output[1]);
  Serial.print(" enc_time: ");  
  Serial.print(enc_time[1]);
  Serial.println(" ");
}

void control_loop(void *p)
{
  int i;
  for(i=0; i<4; i++) {
    unsigned long cur_freq;
    pid_setpoint[i] = abs(req_speed[i]);
    if(micros() - last_change[i] > ENC_STALL_DETECT_TIME_MS*1000 ||
      running[i] == 0) {
      cur_speed[i] = 0;
      running[i] = 0;
    } else {
      noInterrupts();
      cur_freq = (1000000 * ENC_MEASURE_PULSE_COUNT)/enc_time[i];
      interrupts();
      cur_speed[i] = (cur_freq * 100) / MAX_SPEED_ENC_FREQ;
    }
    
    pid_input[i] = cur_speed[i];
    if (pid_input[i] > 1000)
      pid_input[i] = 0;
    if(pid_setpoint[i] == 0)
      pid_output[i] = 0;
    else
      m_pid[i].Compute();
    running[i] = ((int)pid_output[i] > 0);
    //pid_output[i] = (abs(req_speed[i]) * 255) / 100;
    analogWrite(pins_pwm[i], (int) pid_output[i]);
  }
}

void update_pid(void *p)
{
  int i;
  for(i=0; i<4; i++) {
    m_pid[i].SetTunings(kp, ki, kd);
  }
}

void setup() {
    int i;
    pinMode(13, OUTPUT);
    Serial.begin(9600);
    Wire.begin(SLAVE_ADDRESS);
    Wire.onReceive(receiveDataOnI2C);
    Wire.onRequest(sendDataToI2C);
    cbi(PORTC, 4); // disable i2c pull-ups
    cbi(PORTC, 5);
    init_motors();
    init_encoders();
    for(i=0; i<4; i++) {
      m_pid[i].SetMode(AUTOMATIC);
    }
    t.every(500, display_serial, (void*) 0);
    t.every(10, control_loop, (void*) 0);
    //t.every(20, update_pid, 0);
    Serial.println("RV Controller Interface: Ready");
}

void loop() {
 t.update();
}

void handle_encoder(int i)
{
  unsigned long t1 = micros();
  if (enc_count[i] == ENC_MEASURE_PULSE_COUNT) {
    enc_time[i] = t1 - enc_time_t0[i];
    enc_time_t0[i] = t1;
    enc_count[i] = 0;
  }
  last_change[i] = t1;
  enc_count[i] ++; 
}

void enc0isr() {
  handle_encoder(0);
}

void enc1isr() {
  handle_encoder(1);
}

void enc2isr() {
  handle_encoder(2);  
}

void enc3isr() {
  handle_encoder(3);  
}

int processCommand(int cmd, unsigned int data) {
  int ret = 255;
  switch(cmd) {
    case CMD_SET_LSPEED:
      lspeed = (signed char) data;
      req_speed[CH_LEFT0] = lspeed;
      req_speed[CH_LEFT1] = lspeed;
      digitalWrite(pin_left_dir, lspeed > 0);
      //Serial.print("lspeed = ");
      //Serial.println(lspeed);
      break;
    case CMD_SET_RSPEED:
      rspeed = (signed char) data;
      req_speed[CH_RIGHT0] = rspeed;
      req_speed[CH_RIGHT1] = rspeed;    
      digitalWrite(pin_right_dir, rspeed > 0);
      //Serial.print("rspeed = ");
      //Serial.println(rspeed);       
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
            //Serial.print("cmd: ");
            //Serial.println(cmd);
            t_step = 1;
            t_cmd = cmd;
            break;
          case 1:
            data = b;
            //Serial.print("data: ");
            //Serial.println(data);
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
    //Serial.print("response: ");
    //Serial.println(reply);
    Wire.write(reply);
    t_step = 0;
}


#include <PID_v1.h>
#include <PinChangeInt.h>
#include <Wire.h>

#define SLAVE_ADDRESS 0x11


#define DIR_FWD   1
#define DIR_BACK  0

#define LEFT  0
#define RIGHT 1

#define PIN_ENC0_INT A0
#define PIN_ENC1_INT A1
#define PIN_ENC2_INT A2
#define PIN_ENC3_INT A3

#define CH_LEFT0   1
#define CH_LEFT1   3
#define CH_RIGHT0  0
#define CH_RIGHT1  2

#define SERIAL_UPDATE_INTERVAL 500000


volatile int pulse_count[4] = {0, 0, 0, 0};
volatile unsigned long change_time[4];
volatile unsigned long pulse_width10[4] = {0, 0, 0, 0};
volatile unsigned long pulse_width20[4] = {0, 0, 0, 0};
volatile unsigned long pulse_width50[4] = {0, 0, 0, 0};

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

unsigned long ptime = 0;
unsigned int cur_pps[4];

unsigned int s_changes[4] = {0, 0, 0, 0};

double pid_input, pid_output, pid_setpoint;

#define PULSE_FREQ_MAX_SPEED 310

double kp=0.2, ki=1.5, kd=0;
PID my_pid(&pid_input, &pid_output, &pid_setpoint, kp, ki, kd, DIRECT);

int t_step = 0;
int t_cmd = -1;
int reply = -1;

volatile unsigned long last_change[4];


void setup() {
    Serial.begin(9600);
    init_motors();
    init_encoders();
    Wire.begin(SLAVE_ADDRESS);
    Wire.onReceive(receiveDataOnI2C);
    Wire.onRequest(sendDataToI2C);
    
    cbi(PORTC, 4);
    cbi(PORTC, 5);

#if 0
    digitalWrite(pins_pwm[0], HIGH);
#else
    req_speed[0] = 0;
    req_speed[1] = 20;
    req_speed[2] = 20;
    req_speed[3] = 20;
#endif
  my_pid.SetMode(AUTOMATIC);
    Serial.println("PID test: Ready");
}

void loop() {
 // measure_speed();
  update_params();
 
  /*pid_input = (3*(1000000 * 10/pulse_width10[0]) +
    2*(1000000 * 20/pulse_width20[0]) +
    1*(1000000 * 50/pulse_width50[0]))/6;
    */
  noInterrupts();
  pid_input = (1000000 * 20/pulse_width20[0]);
  if(micros() - last_change[0] > 100000) {
    pid_input = 0;
  }
  interrupts();
  cur_speed[0] = (pid_input * 100) / PULSE_FREQ_MAX_SPEED;
  pid_input = cur_speed[0];
  
  if (pid_input > 200)
    pid_input = 0;
  
  if(pid_setpoint == 0)
    pid_output = 0;
  else
    my_pid.Compute();
  //pid_output = 20;
  analogWrite(pins_pwm[0], (int) pid_output);
  display_serial(0);
 
}

void update_params() {
  static unsigned long utime=0;
  if(micros() - utime >= 100000) {
    my_pid.SetTunings(kp, ki, kd);
    pid_setpoint = req_speed[0];
  }
}


void display_serial(void *arg) {
  static unsigned long dtime=0;
  if(micros() - dtime >= SERIAL_UPDATE_INTERVAL) {
    dtime = micros();
    Serial.print("set_point: ");
    Serial.print(pid_setpoint);
    Serial.print(" tuning: ");    
    Serial.print(kp); Serial.print(",");
    Serial.print(ki); Serial.print(",");
    Serial.print(kd); Serial.print(",");
    Serial.print(" pid_input: ");
    Serial.print(pid_input);
    Serial.print(" pid_output: ");  
    Serial.print(pid_output);
    Serial.print(" pulse_width: ");  
    Serial.print(pulse_width20[0]);
    Serial.println(" ");
  }
}

void init_encoders()
{
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  PCintPort::attachInterrupt(PIN_ENC0_INT, &enc0isr, CHANGE);
  PCintPort::attachInterrupt(PIN_ENC1_INT, &enc1isr, RISING);
  PCintPort::attachInterrupt(PIN_ENC2_INT, &enc2isr, RISING);
  PCintPort::attachInterrupt(PIN_ENC3_INT, &enc3isr, RISING);
}

void enc0isr() {
  static unsigned long t0_10=0, t0_20=0, t0_50=0;
  static unsigned int count10, count20, count50;
  unsigned long t1;
  if (count10 == 10) {
    t1 = micros();
    pulse_width10[0] = t1 - t0_10;
    t0_10 = t1;
    count10 = 0;
  } 
  if (count20 == 20) {
    t1 = micros();
    pulse_width20[0] = t1 - t0_20;
    t0_20 = t1;
    count20 = 0;
  }
  if (count50 == 50) {
    t1 = micros();
    pulse_width50[0] = t1 - t0_50;
    t0_50 = t1;
    count50 = 0;
  }  
  last_change[0] = micros();
  count10 ++;
  count20 ++;
  count50 ++; 
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


int processCommand(int cmd, unsigned int data) {
  int ret = 255;
  switch(cmd) {
    case 0:
      kp = data/10.0;
      break;
    case 1:
      ki = data/10.0;    
      break;
    case 2:
      kd = data/10.0;    
      break;
    case 3:
      req_speed[0] = data;    
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


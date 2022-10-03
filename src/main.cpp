#include <Arduino.h>
// https://github.com/GreyGnome/EnableInterrupt/wiki/Usage#Fast_Start

//#define EI_NOTPORTB
//#define EI_NOTPORTC
#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>

//#include <PinChangeInt.h>
 
//#define MY_PIN 5 // PD5  we could choose any pin
#define H_PIN SDA // Blue wire on sensor
#define T_PIN SCL //
 
#define H2_PIN A3  // PC3
#define T2_PIN A2  //PC2

#define M_PIN 4 // PD4 

//volatile int pwm_hvalue = 0, pwm_t1value = 0, cnt_t1, cnt_h1;
volatile long prev_h1time, prev_t1time, pwm_t1, pwm_h1;
volatile int cnt_t1=0, cnt_h1=0, cnt_m=0;

volatile int cnt_t2, cnt_h2;
volatile long prev_h2time, prev_t2time, pwm_t2, pwm_h2;

//uint8_t latest_interrupted_pin;
bool state = false;
// Funtion prototypes
void h1_falling() {
  unsigned long us;
  us = micros();
  pwm_h1 += us - prev_h1time;
  prev_h1time = us;
  cnt_h1++;
}
void t1_falling() {
  unsigned long us;
  us = micros();
  pwm_t1 += us - prev_t1time;
  prev_t1time = us;
  cnt_t1++;
}
void h2_falling() {
  unsigned long us;
//  disableInterrupt(H2_PIN); enableInterrupt(H2_PIN, h2_rising, RISING);
  us = micros();
  pwm_h2 += us - prev_h2time;
  prev_h2time = us;
  cnt_h2++;
}
void t2_falling() {
  unsigned long us;
  us = micros();
  pwm_t2 += us - prev_t2time;
  prev_t2time = us;
  cnt_t2++;
}
void m_falling() {
  cnt_m++;
}

void setup() {
  pinMode(H_PIN, INPUT_PULLUP);
  pinMode(H2_PIN, INPUT_PULLUP);
  pinMode(T_PIN, INPUT_PULLUP);
  pinMode(T2_PIN, INPUT_PULLUP);
  pinMode(M_PIN, INPUT_PULLUP);
  // digitalWrite(H_PIN, HIGH);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.println(F("Hello"));
  enableInterrupt(H_PIN, h1_falling, FALLING);
  enableInterrupt(T_PIN, t1_falling, FALLING);
  enableInterrupt(H2_PIN, h2_falling, FALLING);
  enableInterrupt(T2_PIN, t2_falling, FALLING);
  enableInterrupt(M_PIN, m_falling, FALLING);
  prev_h1time = micros();
  prev_t1time = prev_h1time;
}
 
void loop() { 
    Serial.print(millis()/1000);
    Serial.print(F(", "));
    float hval = ((float) pwm_h1 ) / cnt_h1; 
    pwm_h1 = 0; cnt_h1 = 0;
    Serial.print(hval);
    Serial.print(F(", "));    
    //float tval = ((float) pwm_t1 ); // cnt_t1; 
    float tval = ((float) pwm_t1 ) / cnt_t1; 
    pwm_t1 = 0; cnt_t1 = 0;
    Serial.print(tval);
    hval = ((float) pwm_h2 ) / cnt_h2; 
    pwm_h2 = 0; cnt_h2 = 0;
    Serial.print(F(", "));    
    Serial.print(hval);
    tval = ((float) pwm_t2 ) / cnt_t2; 
    pwm_t2 = 0; cnt_t2 = 0;
    Serial.print(F(", "));    
    Serial.print(tval);
    Serial.print(F(", "));    
    Serial.println(cnt_m);
    
    digitalWrite(LED_BUILTIN, state);
    state = not state;

    delay(1000);
}
//void h_falling(void);
//void t_falling(void);
//void h2_falling(void);
//void t2_falling(void);
/*
void h_rising() {
  //latest_interrupted_pin=PCintPort::arduinoPin;
 // PCintPort::at    static bool state;tachInterrupt(latest_interrupted_pin, &falling, FALLING);
  disableInterrupt(H_PIN);
  enableInterrupt(H_PIN, h_falling, FALLING);
  pwm_hvalue = micros() - prev_htime;
  pwm_h += pwm_hvalue;
  cnt_h++;
}
void t_rising() {
  disableInterrupt(T_PIN);
  enableInterrupt(T_PIN, t_falling, FALLING);
  pwm_tvalue = micros() - prev_ttime;
  pwm_t += pwm_tvalue;
  cnt_t++;
}
*/
/*
void h_falling() {
  disableInterrupt(H_PIN); enableInterrupt(H_PIN, h_rising, RISING);
  prev_htime = micros();
}
void t_falling() {
  disableInterrupt(T_PIN); enableInterrupt(T_PIN, t_rising, RISING);
  prev_ttime = micros();
}

void h2_rising() {
  disableInterrupt(H2_PIN);
  enableInterrupt(H2_PIN, h2_falling, FALLING);
  pwm_h2 += micros() - prev_h2time;
  cnt_h2++;
}
void t2_rising() {
  disableInterrupt(T2_PIN);
  enableInterrupt(T2_PIN, t2_falling, FALLING);
  pwm_t2 += micros() - prev_t2time;
  cnt_t2++;
}
*/

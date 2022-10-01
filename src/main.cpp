#include <Arduino.h>
// https://github.com/GreyGnome/EnableInterrupt/wiki/Usage#Fast_Start

//#define EI_NOTPORTB
//#define EI_NOTPORTC
#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>

//#include <PinChangeInt.h>
 
//#define MY_PIN 5 // PD5  we could choose any pin
#define H_PIN SDA //
#define T_PIN SCL //
 
volatile int pwm_hvalue = 0, pwm_tvalue = 0, cnt_t, cnt_h;
volatile long prev_htime, prev_ttime, pwm_t, pwm_h;

uint8_t latest_interrupted_pin;
bool state = false;
void h_falling(void);
void t_falling(void);

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

void h_falling() {
  disableInterrupt(H_PIN); enableInterrupt(H_PIN, h_rising, RISING);
  prev_htime = micros();
}

void t_falling() {
  disableInterrupt(T_PIN); enableInterrupt(T_PIN, t_rising, RISING);
  prev_ttime = micros();
}

void setup() {
  pinMode(H_PIN, INPUT_PULLUP);
  // digitalWrite(H_PIN, HIGH);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.println(F("Hello"));
  enableInterrupt(H_PIN, h_falling, FALLING);
  enableInterrupt(T_PIN, t_falling, FALLING);
  prev_htime = micros();
  prev_ttime = prev_htime;
  //PCintPort::attachInterrupt(MY_PIN, &rising, RISING);
}
 
void loop() { 
    //float tval, hval;
    Serial.print(F("pwm H: "));
    float hval = ((float) pwm_h ) / cnt_h; 
    pwm_h = 0; cnt_h = 0;
    Serial.print(hval);
    Serial.print(F(", pwm T: "));    
    float tval = ((float) pwm_t ) / cnt_t; 
    pwm_t = 0; cnt_t = 0;
    Serial.println(tval);
//    Serial.println(pwm_tvalue);
    digitalWrite(LED_BUILTIN, state);
    state = not state;

    delay(1000);
}
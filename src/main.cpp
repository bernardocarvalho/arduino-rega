#include <Arduino.h>
// https://github.com/GreyGnome/EnableInterrupt/wiki/Usage#Fast_Start

#define EI_NOTPORTB
#define EI_NOTPORTC
#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>

//#include <PinChangeInt.h>
 
#define MY_PIN 5 // PD5  we could choose any pin
 
volatile int pwm_value = 0;
volatile long prev_time = 0;
uint8_t latest_interrupted_pin;

void falling(void);

void rising() {
  //latest_interrupted_pin=PCintPort::arduinoPin;
 // PCintPort::attachInterrupt(latest_interrupted_pin, &falling, FALLING);
  disableInterrupt(MY_PIN);
  enableInterrupt(MY_PIN,falling,FALLING);
  prev_time = micros();
}
 
void falling() {
  //latest_interrupted_pin=PCintPort::arduinoPin;
  //PCintPort::attachInterrupt(latest_interrupted_pin, &rising, RISING);
  disableInterrupt(MY_PIN);   enableInterrupt(MY_PIN,rising,RISING);
  pwm_value = micros() - prev_time;

  //Serial.println(state);
}
 
void setup() {
  pinMode(MY_PIN, INPUT); digitalWrite(MY_PIN, HIGH);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.println(F("Hello"));
  enableInterrupt(MY_PIN,rising,RISING);
  //PCintPort::attachInterrupt(MY_PIN, &rising, RISING);
}
 
void loop() { 
    Serial.print(F("ledControl"));
    Serial.println(pwm_value);
    delay(1000);
}
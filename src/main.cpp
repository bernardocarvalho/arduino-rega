#include <Arduino.h>
// https://github.com/GreyGnome/EnableInterrupt/wiki/Usage#Fast_Start
// https://github.com/ppedro74/Arduino-SerialCommands

//#define EI_NOTPORTB
//#define EI_NOTPORTC
#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <TimeLib.h>
#include <SerialCommands.h>


char serial_command_buffer_[32];
SerialCommands serial_commands_(&Serial, serial_command_buffer_, sizeof(serial_command_buffer_), "\r\n", " ");

//This is the default handler, and gets called when no other command matches.
void cmd_unrecognized(SerialCommands* sender, const char* cmd)
{
  sender->GetSerial()->print("Unrecognized command [");
  sender->GetSerial()->print(cmd);
  sender->GetSerial()->println("]");
}

const int relayPin = 6; // PD6
//called for ON command
void cmd_relay_on(SerialCommands* sender)
{
  digitalWrite(relayPin, HIGH);
  sender->GetSerial()->println("Relay is on");
}

//called for OFF command
void cmd_relay_off(SerialCommands* sender)
{
  digitalWrite(relayPin, LOW);
  sender->GetSerial()->println("relay is off");
}

//Note: Commands are case sensitive
SerialCommand cmd_relay_on_("ON", cmd_relay_on);
SerialCommand cmd_relay_off_("OFF", cmd_relay_off);


//#define MY_PIN 5 // PD5  we could choose any pin
const int H_PIN = SDA; //  PC4 Blue wire on sensor
const int T_PIN = SCL; //  PC5
 
#define H2_PIN A3  // PC3
#define T2_PIN A2  //PC2

#define M_PIN 4 // PD4 

//volatile int pwm_hvalue = 0, pwm_t1value = 0, cnt_t1, cnt_h1;
volatile unsigned long prev_h1time, prev_t1time, pwm_t1, pwm_h1;
volatile unsigned long cnt_t1=0, cnt_h1=0, cnt_m=0;

volatile unsigned long cnt_t2, cnt_h2;
volatile unsigned long prev_h2time, prev_t2time, pwm_t2, pwm_h2;

volatile unsigned long ena_irq;
const int irq_sleep = 1000; // 1 sec

//uint8_t latest_interrupted_pin;
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
  disableInterrupt(M_PIN);
  ena_irq = millis() + irq_sleep;

  cnt_m++;
}

void setup() {
  Serial.begin(115200);
  pinMode(H_PIN, INPUT_PULLUP);
  pinMode(H2_PIN, INPUT_PULLUP);
  pinMode(T_PIN, INPUT_PULLUP);
  pinMode(T2_PIN, INPUT_PULLUP);
  pinMode(M_PIN, INPUT_PULLUP);
  // digitalWrite(H_PIN, HIGH);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);

  serial_commands_.SetDefaultHandler(cmd_unrecognized);
  serial_commands_.AddCommand(&cmd_relay_on_);
  serial_commands_.AddCommand(&cmd_relay_off_);

  Serial.print(F("Hello, now is "));
// Hello, now is 1664892600

  //setTime(hr,min,sec,day,month,yr); // Another way to set
  setTime(14,10,1,4,10,2022); 
  time_t t = now(); // Store the current time in time 
  Serial.println(t);

  enableInterrupt(H_PIN, h1_falling, FALLING);
  enableInterrupt(T_PIN, t1_falling, FALLING);
  enableInterrupt(H2_PIN, h2_falling, FALLING);
  enableInterrupt(T2_PIN, t2_falling, FALLING);
  enableInterrupt(M_PIN, m_falling, FALLING);
  prev_h1time = micros();
  prev_t1time = prev_h1time;
}
 
void print_loop(unsigned long now) {
    static unsigned long nextTime = 0;
    const long print_interval = 30000;
    static bool led_state = false;

//    unsigned long now = millis();	
    if ( now > nextTime ) {
    	nextTime = now + print_interval;

	Serial.print(now/1000);
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

	digitalWrite(LED_BUILTIN, led_state);
	led_state = not led_state;
    }
    //delay(1000);
}

void loop(){
    unsigned long now = millis();	
    if (now > ena_irq){ 
        enableInterrupt(M_PIN, m_falling, FALLING);
   	ena_irq += 1000000; // run once until next interrupt.  ~ 1000 s
    }
    serial_commands_.ReadSerial();
    print_loop(now);
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

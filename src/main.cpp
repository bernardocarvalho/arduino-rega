// vim: sta:et:sw=4:ts=4:sts=4
#include <Arduino.h>
// https://github.com/GreyGnome/EnableInterrupt/wiki/Usage#Fast_Start
// https://github.com/ppedro74/Arduino-SerialCommands

//#define EI_NOTPORTB
//#define EI_NOTPORTC
#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <TimeLib.h>
#include <SerialCommands.h>

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void digitalClockDisplay(){
  // digital clock display of the time
  Serial.print(day());
  Serial.print("-");
  Serial.print(month());
  Serial.print("-");
  Serial.print(year()); 
  Serial.print(" ");
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  //Serial.print(", "); 
}

char serial_command_buffer_[32];
SerialCommands serial_commands_(&Serial, serial_command_buffer_, sizeof(serial_command_buffer_), "\r\n", " ");

//This is the default handler, and gets called when no other command matches.
void cmd_unrecognized(SerialCommands* sender, const char* cmd)
{
  sender->GetSerial()->print("Unrecognized command [");
  sender->GetSerial()->print(cmd);
  sender->GetSerial()->println("]");
}
//expects one single parameter
void cmd_datetime_set(SerialCommands* sender)
{
	//Note: Every call to Next moves the pointer to next parameter
	// Use date +%s to get Linux Timestamp
 
	char* ts_str = sender->Next();
	if (ts_str == NULL)
	{
		sender->GetSerial()->println("ERROR NO_TS");
		return;
	}

	time_t ts = strtoul(ts_str, NULL, 0); //atoi(ts_str);
	setTime(ts);
	time_t t = now();
	sender->GetSerial()->println(ts);
	sender->GetSerial()->println(t);
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
SerialCommand cmd_datetime_set_("TS", cmd_datetime_set); // requires one argument

const int T_PIN = SCL; //  PC5
const int H_PIN = SDA; //  PC4 Blue wire on sensor
 
const int TLT_PIN = A2; // PC2 Yellow Sensor
const int HLT_PIN = A3; // PC3 Blue
 
const int TRB_PIN = A0; // PC0 Yellow Sensor
const int HRB_PIN = A1;
 
const int M_PIN = 4;  // PD4 

volatile unsigned long prev_h1time, prev_t1time, pwm_t1, pwm_h1;
volatile unsigned long cnt_t1=0, cnt_h1=0, cnt_m=0;

//volatile unsigned long cnt_t2, cnt_h2;
//volatile unsigned long prev_h2time, prev_t2time, pwm_t2, pwm_h2;

volatile unsigned long prev_time_hlt, prev_time_tlt, pwm_tlt, pwm_hlt,
	 cnt_hlt, cnt_tlt;
volatile unsigned long prev_time_hrb, prev_time_trb, pwm_trb, pwm_hrb,
	 cnt_hrb, cnt_trb;

volatile unsigned long ena_irq;
const int irq_sleep = 1000; // 1 sec

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
void hrb_falling() {
  unsigned long us = micros();
  pwm_hrb += us - prev_time_hrb;
  prev_time_hrb = us;
  cnt_hrb++;
}
void trb_falling() {
  unsigned long us = micros();
  pwm_trb += us - prev_time_trb;
  prev_time_trb = us;
  cnt_trb++;
}
void hlt_falling() {
  unsigned long us = micros();
  pwm_hlt += us - prev_time_hlt;
  prev_time_hlt = us;
  cnt_hlt++;
}
void tlt_falling() {
  unsigned long us = micros();
  pwm_tlt += us - prev_time_tlt;
  prev_time_tlt = us;
  cnt_tlt++;
}
void m_falling() {
  disableInterrupt(M_PIN);
  ena_irq = millis() + irq_sleep;
  cnt_m++;
}

void setup() {
  Serial.begin(115200);
  pinMode(H_PIN, INPUT_PULLUP);
  pinMode(T_PIN, INPUT_PULLUP);
  pinMode(HLT_PIN, INPUT_PULLUP);
  pinMode(TLT_PIN, INPUT_PULLUP);
  pinMode(HRB_PIN, INPUT_PULLUP);
  pinMode(TRB_PIN, INPUT_PULLUP);
  pinMode(M_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);

  serial_commands_.SetDefaultHandler(cmd_unrecognized);
  serial_commands_.AddCommand(&cmd_relay_on_);
  serial_commands_.AddCommand(&cmd_relay_off_);
  serial_commands_.AddCommand(&cmd_datetime_set_);

  Serial.print(F("Hello, now is "));
// Hello, now is 1664892600

  //setTime(hr,min,sec,day,month,yr); // Another way to set
  setTime(14,10,1,6,10,2022); 
  time_t t = now(); // Store the current time in time 
  Serial.println(t);
  Serial.println(F("timestamp,sec,Hum_0,Temp_0,Hum_LT,Temp_LT,Hum_RB,Temp_RB,H20_Meas,H2O_Pump"));

  enableInterrupt(H_PIN, h1_falling, FALLING);
  enableInterrupt(T_PIN, t1_falling, FALLING);
  enableInterrupt(HLT_PIN, hlt_falling, FALLING);
  enableInterrupt(TLT_PIN, tlt_falling, FALLING);
  enableInterrupt(HRB_PIN, hrb_falling, FALLING);
  enableInterrupt(TRB_PIN, trb_falling, FALLING);

  enableInterrupt(M_PIN, m_falling, FALLING);
  prev_h1time = micros();
  prev_t1time = prev_h1time;
  prev_time_hrb = micros();
  prev_time_trb = prev_time_hrb;
  prev_time_hlt = prev_time_hrb;
  prev_time_tlt = prev_time_hrb;
}
 
void print_loop(unsigned long now_us) {
    static unsigned long nextTime = 0;
    const long print_interval = 30000;
    static bool led_state = false;

    if ( now_us > nextTime ) {
    	nextTime = now_us + print_interval;

	digitalClockDisplay();
	Serial.print(F(", ")); Serial.print(now_us/1000);
	float hval = ((float) pwm_h1 ) / cnt_h1; 
	pwm_h1 = 0; cnt_h1 = 0;
	Serial.print(F(", ")); Serial.print(hval);
	//float tval = ((float) pwm_t1 ); // cnt_t1; 
	float tval = ((float) pwm_t1 ) / cnt_t1; 
	pwm_t1 = 0; cnt_t1 = 0;
	Serial.print(F(", ")); Serial.print(tval);

	hval = ((float) pwm_hlt ) / cnt_hlt; 
	pwm_hlt = 0; cnt_hlt = 0;
	Serial.print(F(", ")); Serial.print(hval);
	tval = ((float) pwm_tlt ) / cnt_tlt; 
	pwm_tlt = 0; cnt_tlt = 0;
	Serial.print(F(", ")); Serial.print(tval);

	hval = ((float) pwm_hrb ) / cnt_hrb; 
	pwm_hrb = 0; cnt_hrb = 0;
	Serial.print(F(", ")); Serial.print(hval);
	tval = ((float) pwm_trb ) / cnt_trb; 
	pwm_trb = 0; cnt_trb = 0;
	Serial.print(F(", ")); Serial.print(tval);
	Serial.print(F(", ")); Serial.print(cnt_m);

	Serial.println(F(", 0"));

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

// vim: sta:et:sw=4:ts=4:sts=4
// https://www.seeedstudio.com/Seeeduino-Lotus-V1-1-ATMega328-Board-with-Grove-Interface.html
// Seeeduino Lotus V1.1
#include <Arduino.h>
// https://github.com/GreyGnome/EnableInterrupt/wiki/Usage#Fast_Start
// https://github.com/ppedro74/Arduino-SerialCommands

//#define EI_NOTPORTB
//#define EI_NOTPORTC
#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <TimeLib.h>
#include <SerialCommands.h>

/*
 * /dev/ttyS1 on BeagleBone Black
 * Arduino TX: BB: UART1_RX – Pin26
 *         RX: BB: UART1_TX – Pin24
 */
const int N_IRQ = 20;

/*
 * https://docs.arduino.cc/hardware/uno-rev3
 D0  RX
 D1  TX

 D4  RX_LED
 D5  TX_LED
 PB5 13 LED_BUILTIN
 */
const int relayPin = 2; // PD2
const int M_PIN = 3;  // PD3 

const int TRT_PIN = 6; // PD6 Yellow Sensor
const int HRT_PIN = 7;

const int TRB_PIN = A0; // PC0 Yellow Sensor
const int HRB_PIN = A1;
const int TLT_PIN = A2; //A2; // PC2 Yellow Sensor. NOT Used
const int HLT_PIN = A3; //A3; // PC3 Blue
const int HAD_PIN = A2; // PC2

const int T0_PIN = SCL; //  PC5/A5
const int H0_PIN = SDA; //  PC4/A4 Blue wire on sensor


bool relayState = false;

unsigned long ena_irq,  ena_sensor_irq;  // in millis()

unsigned long prev_time_h0, prev_time_t0, prd_t0, prd_h0;
unsigned long cnt_t0=0, cnt_h0=0, cnt_m=0;

volatile unsigned long prev_time_hlt, prev_time_tlt, prd_tlt, prd_hlt,
         cnt_hlt, cnt_tlt;
unsigned long prev_time_hrb, prev_time_trb, prd_trb, prd_hrb,
         cnt_hrb, cnt_trb;
unsigned long prev_time_hrt, prev_time_trt, prd_hrt, prd_trt,
              cnt_hrt, cnt_trt;
bool t0, hlt, tlt, hrb, trb, hrt, trt;

const int irq_sleep = 1000UL; // 1 sec
const int WATER_AUTO = 60UL; // in sec. Auto water/ day

time_t stop_valve = 0, next_water = 0;

// Funtion Prototype
void print_loop(unsigned long now_ms);

void printDigits(int digits){
    // utility function for digital clock display: prints preceding colon and leading 0
    Serial.print(":");
    if(digits < 10)
        Serial.print('0');
    Serial.print(digits);
}

void digitalClockPrint(){
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
}

char serial_command_buffer_[32];
SerialCommands serial_commands_(&Serial, serial_command_buffer_, sizeof(serial_command_buffer_), "\r\n", " ");

//This is the default handler, and gets called when no other command matches.
void cmd_unrecognized(SerialCommands* sender, const char* cmd) {
    sender->GetSerial()->print("Unrecognized command [");
    sender->GetSerial()->print(cmd);
    sender->GetSerial()->println("]");
}
//expects one single parameter
void cmd_datetime_set(SerialCommands* sender) {
    //Note: Every call to Next moves the pointer to next parameter
    // Use date +%s to get Linux Timestamp

    char* ts_str = sender->Next();
    if (ts_str == NULL) {
        sender->GetSerial()->println("ERROR NO_TS");
        return;
    }

    time_t ts = strtoul(ts_str, NULL, 0); //atoi(ts_str);
    setTime(ts);
    time_t t = now();
    next_water = t + 24UL * 3600UL; // wait one day
    stop_valve = 0; // 
    sender->GetSerial()->print(F("My time now is: "));
    sender->GetSerial()->println(t);
}

//called for ON command
void cmd_relay_on(SerialCommands* sender) {
    const time_t water_interval = 24UL * 3600UL; // in sec
    char * min_str = sender->Next();
    if (min_str == NULL) {
        sender->GetSerial()->println("ERROR Minute Arg");
        return;
    }
    int min = atoi(min_str);
    stop_valve = now() + 60UL * min;
    next_water  = now() + water_interval; // 24 * 3600; // Next day, same hour
    relayState = true;
    digitalWrite(relayPin, relayState);
    sender->GetSerial()->print(F("Relay is on for: "));
    sender->GetSerial()->println(min);
}

//called for OFF command
void cmd_relay_off(SerialCommands* sender) {
    relayState = false;
    digitalWrite(relayPin, relayState);
    sender->GetSerial()->println("relay is off");
}

void cmd_print(SerialCommands* sender) {
    unsigned long now_ms = millis();	
    print_loop(now_ms);
    //sender->GetSerial()->println("off");
}

//Note: Commands are case sensitive
SerialCommand cmd_relay_on_("ON", cmd_relay_on); // requires one argument
SerialCommand cmd_relay_off_("OFF", cmd_relay_off);
SerialCommand cmd_datetime_set_("TS", cmd_datetime_set); // requires one argument
/// Add one_key commands 
SerialCommand cmd_print_("p", cmd_print, true);

// IRQ Funtion 
void t0_falling() {
    unsigned long us;
    if(cnt_t0++ > N_IRQ) {
        disableInterrupt(T0_PIN);
        us = micros();
        prd_t0 = us - prev_time_t0;
        cnt_t0 = 0;
    };
}
void h0_falling() {
    unsigned long us;
    if(cnt_h0++ > N_IRQ) {
        disableInterrupt(H0_PIN);
        us = micros();
        prd_h0 = us - prev_time_h0;
        cnt_h0 = 0;
    };
}
void hlt_falling() {
    if(cnt_hlt++ > N_IRQ) {
        disableInterrupt(HLT_PIN);
        prd_hlt = micros() - prev_time_hlt;
        cnt_hlt = 0;
    };
}
void tlt_falling() {
    if(cnt_tlt++ > N_IRQ) {
        disableInterrupt(TLT_PIN);
        prd_tlt = micros() - prev_time_tlt;
        cnt_tlt = 0;
    };
}
void hrb_falling() {
    if(cnt_hrb++ > N_IRQ) {
        disableInterrupt(HRB_PIN);
        prd_hrb = micros() - prev_time_hrb;
        cnt_hrb = 0;
    };
}
void trb_falling() {
    if(cnt_trb++ > N_IRQ) {
        disableInterrupt(TRB_PIN);
        prd_trb = micros()- prev_time_trb;
        cnt_trb = 0;
    };
}
void hrt_falling() {
    if(cnt_hrt++ > N_IRQ) {
        disableInterrupt(HRT_PIN);
        prd_hrt = micros() - prev_time_hrt;
        cnt_hrt = 0;
    };
}
void trt_falling() {
    if(cnt_trt++ > N_IRQ) {
        disableInterrupt(TRT_PIN);
        prd_trt = micros() - prev_time_trt;
        cnt_trt = 0;
    };
}

void m_falling() {
    disableInterrupt(M_PIN);
    ena_irq = millis() + irq_sleep; // Debouncing reed sensor
    cnt_m++;
}

void setup() {
    Serial.begin(115200);
    pinMode(H0_PIN, INPUT_PULLUP);
    pinMode(T0_PIN, INPUT_PULLUP);
//    pinMode(HLT_PIN, INPUT_PULLUP);
//    pinMode(TLT_PIN, INPUT_PULLUP);
    pinMode(HRB_PIN, INPUT_PULLUP);
    pinMode(TRB_PIN, INPUT_PULLUP);
    pinMode(HRT_PIN, INPUT_PULLUP);
    pinMode(TRT_PIN, INPUT_PULLUP);
    pinMode(M_PIN, INPUT_PULLUP);

    pinMode(LED_BUILTIN, OUTPUT);

    pinMode(relayPin, OUTPUT);
    relayState = false;
    digitalWrite(relayPin, relayState);

    serial_commands_.SetDefaultHandler(cmd_unrecognized);
    serial_commands_.AddCommand(&cmd_relay_on_);
    serial_commands_.AddCommand(&cmd_relay_off_);
    serial_commands_.AddCommand(&cmd_datetime_set_);
    serial_commands_.AddCommand(&cmd_print_);

    Serial.print(F("Hello, now is "));

    //setTime(hr,min,sec,day,month,yr); // Another way to set
    setTime(00,10,1,1,11,2022); 
    time_t t = now(); // Store the current time in time (UTC)
                      // get Linux date : date +%s
    Serial.println(t);
    Serial.println(F("timestamp,sec,Hum_0,Temp_0,Hum_LT,Temp_LT,Hum_RB,Temp_RB,Hum_RT,Temp_RT,H20_Meas,H2O_Pump,H_ADC"));

    enableInterrupt(H0_PIN, h0_falling, FALLING);
    enableInterrupt(M_PIN, m_falling, FALLING);
    next_water = t + 48UL * 3600UL; // wait two days on Starting
    stop_valve = 0; // in sec
    ena_sensor_irq= millis();
}

void print_loop(unsigned long now_ms) {
    int sensorA6Value = 0;

    digitalClockPrint();
    Serial.print(F(", ")); Serial.print(now_ms/1000);
    float hval = ((float) prd_h0 );
    hval = hval / N_IRQ;
    prd_h0 = 0; cnt_h0 = 0;
    Serial.print(F(", ")); Serial.print(hval);
    float tval = (float) prd_t0; 
    tval = tval / N_IRQ;
    Serial.print(F(", ")); Serial.print(tval);

    hval = (float) prd_hlt;
    hval = 0; //hval / N_IRQ;
    Serial.print(F(", ")); Serial.print(hval);
    tval = (float) prd_tlt;
    tval = 0; // tval / N_IRQ;
    Serial.print(F(", ")); Serial.print(tval);

    hval = (float) prd_hrb;
    hval = hval / N_IRQ;
    Serial.print(F(", ")); Serial.print(hval);
    tval = (float) prd_trb;
    tval = tval / N_IRQ;
    Serial.print(F(", ")); Serial.print(tval);

    hval = (float) prd_hrt;
    hval = hval / N_IRQ;
    Serial.print(F(", ")); Serial.print(hval);
    tval = (float) prd_trt;
    tval = tval / N_IRQ;
    Serial.print(F(", ")); Serial.print(tval);

    Serial.print(F(", ")); Serial.print(cnt_m);

    Serial.print(F(", ")); Serial.print(relayState);

    sensorA6Value = analogRead(HAD_PIN);
    Serial.print(F(", ")); Serial.println(sensorA6Value);

}

const int T0_WAIT =  400UL; 
const int HLT_WAIT =  T0_WAIT + 100UL; 
const int TLT_WAIT =  HLT_WAIT + 100UL; 
const int HRB_WAIT =  TLT_WAIT + 100UL; 
const int TRB_WAIT =  HRB_WAIT + 400UL; 
const int HRT_WAIT =  TRB_WAIT + 100UL; 
const int TRT_WAIT =  HRT_WAIT + 400UL; 

void loop(){
    static unsigned long nextPtime = 0;
    const long print_interval = 30 * 1000UL; // in millis
    static bool led_state = false;

    unsigned long now_ms = millis();	
    if (now_ms > ena_irq){  // debouncing 
        enableInterrupt(M_PIN, m_falling, FALLING);
        ena_irq += 1000000UL; // run once until next interrupt.  ~ 1000 s
    }

    time_t now_ts = now();
    if (now_ts > next_water){
	next_water += 24UL * 3600UL; // repeat next day
        relayState = true;
        digitalWrite(relayPin, relayState);
        stop_valve = now_ts + WATER_AUTO;
    }
    if (now_ts > stop_valve){
        relayState = false;
        digitalWrite(relayPin, relayState);
    }

    unsigned long us = micros();
    if ((now_ms > ena_sensor_irq + T0_WAIT) && t0){
        prev_time_t0 = us;
        enableInterrupt(T0_PIN, t0_falling, FALLING);
        t0 = false;
    }
    if ((now_ms > ena_sensor_irq + HLT_WAIT) && hlt){
        prev_time_hlt = us;
        enableInterrupt(HLT_PIN, hlt_falling, FALLING);
        hlt = false;
    }
    if ((now_ms > ena_sensor_irq + TLT_WAIT) && tlt){
        prev_time_tlt = us;
        enableInterrupt(TLT_PIN, tlt_falling, FALLING);
        tlt = false;
    }
    if ((now_ms > ena_sensor_irq + HRB_WAIT) && hrb){
        prev_time_hrb = us;
        enableInterrupt(HRB_PIN, hrb_falling, FALLING);
        hrb = false;
    }
    if ((now_ms > ena_sensor_irq + TRB_WAIT) && trb){
        prev_time_trb = us;
        enableInterrupt(TRB_PIN, trb_falling, FALLING);
        trb = false;
    }
    if ((now_ms > ena_sensor_irq + HRT_WAIT) && hrt){
        prev_time_hrt = us;
        enableInterrupt(HRT_PIN, hrt_falling, FALLING);
        hrt = false;
    }
    if ((now_ms > ena_sensor_irq + TRT_WAIT) && trt){
        prev_time_trt = us;
        enableInterrupt(TRT_PIN, trt_falling, FALLING);
        trt = false;
    }
    serial_commands_.ReadSerial();
    if ( now_ms > nextPtime ) {
        nextPtime = now_ms + print_interval;
	digitalWrite(LED_BUILTIN, led_state);
	led_state = not led_state;
	prev_time_h0 = micros();
	enableInterrupt(H0_PIN, h0_falling, FALLING);
	ena_sensor_irq = millis(); // start Interrupt Cycle
	t0 = true; hlt = false; tlt = false; hrb = true; trb = true; hrt = true; trt = true;  // enable irq flags
    //t0 = true; hlt = true; tlt = true; hrb = true; trb = true; hrt = true; trt = true;  // enable irq flags
	if ((hour(now_ts) < 1) || (hour(now_ts) > 8))  // Do no print 1AM -> 8AM
	    print_loop(now_ms);
    }
}


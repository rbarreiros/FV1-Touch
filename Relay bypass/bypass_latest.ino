// Relay.ino
// true-bypass pedal switcher for guitar effects
// (c) 2014 Iwan Heskamp, but this is really basic so anybody could probably do it better!

#include <EEPROM.h>
#include <Bounce2.h>

// for attiny25/45/85
#define SW            3    // pin 2 
//#define SW_INT        PCINT3 // interrupt name
#define RELAY1        4    // pins for AL5WN-K bistable (latching) relay
#define RELAY2        0
#define LED           2    // pin 7 on ATtiny 45/85

#define ON            1
#define OFF           0
#define DEBOUNCETIME  25   
#define RELAYDELAY    15   // bi-stable relay, time for the coil to be energized
#define PRIMETIME     1000 // PrimeTime delay in milliseconds
#define DOUBLECLICK   1000  // click twice in less than these ms to be registered as doubleclick 

Bounce sw = Bounce(); // initialize debouncer

int state     = 0;   // effect on or off?
int primetime = 0;   // TC Electronics 'PrimeTime' mode
int address   = 0;   // EEPROM address of state value
unsigned long lastMillis=0;
unsigned long doubleMillis=0;

void effectOn() {
 digitalWrite(LED, LOW);      // turn LED on
 digitalWrite(RELAY1, HIGH);  // switch relay to on position
 delay(RELAYDELAY);           // wait a little bit
 digitalWrite(RELAY1, LOW);   // turn current through coil off
 state = ON;                  // remember on state
}

void effectOff() {
 digitalWrite(LED, HIGH);     // turn LED off
 digitalWrite(RELAY2, HIGH);  // switch relay to off position
 delay(RELAYDELAY);           // wait a little bit
 digitalWrite(RELAY2, LOW);   // turn current through coil off
 state = OFF;                 // remember off state
}

void setup() {
 ADCSRA = 0;  // turn ADC off
 pinMode(SW, INPUT);                    // make switch input
 digitalWrite(SW, HIGH);                // internal pullup 
 sw.attach(SW); 
 sw.interval(DEBOUNCETIME);
 digitalWrite(LED, HIGH);               // led off
 pinMode(LED, OUTPUT);                  // make led output
 digitalWrite(RELAY1, LOW);
 pinMode(RELAY1, OUTPUT); digitalWrite(RELAY1, LOW);
 digitalWrite(RELAY2, LOW);
 pinMode(RELAY2,OUTPUT); digitalWrite(RELAY2, LOW);

 state = EEPROM.read(address);          // get state from EEPROM
 sw.update();
 if(digitalRead(SW) == 0) {             // button pushed at startup -> save 'on' state for next!
   state &= 1;                          // to be sure it's written only 1 or 0. (chip can be initialized to random value!)
   state ^= 1;                          // flip stored state
   EEPROM.write(address, state);        // write flipped state to EEPROM
   digitalWrite(LED, HIGH); delay(300); // blink three times as confirmation
   digitalWrite(LED, LOW);  delay(300);
   digitalWrite(LED, HIGH); delay(300);
   digitalWrite(LED, LOW);  delay(300);
   digitalWrite(LED, HIGH);               
 } 
 if(state == OFF) {             // state == 0 -> turn effect off, led off
   effectOff();
 } else {                       // state == 1 -> turn effect on, led on
    effectOn();
 }
 // TODO: this would also be the time to switch some stuff off (UART, ADC, etc) (p.34?)
 // more power can be saved by running the attiny on 128kHz! (p.28)
 // TODO: interrupt on switch input? 
 unsigned long lastMillis = millis(); // store timestamp for PrimeTime function


}

void loop() {
 if(sw.update()) {
   if(sw.fell() == 1) { // switch is pressed (it's pulled-up, remember?)
     if(state == 1) {  // were we on, turn off
       // remember: we cannot be on *and* in PrimeTime when the button is pressed!
       effectOff();    // turn effect off, led off
     } else {          // we were off, turn on
       effectOn();
       lastMillis = millis(); // update timer for PrimeTime checks
     }
     // now check when the last time was that we were here        
     /* if(millis() - doubleMillis < DOUBLECLICK) { // SPECIAL SUPER DUPER MODE!
       digitalWrite(LED, LOW);  delay(100);
       digitalWrite(LED, HIGH); delay(100);
       digitalWrite(LED, LOW);  delay(100);
       digitalWrite(LED, HIGH); delay(100);
       digitalWrite(LED, LOW);  delay(100);
       digitalWrite(LED, HIGH); delay(100);
       digitalWrite(LED, LOW);  delay(100);
       digitalWrite(LED, HIGH);
       doubleMillis = 0;
     } else {
       doubleMillis = millis();     
     } */
   }
   if(sw.fell() == 0) {    // switch is released (it's pulled up so invert test)    
     if(primetime == ON) {        // we're in PrimeTime mode, so switch effect off
       effectOff();               // turn effect off
       primetime = OFF;           // turn off PrimeTime mode
     } // there is no else: if not in PrimeTime mode, do nothing on release of the switch...
   }
 }
 // now check if enough time has passed to turn on PrimeTime mode
 if(state == ON && (millis() - lastMillis > PRIMETIME) && primetime == OFF) { // more than PRIMETIME ms since button was pressed!
   if(!sw.read()) {                 // button still pressed, so enter PrimeTime
     primetime = ON;
     digitalWrite(LED, HIGH); delay(50);
     digitalWrite(LED, LOW);  delay(50);
     digitalWrite(LED, HIGH); delay(50);
     digitalWrite(LED, LOW);
   }
 }
 // TODO: check if we should sleep for a while?
}

/* 
AudioCPU Controller v2 for Atmel Arduino Uno CPU's
by Ben Provenzano III
*/
 
#include <IRremote.h>
 
int RECV_PIN = 2; // output pin of TSOP4838

#define fet1 5 // Play/Pause PIN-18
#define fet2 6 // Rewind PIN-18
#define fet3 7 // Forward PIN-13
#define fet4 8 // Stop/Load PIN-14
#define fet5 9 // Optical PIN-15
#define fet6 10 // Analog PIN-16
#define fet7 11 // Pot Select PIN-17
#define fet8 12 // CW Motor PIN-18
#define fet9 13 // CCW Motor PIN-19

int itsONfet[] = {0,0,0,0,0,0,0,0,0};

#define code1 0x2FD0CF3 // Play/Pause (Unused)
#define code2 0x2FDEC13 // Rewind (Unused)
#define code3 0x2FD6C93 // Forward (Unused)
#define code4 0x2FD8C73 // Stop/Load (Unused)
//
#define code5 0x40BEDA25 // Optical Input "Yellow 2"
#define code6 0x40BEE817 // Analog Input "Blue 3"
#define code7 0x40BE8D72 // LFE/Midpass Select
#define code8 0x40BEF00F // Volume Up CH+
#define code9 0x40BE5AA5 // Volume Down CH-
#define code10 0x40BE6897 // Pi Input "Green 1"

IRrecv irrecv(RECV_PIN);
 
decode_results results;
 
void setup()
{
 // Serial.begin(9600);
  irrecv.enableIRIn();  // Start the receiver
  pinMode(fet1, OUTPUT);
  digitalWrite(fet1, LOW);
  pinMode(fet2, OUTPUT);
  digitalWrite(fet2, LOW);
  pinMode(fet3, OUTPUT);
  digitalWrite(fet3, LOW);
  pinMode(fet4, OUTPUT);
  digitalWrite(fet4, LOW);  
  pinMode(fet5, OUTPUT);
  digitalWrite(fet5, LOW);
  pinMode(fet6, OUTPUT);
  digitalWrite(fet6, LOW);
  pinMode(fet7, OUTPUT);
  digitalWrite(fet7, LOW); 
  pinMode(fet8, OUTPUT);
  digitalWrite(fet8, LOW);
  pinMode(fet9, OUTPUT);
  digitalWrite(fet9, LOW);
}
 
void loop() {
  if (irrecv.decode(&results)) {
    unsigned int value = results.value;
    switch(value) {
       case code1:
         if(itsONfet[1] == 1) {
            digitalWrite(fet1, HIGH);
            delay(75);  // Button Press Time
            digitalWrite(fet1, LOW);
            itsONfet[1] = 0;
            delay(25);          
         } else {                     
            digitalWrite(fet1, HIGH);
            delay(100);  // Button Press Time
            digitalWrite(fet1, LOW);
            itsONfet[1] = 1;
            delay(25);         
         }
          break; 
       case code2:
         if(itsONfet[2] == 1) {
             digitalWrite(fet2, HIGH);
             delay(75);  // Button Press Time
             digitalWrite(fet2, LOW);
             itsONfet[2] = 0;
             delay(25);
         } else {
             digitalWrite(fet2, HIGH);
             delay(75);  // Button Press Time
             digitalWrite(fet2, LOW);
             itsONfet[2] = 1;
             delay(25);
         }
          break;
       case code3:
         if(itsONfet[3] == 1) {
            digitalWrite(fet3, HIGH);
            delay(75);  // Button Press Time
            digitalWrite(fet3, LOW);
            itsONfet[3] = 0;
            delay(25);
         } else {
             digitalWrite(fet3, HIGH);
             delay(75);  // Button Press Time
             digitalWrite(fet3, LOW);
             itsONfet[3] = 1;
             delay(25);
         }
          break;
       case code4:
         if(itsONfet[4] == 1) {
            digitalWrite(fet4, HIGH);
            delay(75);  // Button Press Time
            digitalWrite(fet4, LOW);
            itsONfet[4] = 0;
            delay(25);
         } else {
             digitalWrite(fet4, HIGH);
             delay(75);  // Button Press Time
             digitalWrite(fet4, LOW);
             itsONfet[4] = 1;
             delay(25);
         }
          break;
       case code5:
         if(itsONfet[5] == 1) {
            digitalWrite(fet5, HIGH); // Optical Mode On
            delay(75);
            digitalWrite(fet6, LOW); // Analog Mode Off
            itsONfet[5] = 0;
            delay(25);
         } else {
            digitalWrite(fet5, HIGH); // Optical Mode On
            delay(75);
            digitalWrite(fet6, LOW); // Analog Mode Off
            itsONfet[5] = 1;
            delay(25);
         }
          break;
       case code6:
         if(itsONfet[6] == 1) {
            digitalWrite(fet5, LOW); // Optical Mode Off
            delay(75);
            digitalWrite(fet6, HIGH); // Analog Mode On
            itsONfet[6] = 0;
            delay(25);
         } else {
            digitalWrite(fet5, LOW); // Optical Mode Off
            delay(100);
            digitalWrite(fet6, HIGH); // Analog Mode On
            itsONfet[6] = 1;
            delay(25);
         }
          break;
       case code7:
         if(itsONfet[7] == 1) {
            digitalWrite(fet7, LOW); // LFE Knob "Low"
            itsONfet[7] = 0;
            delay(75);
         } else {
            digitalWrite(fet7, HIGH); // Midpass Knob "High"
            itsONfet[7] = 1;
            delay(75);
         }
          break;
       case code8:
         if(itsONfet[8] == 1) {
            digitalWrite(fet8, HIGH);
            delay(225);  // CW Motor Rotation Time
            digitalWrite(fet8, LOW);
            itsONfet[8] = 0;
            delay(25);
         } else {
            digitalWrite(fet8, HIGH);
            delay(225);  // CW Motor Rotation Time
            digitalWrite(fet8, LOW);
            itsONfet[8] = 1;
            delay(25);
         }
          break;
       case code9:
         if(itsONfet[9] == 1) {
            digitalWrite(fet9, HIGH);
            delay(225);  // CCW Motor Rotation Time
            digitalWrite(fet9, LOW);
            itsONfet[9] = 0;
            delay(25);
         } else {
            digitalWrite(fet9, HIGH);
            delay(225);  // CCW Motor Rotation Time
            digitalWrite(fet9, LOW);
            itsONfet[9] = 1;
            delay(25);
         }
          break;
        case code10:
         if(itsONfet[10] == 1) {
            digitalWrite(fet6, LOW); // Analog Mode Off
            delay(100);
            digitalWrite(fet5, LOW); // Optical Mode Off
            itsONfet[10] = 0;
            delay(25);
         } else {
            digitalWrite(fet6, LOW); // Analog Mode Off
            delay(100);
            digitalWrite(fet5, LOW); // Optical Mode Off
            itsONfet[10] = 1;
            delay(25);
         }
          break;                 
    }
    irrecv.resume(); // Receive the next value
  }
}

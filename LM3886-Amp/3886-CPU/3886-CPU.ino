/* 
MultiCPU for Living Room LM3886 Amplifier
by Ben Provenzano III
*/
 
#include <IRremote.h>
 
int RECV_PIN = 2; // output pin of TSOP4838

#define fet1 5 // N/A
#define fet2 13 // N/A
#define fet3 12 // N/A
#define fet4 11 // N/A
#define fet5 9 // N/A
#define fet6 10 // N/A
#define fet7 8 // N/A
#define fet8 6 // CW Motor PIN-13
#define fet9 7 // CCW Motor PIN-12

int itsONfet[] = {0,0,0,0,0,0,0,0,0};

#define code1 0x2FD0CF3 //
#define code2 0x2FDEC13 //
#define code3 0x2FD6C93 //
#define code4 0x2FD8C73 //
#define code5 0x2FDBC43 //
#define code6 0x2FD04FB //
#define code7 0x20DF4EB1 // 
#define code8 0x5EA158A7 // Vol Up (NEC 32-Bit from Yamaha Receiver Remote)
#define code9 0x5EA1D827 // Vol Down (NEC 32-Bit from Yamaha Receiver Remote)
#define code10 0x5EA138C7 // Mute (NEC 32-Bit from Yamaha Receiver Remote)

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
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
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
            digitalWrite(fet7, LOW); // Mute Off
            itsONfet[7] = 0;
            delay(75);
         } else {
            digitalWrite(fet7, HIGH); // Mute On
            itsONfet[7] = 1;
            delay(75);
         }
          break;
       case code8:
         if(itsONfet[8] == 1) {
            digitalWrite(fet8, HIGH);
            digitalWrite(LED_BUILTIN, HIGH);
            delay(375);  // CW Motor Rotation Time
            digitalWrite(LED_BUILTIN, LOW);
            digitalWrite(fet8, LOW);
            itsONfet[8] = 0;
            delay(25);
         } else {
            digitalWrite(fet8, HIGH);
            digitalWrite(LED_BUILTIN, HIGH);
            delay(375);  // CW Motor Rotation Time
            digitalWrite(LED_BUILTIN, LOW);
            digitalWrite(fet8, LOW);
            itsONfet[8] = 1;
            delay(25);
         }
          break;
       case code9:
         if(itsONfet[9] == 1) {
            digitalWrite(fet9, HIGH);
            digitalWrite(LED_BUILTIN, HIGH);
            delay(375);  // CCW Motor Rotation Time
            digitalWrite(LED_BUILTIN, LOW);
            digitalWrite(fet9, LOW);
            itsONfet[9] = 0;
            delay(25);
         } else {
            digitalWrite(fet9, HIGH);
            digitalWrite(LED_BUILTIN, HIGH);
            delay(375);  // CCW Motor Rotation Time
            digitalWrite(LED_BUILTIN, LOW);
            digitalWrite(fet9, LOW);
            itsONfet[9] = 1;
            delay(25);
         }
          break;
        case code10:
         if(itsONfet[10] == 1) {
            digitalWrite(fet9, HIGH);
            digitalWrite(LED_BUILTIN, HIGH);
            delay(10000);  // CCW Motor Rotation Time
            digitalWrite(LED_BUILTIN, LOW);
            digitalWrite(fet9, LOW);
            itsONfet[10] = 0;
            delay(25);
         } else {
            digitalWrite(fet9, HIGH);
            digitalWrite(LED_BUILTIN, HIGH);
            delay(10000 );  // CCW Motor Rotation Time
            digitalWrite(LED_BUILTIN, LOW);
            digitalWrite(fet9, LOW);
            itsONfet[10] = 1;
            delay(25);
         }
          break;                 
    }
    irrecv.resume(); // Receive the next value
  }
}

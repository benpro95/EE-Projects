/*  
DAM1021 DAC Remote Receiver for Arduino Uno
by Ben Provenzano III
*/
 
#include <IRremote.h>
 
int RECV_PIN = 2; // Pin of TSOP34838YA1

#define fet1 8 // Input 0
#define fet2 7 // Input 1
#define fet9 6 // LED

// Onn Soundbar Remote
#define code1 0xEE110AF5 // USB Input (Music Button)
#define code2 0xEE11E817 // Coaxial Input (Aux Button)
#define code3 0xEE11F20D // Optical Input (TV Button)
#define code4 0xEE11A45B // Auto Input (Play Button)

IRrecv irrecv(RECV_PIN);
 
decode_results results;
 
void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn();  // Start the receiver
  pinMode(fet1, OUTPUT);
  digitalWrite(fet1, LOW);
  pinMode(fet2, OUTPUT);
  digitalWrite(fet2, LOW);
  pinMode(fet9, OUTPUT);
  digitalWrite(fet9, LOW);
}
 
void loop() {
  if (irrecv.decode(&results)) {
    // Only respond to NEC IR code
    if (results.decode_type == NEC) {
    unsigned int value = results.value;
    Serial.print("NEC: ");
    Serial.print(results.value);
    Serial.print(" ");
    switch(value) {
       case code1:
         {
            digitalWrite(fet1, HIGH);
            digitalWrite(fet2, HIGH);
            digitalWrite(fet9, HIGH);
            delay(75);
            digitalWrite(fet9, LOW);
            delay(50);
         }
          break; 
       case code2:
         {
             digitalWrite(fet1, HIGH);
             digitalWrite(fet2, LOW);
             digitalWrite(fet9, HIGH);
             delay(75);
             digitalWrite(fet9, LOW);
             delay(50);
         }
          break;
       case code3:
         {
             digitalWrite(fet1, LOW);
             digitalWrite(fet2, HIGH);
             digitalWrite(fet9, HIGH);
             delay(75);
             digitalWrite(fet9, LOW);
             delay(50);
         }
          break;
       case code4:
         {
            digitalWrite(fet1, LOW);
            digitalWrite(fet2, LOW);
            digitalWrite(fet9, HIGH);
            delay(75);
            digitalWrite(fet9, LOW);
            delay(50);
         }
          break;                 
    }
    } else if (results.decode_type == SONY) {
      Serial.print("SONY: ");
    } else if (results.decode_type == RC5) {
      Serial.print("RC5: ");
    } else if (results.decode_type == RC6) {
      Serial.print("RC6: ");
    } else if (results.decode_type == UNKNOWN) {
      Serial.print("UNKNOWN: ");
    }
    irrecv.resume(); // Receive the next value
  }
}

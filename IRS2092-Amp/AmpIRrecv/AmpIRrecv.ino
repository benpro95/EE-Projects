/* 
Class D Amp Remote Receiver for Arduino Uno
by Ben Provenzano III
*/
 
#include <IRremote.hpp> // v3+

// IR
#define IRpin 2 // receiver in
uint8_t debounceIR = 75; // IR receive max rate (ms)
bool powerToggle = 0;

// Outputs
#define mutePin 8 // Mute PIN-14
#define motorCW 7 // CW Motor PIN-13
#define motorCCW 6 // CCW Motor PIN-12

// receive IR remote commands 
// Philips SRP9232D/27 Universal Remote
// programmed with code '1221' (NEC DVD Player)
// commented codes are for Xmit
void irReceive() 
{
  if (IrReceiver.decode()) {
  // code detected      
    if (IrReceiver.decodedIRData.address == 0x4) { // address   
      digitalWrite(LED_BUILTIN, HIGH); 
      if (IrReceiver.decodedIRData.command == 0xA) { // toggle mute 551506095
        if(powerToggle == 1) {
          digitalWrite(mutePin, LOW); 
          powerToggle = 0;
        } else {
          digitalWrite(mutePin, HIGH);
          powerToggle = 1;
        } 
      }     
      if (IrReceiver.decodedIRData.command == 0x11) { // mute on (1) 551520375
        digitalWrite(mutePin, HIGH); 
        powerToggle = 1;
      }    
      if (IrReceiver.decodedIRData.command == 0x12) { // mute off (2) 551504055
        digitalWrite(mutePin, LOW); 
        powerToggle = 0;
      }         
      if (IrReceiver.decodedIRData.command == 0x2) { // volume up 551502015
        digitalWrite(motorCW, HIGH);
        delay(200);  // CW Motor Rotation Time
        digitalWrite(motorCW, LOW);
      }  
      if (IrReceiver.decodedIRData.command == 0x3) { // volume down 551534655
        digitalWrite(motorCCW, HIGH);
        delay(200);  // CCW Motor Rotation Time
        digitalWrite(motorCCW, LOW);
      } 
    }                                   
  delay(debounceIR);
  IrReceiver.resume();      
  digitalWrite(LED_BUILTIN, LOW);
  }
}


void setup()
{
  // GPIO 	
  pinMode(LED_BUILTIN, OUTPUT);  
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(mutePin, OUTPUT);
  digitalWrite(mutePin, LOW); 
  pinMode(motorCW, OUTPUT);
  digitalWrite(motorCW, LOW);
  pinMode(motorCCW, OUTPUT);
  digitalWrite(motorCCW, LOW);
  delay(250); 
    // IR remote
  IrReceiver.begin(IRpin);  
  delay(500); 
}
 
void loop() {
  irReceive();
}

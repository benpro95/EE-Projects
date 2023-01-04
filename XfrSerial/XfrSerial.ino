char serialin;

#define relay 8
#include <SoftwareSerial.h>


void setup() {
  //Begin Serial Comunication(configured for 9600baud)
  Serial.begin(9600);
  //pin relay as OUTPUT
  pinMode(relay, OUTPUT); 
  //Relay 
  digitalWrite(relay,LOW);
  }

void loop() {
  //Verify connection by serial
  while (Serial.available() > 0) {
    //Read Serial data and alocate on serialin
    serialin = Serial.read();
    //If serialin is equal as 'o' or 'O' LAMP OFF
    if (serialin == '0' || serialin =='O'){ // Two Pipeines(||) to make a boolean OR Comparission
      digitalWrite(relay,LOW); 
      }
    /*serialin is equal as 'c' or C */
    else if (serialin == '1' || serialin =='C'){ // Two Pipeines(||) to make a boolean OR Comparission
      digitalWrite(relay,HIGH);
      }
      Serial.println(serialin);
  }
}

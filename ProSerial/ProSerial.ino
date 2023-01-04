// Ben Provenzano III - 09/14/22
//A very simple example of sending a string of characters
//from the serial monitor, capturing the individual
//characters into a String, then evaluating the contents
//of the String to perform an action.
 
#include <IRremote.h>
IRsend irsend;

int Relay1 = 8;
int Relay2 = 9;
int Relay3 = 10;
int Relay4 = 11;

String readString;

void setup() {
  Serial.begin(9600);
  pinMode(Relay1, OUTPUT);
  pinMode(Relay2, OUTPUT);
  pinMode(Relay3, OUTPUT);
  pinMode(Relay4, OUTPUT);
  Serial.println("ProSerial - v003"); // so I can keep track
}

void loop() {

  while (Serial.available()) {
    delay(3); 
    char c = Serial.read();
    readString.concat(c);
  //  readString += c;
  }

  if (readString.length() >0) {
    Serial.println("*************");
    Serial.println(readString);
    readSerial(); 
    readString="";
    Serial.println("*************");
  }
}


void readSerial() {
//////////////////////
  if(readString.indexOf("on1") >=0)
  {
    digitalWrite(Relay1, HIGH);
    Serial.println("Relay1 ON");
    return;
  }
//////////////////////
  if(readString.indexOf("off1") >=0)
  {
    digitalWrite(Relay1, LOW);
    Serial.println("Relay1 OFF");
    return;
  }
//////////////////////
  if(readString.indexOf("on2") >=0)
  {
    digitalWrite(Relay2, HIGH);
    Serial.println("Relay2 ON");
    return;
  }
//////////////////////
  if(readString.indexOf("off2") >=0)
  {
    digitalWrite(Relay2, LOW);
    Serial.println("Relay2 OFF");
    return;
  }
////////////////////////
  if(readString.indexOf("on3") >=0)
  {
    digitalWrite(Relay3, HIGH);
    Serial.println("Relay3 ON");
    return;
  }
//////////////////////
  if(readString.indexOf("off3") >=0)
  {
    digitalWrite(Relay3, LOW);
    Serial.println("Relay3 OFF");
    return;
  }
////////////////////////
  if(readString.indexOf("on4") >=0)
  {
    digitalWrite(Relay4, HIGH);
    Serial.println("Relay4 ON");
    return;
  }
//////////////////////
  if(readString.indexOf("off4") >=0)
  {
    digitalWrite(Relay4, LOW);
    Serial.println("Relay4 OFF");
    return;
  }
}

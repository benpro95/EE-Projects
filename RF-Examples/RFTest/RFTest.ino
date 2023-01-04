/*
 * Ben Provenzano III
 * -----------------
 * Oct. 24th 2015
 * Serial Home Automation / DAM1021 Input Controller
 * v2
 *
 */





#define LED2 2
#define LED3 3
#define LED4 4
#define LED5 5
#define LED6 6
#define LED7 7
#define LED8 8
#define LED9 9

int input = 0;       // variable to keep the data from the serial port

void setup() {
  pinMode(LED2,OUTPUT);    // declare the LED's pin as output
  pinMode(LED3,OUTPUT);    // declare the LED's pin as output
  pinMode(LED4,OUTPUT);    // declare the LED's pin as output
  pinMode(LED5,OUTPUT);    // declare the LED's pin as output
  pinMode(LED6,OUTPUT);    // declare the LED's pin as output
  pinMode(LED7,OUTPUT);    // declare the LED's pin as output
  pinMode(LED8,OUTPUT);    // declare the LED's pin as output
  pinMode(LED9,OUTPUT);    // declare the LED's pin as output
  Serial.begin(9600);        // connect to the serial port
}

void loop () {
  input = Serial.read();      // read the serial port
  
    if (input == '0'){
      digitalWrite(LED4, LOW);
      digitalWrite(LED9, LOW);
  } 
    if (input == '1'){
      digitalWrite(LED4, LOW);
      digitalWrite(LED9, HIGH);
  }
    if (input == '2'){
      digitalWrite(LED9, LOW);
      digitalWrite(LED4, HIGH);
  } 
    if (input == '3'){
      digitalWrite(LED2, HIGH);
  }
    if (input == '4'){
      digitalWrite(LED2, LOW);
  } 
    if (input == '5'){
      digitalWrite(LED3, HIGH);
  }
    if (input == '6'){
      digitalWrite(LED3, LOW);
  }
    if (input == '7'){
      digitalWrite(LED5, HIGH);
  } 
    if (input == '8'){
      digitalWrite(LED5, LOW);
  }
    if (input == '9'){
      digitalWrite(LED6, HIGH);
  } 
    if (input == '11'){
      digitalWrite(LED6, LOW);
  }
    if (input == '12'){
      digitalWrite(LED7, HIGH);
  }
    if (input == '13'){
      digitalWrite(LED7, LOW);
  } 
    if (input == '14'){
      digitalWrite(LED8, HIGH);
  }
    if (input == '15'){
      digitalWrite(LED8, LOW);
  }

}

// Wire Peripheral Receiver
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Receives data as an I2C/TWI Peripheral device
// Refer to the "Wire Master Writer" example for use with this

// Created 29 March 2006

// This example code is in the public domain.


#include <Wire.h>

void setup()
{
  Wire.begin(0x3E);             // join i2c bus 
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(9600);           // start serial for output
}

void loop()
{

}

void receiveEvent(int howMany)
{
 while (0 <Wire.available()) {
    int c = Wire.read();      /* receive byte as a character */
    Serial.print(c, BIN);           /* print the character */
  }
 Serial.println();             /* to newline */
}

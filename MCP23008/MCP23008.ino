#include <Wire.h>

void setup()
{
Wire.begin(); //creates a Wire object

// set I/O pins to outputs
Wire.beginTransmission(0x27); //begins talking to the slave device
Wire.write(0x00); //selects the IODIRA register
Wire.write(0x00); //this sets all port A pins to outputs
Wire.endTransmission(); //stops talking to device
}

void loop()
{
Wire.beginTransmission(0x27); //starts talking to slave device
Wire.write(0x09); //selects the GPIO pins
Wire.write(0xff); // turns on pins 0 and 1 of GPIOA
Wire.endTransmission(); //ends communication with the device
}


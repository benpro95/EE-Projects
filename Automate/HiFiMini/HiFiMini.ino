/*
 * Ben Provenzano III
 * -----------------
 * v1 09/03/2022
 433MHz Wireless Amplifier Controller
 *
 */
 
// Libraries
#include <Wire.h>
#include <RCSwitch.h>

// Definitions
#define MAX9744_I2CADDR 0x4B    // 0x4B is the default i2c address
RCSwitch mySwitch = RCSwitch(); // 433Mhz receiver

// Constants
const int potPin = A0;          // pin A0 to read analog input
int potThresh = 20;

// Variables
unsigned long rfvalue = 0;
int _maxByte = 63;
int theVol = 0; 
int theLastVol = 0; 
int maxRange = 512;
int potVal = 0;  
int potFinal = 0; 
int lastPotVal = 0;
int potMaxRange = 502;
bool changeVol = 0;
bool vMute = 0;
int volFine = 10;
int volSemiCourse = 30;
int volCourse = 50;
int muteVol = 0;

void setup() {
// RS232  
  Serial.begin(9600);
  Serial.println("MAX9744 amp controller");
// I2C  
  Wire.begin();
  if (! setvolume(theVol)) {
    Serial.println("Failed to set volume!");
    while (1);
  }
// 433MHz RF   
  delay(500); 
  mySwitch.enableReceive(0);  // Receiver on interrupt 0 => that is [pin #2]
}

// Main
void loop() {
  receiveRF();
  readPot();   
  if (changeVol == 1) {
    if (theVol > maxRange) theVol = maxRange;
    if (theVol < 0) theVol = 0;
    if (theVol != theLastVol) {
      Serial.print("Volume range: ");
      Serial.println(theVol);
      unsigned long _vol = map(theVol, maxRange, 0, _maxByte, 0);
      setvolume(_vol);   
      changeVol = 0;
      theLastVol = theVol;
    }  
  }
}

// write the 6-bit volume to the I2C bus
boolean setvolume(int8_t vol) {
  // cant be higher than 63 or lower than 0
  if (vol > _maxByte) vol = _maxByte;
  if (vol < 0) vol = 0;
  Serial.print("Setting volume to ");
  Serial.println(vol);
  Serial.println("-----------");
  Wire.beginTransmission(MAX9744_I2CADDR);
  Wire.write(vol);
  if (Wire.endTransmission() == 0) 
    return true;
  else
    return false;
}

void readPot() {
    potVal = 0;  
    potFinal = 0; 
    // Read analog value from potentiometer
    potVal = analogRead(potPin);             
    // Map raw values to 0-512 range
    potVal = map(potVal, 0, potMaxRange, (maxRange * 2), 0);
    // Return value if pot changes more than threshold 
    if(abs(potVal - lastPotVal) >= potThresh){
      potFinal = (potVal/2);       
      lastPotVal = potVal; 
      // Change the volume to the pot value
      theVol = potFinal;
      if (vMute == 0) {
        changeVol = 1;
      }    
    }
}

void receiveRF() {
  if (mySwitch.available()) {
    rfvalue = mySwitch.getReceivedValue();
    if (rfvalue == 696912) //
    {
      // Volume Up (Fine)
      theVol = theVol + volFine;
      changeVol = 1;
    }  
    if (rfvalue == 696913) //
    {
      // Volume Down (Fine)
      theVol = theVol - volFine;
      changeVol = 1;
    } 
    if (rfvalue == 696922) //
    {
      // Volume Up (Semi-Course)
      theVol = theVol + volSemiCourse;
      changeVol = 1;  
    }
    if (rfvalue == 696923) //
    {
      // Volume Down (Semi-Course)
      theVol = theVol - volSemiCourse;
      changeVol = 1;
    } 
    if (rfvalue == 696932) //
    {
      // Volume Up (Course)
      theVol = theVol + volCourse;
      changeVol = 1;
    }
    if (rfvalue == 696933) //
    {
      // Volume Down (Course)
      theVol = theVol - volCourse;
      changeVol = 1;
    } 
    if (rfvalue == 696905) //
    {
      // Set Level (Quietest)
      theVol = 50;
      changeVol = 1;
    }    
    if (rfvalue == 696910) //
    {
      // Set Level
      theVol = 100;
      changeVol = 1;
    }
    if (rfvalue == 696920) //
    {
      // Set Level
      theVol = 200;
      changeVol = 1;
    }
    if (rfvalue == 696930) //
    {
      // Set Level
      theVol = 300;
      changeVol = 1;
    }
    if (rfvalue == 696940) //
    {
      // Set Level
      theVol = 400;
      changeVol = 1;
    }
    if (rfvalue == 696997) //
    {
      // Set Level (Loudest)
      theVol = 500;
      changeVol = 1;
    }
    // Mute
    if (vMute == 1) {
      changeVol = 0;
    }     
    if (rfvalue == 696999) // unmute
    { 
      if (vMute == 1) {
        theVol = muteVol;
        theLastVol = -1;
        vMute = 0;
        changeVol = 1;
      } 
    }    
    if (rfvalue == 696944) // mute
    { 
      if (vMute == 0) {
        vMute = 1;
        muteVol = theVol;
        theVol = 0;
        changeVol = 1;
      }
    }  
    mySwitch.resetAvailable();
  }
}

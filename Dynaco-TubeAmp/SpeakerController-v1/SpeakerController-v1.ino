///////////////////////////////////////////////////////////////////////////////
// Speaker Controller v1.0 
// by Ben Provenzano III
// 11/14/2022
///////////////////////////////////////////////////////////////////////////////


// Libraries //
#include "LiquidCrystal_I2C.h" // custom for MCP23008-E/P, power button support
#include <IRremote.hpp> // v3+
#include <Arduino.h>
#include <Wire.h> 

// I2C addresses
#define lcdAddr 0x27

// 16x2 Display
LiquidCrystal_I2C lcd(lcdAddr);
uint8_t lcdCols = 16; // number of columns in the LCD
uint8_t lcdRows = 2;  // number of rows in the LCD
#define lcdBacklightPin 9 // display backlight pin
uint8_t lcdOffBrightness = 0; // standby-off LCD brightness level ***
uint8_t lcdOnBrightness = 255; // online LCD brightness level ***
// custom characters
uint8_t bar1[8] = {0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10};
uint8_t bar2[8] = {0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18};
uint8_t bar3[8] = {0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C};
uint8_t bar4[8] = {0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E};
uint8_t bar5[8] = {0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F};
uint8_t speaker[8] = {B00001,B00011,B01111,B01111,B01111,B00011,B00001,B00000};
uint8_t sound[8] = {B00001,B00011,B00101,B01001,B01001,B01011,B11011,B11000};

// IR
int IRpin = 8; // receiver in TSOP34838YA1
bool irCodeScan = 0; // enable IR codes on the terminal (1)
uint8_t debounceIR = 75; // IR receive max rate (ms)
uint32_t irRecv;

// Front Panel Control
#define selectPin 1 // A1 analog input
#define functionPin 2 // A2 analog input
#define subPin 3 // A3 analog input
uint8_t selectButton = 0;
uint8_t lastSelectButton = 0;
uint32_t selectButtonMillis;
uint8_t functionButton = 0;
uint8_t lastFunctionButton = 0;
uint32_t functionButtonMillis;
uint8_t subButton = 0;
uint8_t lastSubButton = 0;
uint32_t subButtonMillis;

// Power
#define powerRelayPin 7 
#define powerTriggerPin 0 // A0 analog input
#define powerButtonPin 5 // on MCP chip
uint8_t powerButton = 0;
uint8_t lastPowerButton = 0;
uint32_t powerButtonMillis;
uint8_t powerTrigger = 0;
uint8_t lastPowerTrigger = 0;
uint32_t powerTriggerMillis;
int startDelay = 3; // startup delay in seconds, unmutes after ***
int shutdownTime = 3; // shutdown delay before turning off aux power ***
int initStartDelay = 3; // delay on initial cold start
uint8_t debounceDelay = 50; // button debounce delay in ms
bool powerLock = 0;
bool powerCycle = 0;
bool powerState = 0;


///////////////////////////////////////////////////////////////////////////////


// reverse byte function 
int reverseByte(uint8_t _inByte) 
{
  uint8_t _revByte;
  for (int _revBit = 0; _revBit < 8; _revBit++) {
    if (bitRead(_inByte, _revBit) == 1) {
      bitWrite(_revByte, (7 - _revBit), 1);
    } else {
      bitWrite(_revByte, (7 - _revBit), 0);
    }
  }
  return _revByte;
}


// mapping function (fixed)
long l_map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min + 1) /
         (in_max - in_min + 1) + out_min;
} 


// display a progress bar
void lcdBar (int row, int var, int minVal, int maxVal)
{
  int block = map(var, minVal, maxVal, 0, 16); // Block represent the current LCD space
  int line = map(var, minVal, maxVal, 0, 80); // Line represent the lines to be displayed
  int bar = (line-(block*5)); // Bar represent the actual lines that will be printed
  // Print all the filled blocks
  for (int x = 0; x < block; x++)                       
  {
    lcd.setCursor (x, row);
    lcd.write (1023);
  }
  // Set the cursor at the current block and print the numbers of line needed
  lcd.setCursor (block, row);                           
  if (bar != 0) lcd.write (bar);
  if (block == 0 && line == 0) lcd.write (1022); // Unless there is nothing to print, show blank
  // Print all the blank blocks
  for (int x = 16; x > block; x--) 
  {
    lcd.setCursor (x, row);
    lcd.write (1022);
  }
}


// display progress bar for # of seconds 
void lcdTimedBar(uint8_t _sec, bool _doublebar)
{ // seconds to loops
  int _loops = _sec * 32;
  for ( int _incr = 0; _incr < _loops; _incr++ ) {
  	if (_doublebar == 1) {
      lcdBar(0,_incr,0,_loops);
    }
    lcdBar(1,_incr,0,_loops);
    _incr = _incr + 4;
    delay(40);
  }    
}  


// display in standby mode
void lcdStandby()
{ 
  analogWrite(lcdBacklightPin, lcdOffBrightness);	
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Speaker");
  lcd.setCursor(0,1);
  lcd.print("Controller");
  lcd.setCursor(10,0);  
  lcd.createChar(0, speaker);
  lcd.setCursor(15,1);
  lcd.print(char(0));
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
}  


// receive IR remote commands 
// remote control (Apple TV remote)
// commented codes are for Xmit
void irReceive() 
{
  if (IrReceiver.decode()) {
    digitalWrite(LED_BUILTIN, HIGH); 
    if (IrReceiver.decodedIRData.address == 0xCE) { // remote address        
    // code detected
      if (powerState == 1) { // powered on state
    	  if (IrReceiver.decodedIRData.command == 0x5C) { // center button
          lcd.setCursor(0,0);
          lcd.print("CENTER  ");      	
    	  }      
    	  if (IrReceiver.decodedIRData.command == 0xA) { // up button
          lcd.setCursor(0,0);
          lcd.print("UP      "); 	 
    	  }	  
    	  if (IrReceiver.decodedIRData.command == 0xC) { // down button
          lcd.setCursor(0,0);
          lcd.print("DOWN    "); 	 
    	  }       
        if (IrReceiver.decodedIRData.command == 0x9) { // left button
          Serial.println("Left button");
        }   
        if (IrReceiver.decodedIRData.command == 0x6) { // right button
          Serial.println("Right button"); 
        }     
        if (IrReceiver.decodedIRData.command == 0x3) { // menu button
          lcd.setCursor(0,0);
          lcd.print("MENU    ");   
        }                                
      }   
      if (powerLock == 0) { // powered off state & not locked
        if (IrReceiver.decodedIRData.command == 0x5F) { // play/pause button
          Serial.println("Play/pause button - powering on..."); 
          powerState = !powerState;  
          powerCycle = 1;
        }  
      }  
    }          
    // display IR codes on terminal
    if (irCodeScan == 1) {   
      // display IR codes on terminal   
      Serial.println("--------------");   
      if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
        IrReceiver.printIRResultRawFormatted(&Serial, true);  
      } else {
        IrReceiver.printIRResultMinimal(&Serial); 
      }  
      Serial.println("--------------"); 
      irRecv = IrReceiver.decodedIRData.decodedRawData;
      uint32_t irtype = IrReceiver.decodedIRData.protocol;
      // display v3 format codes
      Serial.print("IR protocol: "); 
      Serial.println(irtype);       
      int32_t irRecv_2s = -irRecv;
      irRecv_2s = irRecv_2s * -1;
      Serial.print("IR v3 decimal: "); 
      Serial.println(irRecv, DEC); 
      Serial.print("IR v3 (2's-compliment) decimal: "); 
      Serial.println(irRecv_2s, DEC);  
      Serial.print("IR v3 hex: "); 
      Serial.println(irRecv, HEX); 
      // convert 32-bit code into 4 seperate bytes (pointer)
      uint8_t *irbyte = (uint8_t*)&irRecv;
      Serial.print("IR v3 address: "); 
      Serial.println(irbyte[3], HEX); 
      Serial.print("IR v3 command: "); 
      Serial.println(irbyte[2], HEX); 
      Serial.println("--------------"); 
      // reverse each byte (v3 format to v2 format)
      uint8_t irbyte0 = reverseByte(irbyte[0]); 
      uint8_t irbyte1 = reverseByte(irbyte[1]); 
      uint8_t irbyte2 = reverseByte(irbyte[2]); 
      uint8_t irbyte3 = reverseByte(irbyte[3]); 
      // assemble 4 reversed bytes into 32-bit code
      uint32_t irv2 = irbyte0; // shift in the first byte
      irv2 = irv2 * 256 + irbyte1; // shift in the second byte
      irv2 = irv2 * 256 + irbyte2; // shift in the third byte
      irv2 = irv2 * 256 + irbyte3; // shift in the last byte
      // display v2 format codes
      int32_t irv2_2s = -irv2;
      irv2_2s = irv2_2s * -1;
      Serial.print("IR v2 decimal: "); 
      Serial.println(irv2, DEC); 
      Serial.print("IR v2 (2's-compliment) decimal: "); 
      Serial.println(irv2_2s, DEC);  
      Serial.print("IR v2 hex: "); 
      Serial.println(irv2, HEX);
      Serial.println(" "); 
      Serial.println("v2 2's-comp is used by Automate/Xmit");
      Serial.println("ignore decimal values with Sony codes, use hex");
      Serial.println(" ");
    }
  delay(debounceIR);
  IrReceiver.resume();      
  digitalWrite(LED_BUILTIN, LOW);   
  }
}


// power on/off
void setPowerState() {
  // read pin state from MCP23008 //////////////////
  int reading = lcd.readPin(powerButtonPin);
  // if switch changed
  if (reading != lastPowerButton) {
    // reset the debouncing timer
    powerButtonMillis = millis();
  }
  if ((millis() - powerButtonMillis) > debounceDelay) {
  	if (powerLock == 0) {
      // if the button state has changed:
      if (reading != powerButton) {
        powerButton = reading;
        // power state has changed!
        if (powerButton == 1) { 
          powerState = !powerState;  
          powerCycle = 1;
        }
      }
    }  
  }
  lastPowerButton = reading; 
  // read power trigger ////////////////////////////
  int val = analogRead(powerTriggerPin);
  reading = 1;
  if (val >= 512) {
	reading = 0;
  }
  // if input changed
  if (reading != lastPowerTrigger) {
    // reset the debouncing timer
    powerTriggerMillis = millis();
  }
  if ((millis() - powerTriggerMillis) > debounceDelay) {
    // if the input state has changed:
    if (reading != powerTrigger) {
      powerTrigger = reading;
      if (powerTrigger == 1){ 
        powerState = 1;  
        powerCycle = 1;
        powerLock = 1;
      } else {  
        powerState = 0;  
        powerCycle = 1;  
        powerLock = 0;
      }          
    }
  }
  lastPowerTrigger = reading;
  // power state actions ///////////////////////////
  if (powerCycle == 1){    	  
	// reset display
	lcd.clear();
	// one-shot triggers
    if (powerState == 1){
      /// runs once on boot ///
      startup();
    } else {  
      /// runs once on shutdown ///
      shutdown();
    }  
  powerCycle = 0;  
  }  
}


void readFrontPanel() {
  if (powerState == 1) { // powered on state
    // read select button
    int val = analogRead(selectPin);
    int reading = 1;
    if (val >= 512) {
    reading = 0;
    }
    // if input changed
    if (reading != lastSelectButton) {
      // reset the debouncing timer
      selectButtonMillis = millis();
    }
    if ((millis() - selectButtonMillis) > debounceDelay) {
      // if the input state has changed:
      if (reading != selectButton) {
        selectButton = reading;
        if (selectButton == 1) {
          Serial.println("select: ");  
        }   
      }
    }
    lastSelectButton = reading; 
    // read function button
    val = analogRead(functionPin);
    reading = 1;
    if (val >= 512) {
    reading = 0;
    }
    // if input changed
    if (reading != lastFunctionButton) {
      // reset the debouncing timer
      functionButtonMillis = millis();
    }
    if ((millis() - functionButtonMillis) > debounceDelay) {
      // if the input state has changed:
      if (reading != functionButton) {
        functionButton = reading;
        if (functionButton == 1) {
          Serial.println("function: ");   
        }  
      }
    }
    lastFunctionButton = reading; 
    // read subwoofer button
    val = analogRead(subPin);
    reading = 1;
    if (val >= 512) {
    reading = 0;
    }
    // if input changed
    if (reading != lastSubButton) {
      // reset the debouncing timer
      subButtonMillis = millis();
    }
    if ((millis() - subButtonMillis) > debounceDelay) {
      // if the input state has changed:
      if (reading != subButton) {
        subButton = reading;
        if (subButton == 1) {
          Serial.println("sub: ");  
        }   
      }
    }
    lastSubButton = reading; 
  }  
}


void shutdown() {	
  // shutdown animation
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Shutting Down..."); 	
  lcdTimedBar(shutdownTime,0);	
  lcdStandby();
  // turn off preamp power here
  digitalWrite(powerRelayPin, LOW);  	
}

// startup routines
void startup() 
{ 
  analogWrite(lcdBacklightPin, lcdOnBrightness);
  lcd.setCursor(0,0);
  lcd.print("Starting Up...");
  // turn on preamp power here
  digitalWrite(powerRelayPin, HIGH);  
  delay(650);
  // loading bar 
  lcdTimedBar(startDelay,0);
  lcd.clear();
  // music icon
  lcd.createChar(0, sound);
  lcd.setCursor(0,1);
  lcd.print(char(0));
}

///////////////////////////////////////////////////////////////////////////////
// initialization 
void setup() 
{ 	
  // print IR codes to serial port
  //irCodeScan = 1;
  // 16x2 display (calls Wire.begin)
  lcd.begin(lcdCols,lcdRows); 	
  // LED status LED
  pinMode(LED_BUILTIN, OUTPUT);  
  digitalWrite(LED_BUILTIN, HIGH);    
  // display backlight
  pinMode(lcdBacklightPin, OUTPUT);  
  analogWrite(lcdBacklightPin, lcdOffBrightness);
  // preamp power control
  pinMode(powerRelayPin, OUTPUT);  
  digitalWrite(powerRelayPin, LOW);
  // IR remote
  IrReceiver.begin(IRpin);  
  // initial boot display
  lcd.createChar(1, bar1);
  lcd.createChar(2, bar2);
  lcd.createChar(3, bar3);
  lcd.createChar(4, bar4);
  lcd.createChar(5, bar5);
  lcdTimedBar(initStartDelay,1);
  // serial support
  Serial.begin(9600);
  // draw standby screen
  lcdStandby();
}

///////////////////////////////////////////////////////////////////////////////
// superloop
void loop()
{
  // IR remote
  irReceive();
  // power management
  setPowerState();
  // respond to front panel buttons	  
  readFrontPanel();
}

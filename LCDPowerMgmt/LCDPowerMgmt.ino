///////////////////////////////////////////////////////////////////////////////
// by Ben Provenzano III
///////////////////////////////////////////////////////////////////////////////


// Libraries //
#include "LiquidCrystal_I2C.h" // custom for MCP23008-E/P, power button support
#include <Arduino.h>
#include <Wire.h> 

// I2C addresses
#define lcdAddr 0x27

// 16x2 Display
LiquidCrystal_I2C lcd(lcdAddr);
uint8_t lcdCols = 16; // number of columns in the LCD
uint8_t lcdRows = 2;  // number of rows in the LCD
#define lcdBacklightPin 9 // display backlight pin
uint8_t lcdOffBrightness = 255; // standby-off LCD brightness level ***
uint8_t lcdOnBrightness = 150; // online LCD brightness level ***
// Custom Characters
uint8_t bar1[8] = {0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10};
uint8_t bar2[8] = {0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18};
uint8_t bar3[8] = {0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C};
uint8_t bar4[8] = {0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E};
uint8_t bar5[8] = {0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F};
uint8_t speaker[8] = {B00001,B00011,B01111,B01111,B01111,B00011,B00001,B00000};
uint8_t sound[8] = {B00001,B00011,B00101,B01001,B01001,B01011,B11011,B11000};

// Power Control
#define powerButtonPin 5 // on MCP chip
uint8_t powerButton = 0;
uint8_t lastPowerButton = 0;
uint32_t powerButtonMillis;
int startDelay = 2; // startup delay in seconds, unmutes after ***
int shutdownTime = 2; // shutdown delay before turning off aux power ***
int initStartDelay = 2; // delay on initial cold start
uint8_t debounceDelay = 50; // button debounce delay in ms
bool powerLock = 0;
bool powerCycle = 1; // set this to 1 for auto power-on / 0 normal 
bool powerState = 1; // set this to 1 for auto power-on / 0 normal 


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
  // power state actions ///////////////////////////
  if (powerCycle == 1){    	  
	// reset display
	lcd.clear();
	// one-shot triggers
    if (powerState == 1){
      /// runs once on boot ///
      Serial.println("powering on..."); 
      startup();
    } else {  
      /// runs once on shutdown ///
      Serial.println("shutting down...");  
      shutdown();
    }  
  powerCycle = 0;  
  }  
}

void shutdown() {	
  // shutdown animation
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Shutting Down..."); 	
  lcdTimedBar(shutdownTime,0);	
  lcdStandby();
}

// startup routines
void startup() 
{ 
  analogWrite(lcdBacklightPin, lcdOnBrightness);
  lcd.setCursor(0,0);
  lcd.print("Starting Up...");
  // loading bar 
  lcdTimedBar(startDelay,0);
  lcd.clear();
  // music icon
  lcd.createChar(0, sound);
  lcd.setCursor(15,1);
  lcd.print(char(0));
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

///////////////////////////////////////////////////////////////////////////////
// initialization 
void setup() 
{ 	
  // 16x2 display (calls Wire.begin)
  lcd.begin(lcdCols,lcdRows); 	
  // LED status LED
  pinMode(LED_BUILTIN, OUTPUT);  
  digitalWrite(LED_BUILTIN, HIGH);    
  // display backlight
  pinMode(lcdBacklightPin, OUTPUT);  
  analogWrite(lcdBacklightPin, lcdOffBrightness);
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
  // power management
  setPowerState();
}

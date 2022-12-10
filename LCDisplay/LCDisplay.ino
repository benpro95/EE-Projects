///////////////////////////////////////////////////////////////////////////////
// by Ben Provenzano III
///////////////////////////////////////////////////////////////////////////////


// Libraries //
#include "LiquidCrystal_I2C.h" // custom for MCP23008-E/P, power button support
#include <RCSwitch.h>
#include <Wire.h> 

// I2C addresses
#define lcdAddr 0x27

char* lcdChars[]={" ","0","1","2","3","4","5","6","7","8","9","a","b","c","d","e","f","g","h","i","j","k","l","m","n","o","p","q","r","s","t","u","v","w","x","y","z","A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z","+","=","-","%","(",")",":"};

// 16x2 Display
LiquidCrystal_I2C lcd(lcdAddr);
uint8_t lcdCols = 16; // number of columns in the LCD
uint8_t lcdRows = 2;  // number of rows in the LCD
#define lcdBacklightPin 9 // display backlight pin
uint8_t lcdOffBrightness = 75; // standby-off LCD brightness level ***
uint8_t lcdOnBrightness = 150; // online LCD brightness level ***
// Custom Characters
uint8_t bar1[8] = {0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10};
uint8_t bar2[8] = {0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18};
uint8_t bar3[8] = {0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C};
uint8_t bar4[8] = {0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E};
uint8_t bar5[8] = {0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F};
#define lcdScrollSpeedSlow 60
uint8_t lcdCharLimit = 100;
uint8_t arraySize = 0;
uint8_t rowCount0 = 0;
uint8_t rowCount1 = 0;
String charBuffer0;
String charBuffer1;
String lastChars;

// RF Control
RCSwitch mySwitch = RCSwitch();
uint32_t RFdebounce = 500; // button debounce delay in ms
uint32_t RFMillis;
String lastRFvalue;

// Power Control
#define powerButtonPin 5 // on MCP chip
uint8_t powerButton = 0;
uint8_t lastPowerButton = 0;
uint32_t powerButtonMillis;
uint8_t powerDelay = 1; // startup delay in seconds, unmutes after ***
uint8_t initStartDelay = 1; // delay on initial cold start
uint8_t debounceDelay = 50; // button debounce delay in ms
bool powerCycle = 1; // set this to 1 for auto power-on / 0 normal 
bool powerState = 1; // set this to 1 for auto power-on / 0 normal 
bool powerLock = 0;

///////////////////////////////////////////////////////////////////////////////


void receiveRF() {
  if (mySwitch.available()) {
    // store RF data
    String rfvalue;
    rfvalue = mySwitch.getReceivedValue();
    if (rfvalue.length() == 6) { // only process 6-bit data
      if (rfvalue != lastRFvalue) {       
        // split up incoming data 
        String _id = rfvalue.substring(0,3);
        String _schar = rfvalue.substring(3,5);
        String _sline = rfvalue.substring(5,6);
        // convert to integers 
        uint8_t _char = _schar.toInt();
        uint8_t _line = _sline.toInt();
        // detect correct RF ID (000xxx)
        if (_id == "828") {
          // read last argument (xxxxx0)
          if (_line == 3) {
            clearLCD(1);
          }
          if (_line == 2) {
            clearLCD(0);
          }
          if (_line <= 1) {
            // draw character (xxx00x)
            drawChar(_line,_char);
          }
        }  
      } 
      lastRFvalue = rfvalue;  
    }
    mySwitch.resetAvailable();
  }
}

// scroll text on display
void drawChar(bool _line, uint8_t _char) {
  if( _char <= arraySize){
    uint8_t _cursor0;
    uint8_t _cursor1;
    if( _line == 0){
      charBuffer0 += lcdChars[_char];
      if( rowCount0 > lcdCols ){
        // overflow behavior
        _cursor0 = lcdCols;
        lastChars = charBuffer0.substring(rowCount0 - lcdCols, rowCount0);
        lcd.setCursor(0, _line);
        lcd.print(lastChars);
      } else {
        _cursor0 = rowCount0;
      } // position cursor   
      lcd.setCursor(_cursor0, _line);
      // count characters
      rowCount0++;
    } else {
      charBuffer1 += lcdChars[_char];
      if( rowCount1 > lcdCols ){
        // overflow behavior
        _cursor1 = lcdCols;
        lastChars = charBuffer1.substring(rowCount1 - lcdCols, rowCount1);
        lcd.setCursor(0, _line);
        lcd.print(lastChars);      
      } else {
        _cursor1 = rowCount1;
      } // position cursor     
      lcd.setCursor(_cursor1, _line);
      // count characters
      rowCount1++;
    } // display a single character from the array
    lcd.print(lcdChars[_char]); 
  } 
  // clear display when character limit exceeded
  if( _line == 0){
    if( rowCount0 > lcdCharLimit){
      lastChars = "";
      rowCount0 = 0;
      charBuffer0 = "";
      for(uint8_t _count = 0; _count < lcdCols + 1; _count++)
      { // draw spaces
        lcd.setCursor(_count, 0);
        lcd.print(" ");
        delay(lcdScrollSpeedSlow);
      }
    }   
  } else {
    if( rowCount1 > lcdCharLimit){
      lastChars = "";
      rowCount1 = 0;
      charBuffer1 = "";
      for(uint8_t _count = 0; _count < lcdCols + 1; _count++)
      { // draw spaces
        lcd.setCursor(_count, 1);
        lcd.print(" ");
        delay(lcdScrollSpeedSlow);
      }      
    }
  }
}

// clear display
void clearLCD(bool _line) 
{
  for(uint8_t _count = 0; _count < lcdCols + 1; _count++)
  { // draw spaces
    lcd.setCursor(_count, _line);
    lcd.print(" ");
    delay(lcdScrollSpeedSlow);
  } // reset lines/rows
  lastChars = "";
  if( _line == 0){
    rowCount0 = 0;
    charBuffer0 = "";
  } else {
    rowCount1 = 0;
    charBuffer1 = "";
  }
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
  int block = map(var, minVal, maxVal, 0, lcdCols); // Block represent the current LCD space
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
      startup();
    } else {  
      /// runs once on shutdown ///
      shutdown();
    }  
  powerCycle = 0;  
  }  
}

void shutdown() { 
  // shutdown animation
  clearLCD(0);
  clearLCD(1);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Shutting Down...");  
  lcdTimedBar(powerDelay,0);  
  standby();
}

// startup routines
void startup() 
{ 
  analogWrite(lcdBacklightPin, lcdOnBrightness);
  lcd.clear();
  lcd.setCursor(0,0);
}

// standby mode
void standby()
{ 
  analogWrite(lcdBacklightPin, lcdOffBrightness); 
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Z-Terminal");
  lcd.setCursor(0,1);
  lcd.print("v1.0");
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
  // calculate number of characters in the array
  arraySize = sizeof(lcdChars) / 2;
  // RF receive  
  mySwitch.enableReceive(0);  // Receiver on interrupt 0 => that is [pin #2]
  mySwitch.setProtocol(1);
  mySwitch.setPulseLength(183);
  // enter standby mode
  standby();
}

///////////////////////////////////////////////////////////////////////////////
// superloop
void loop()
{
  // power management
  setPowerState();
  if (powerState == 1){
    receiveRF();
  }
}

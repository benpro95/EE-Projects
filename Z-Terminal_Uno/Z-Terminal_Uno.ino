 //////////////////////////////////////////////////////////////////////////
// by Ben Provenzano III
//////////////////////////////////////////////////////////////////////////

// Libraries //
#include <Wire.h>
#include <neotimer.h>
#include "LiquidCrystal_I2C.h" // custom for MCP23008-E/P, power button support

//////////////////////////////////////////////////////////////////////////

// LCD Valid Characters
const char lcdChars[]=
	{" 0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ&:',.*|-+=_#@%/[]()<>?{};"};

const int CONFIG_SERIAL = 9600;

// Set Button
#define setButtonPin 12
unsigned long setButtonMillis = 0;
bool setButtonLast = 0;  
bool setButton = 0;

// Power Control
#define clearButtonPin 5 // on MCP chip
bool clearButton = 0;
bool lastclearButton = 0;
unsigned long clearButtonMillis = 0;
uint8_t debounceDelay = 50; // button debounce delay in ms
uint8_t startDelay = 1; // delay on initial start in seconds

// 16x2 LCD Display
#define lcdAddr 0x27 // I2C address
LiquidCrystal_I2C lcd(lcdAddr);
const uint8_t lcdCols = 16; // number of columns in the LCD
const uint8_t lcdRows = 2;  // number of rows in the LCD
// Custom Characters (progress bar)
uint8_t bar1[8] = {0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10};
uint8_t bar2[8] = {0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18};
uint8_t bar3[8] = {0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C};
uint8_t bar4[8] = {0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E};
uint8_t bar5[8] = {0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F};
#define lcdBacklight 9 // display backlight pin
Neotimer lcdDelayTimer = Neotimer();
Neotimer lcdDimmer = Neotimer(80000); // ms before dimming display backlight
const uint8_t lcdClearCharSpeed = 50; // ms delay between drawing each character (clearing display)
const uint32_t lcdDefaultDelay = 325; // ms delay used when none is specified
uint8_t charBuffer0[20]; // trailing character buffer (row 0)
uint8_t charBuffer1[20]; // trailing character buffer (row 1)
uint8_t chrarSize = 0; // character array size
uint8_t rowCount0 = 0; // collumn count (row 0)
uint8_t rowCount1 = 0; // collumn count (row 1)

// Shared resources
#define maxMessage 128
char lcdMessage[maxMessage];
uint32_t lcdMessageStart = 0;
uint32_t lcdMessageEnd = 0;
bool eventlcdMessage = 0;
uint32_t lcdDelay = 0;
uint8_t lcdReset = 0;  

//////////////////////////////////////////////////////////////////////////
// Enable Serial Messages (0 = off) (1 = on)
#define DEBUG 0
/////////////////
#if DEBUG == 1
#define debugstart(x) Serial.begin(x)
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debugstart(x)
#define debug(x)
#define debugln(x)
#endif

//////////////////////////////////////////////////////////////////////////
// initialization
void setup() {
  // calculate number of characters
  chrarSize = sizeof(lcdChars);  
  // 16x2 display (calls Wire.begin)
  lcd.begin(lcdCols,lcdRows);   
  lcd.createChar(1, bar1);
  lcd.createChar(2, bar2);
  lcd.createChar(3, bar3);
  lcd.createChar(4, bar4);
  lcd.createChar(5, bar5);
  lcd.clear();
  // loading bar
  lcdTimedBar(startDelay);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Z-Terminal");   
  lcd.setCursor(0,1);
  lcd.print("v1.2");
  delay(500);
  lcd.clear();
  lcd.setCursor(0,0);
  // built-in LED
  pinMode(LED_BUILTIN, OUTPUT);  
  digitalWrite(LED_BUILTIN, HIGH);    
  // display backlight (low)
  pinMode(lcdBacklight, OUTPUT);  
  digitalWrite(lcdBacklight, LOW);
  // set button
  pinMode(setButtonPin, INPUT_PULLUP);
  // start serial
  Serial.begin(CONFIG_SERIAL);
}

// convert message into character stream 
void lcdMessageEvent() { // (run only from event timer)
  uint32_t _reset = 0;
  uint32_t _end = lcdMessageEnd;
  uint32_t _start = lcdMessageStart;
  uint32_t _delay = lcdDelay; 
  debug("delay data: ");
  debugln(_delay);    
  debug("end position: ");
  debugln(_end);
  uint32_t _charidx;
  debugln("character stream: ");
  // loop through each character in the request array (message only)
  for(uint32_t _idx = _start; _idx < _end; _idx++) { 
    // convert each character into array index positions
    _charidx = (charLookup(lcdMessage[_idx]));
    // read reset state
    _reset = lcdReset;
    charDelay(_delay); // character delay
    drawChar(_charidx,_delay,_reset); // draw each character
    debug(_charidx);
    debug(',');
    // stop drawing if request canceled 
    if (eventlcdMessage == 0) {
      break;
    }    
  }
  debugln(' ');
}

// character search
int charLookup(char _char) {
  int _index;
  for (_index=0; _index <= chrarSize; _index++){
    if (lcdChars[_index] == _char){
      break;
    }
  } // return position 
  return _index;
}

// scroll text on display
void drawChar(uint8_t _char, uint32_t _delay, uint8_t _reset) {
  bool _line = 1; // 1 = text flows bottom to top, 0 = top to bottom
  // clear LCD routine
  if( _reset > 0){
    // disable dimming then reset timer
    lcdDim(); 
    // loop through all display characters
    for(uint8_t _count = 0; _count < lcdCols; _count++) { 
      // draw spaces
      if (_reset > 2) {
        // clear both lines 
        lcd.setCursor(_count, 0);
        lcd.write(' ');
        lcd.setCursor(_count, 1);
      } else {
        // clear a single line
        lcd.setCursor(_count, _reset - 1);
      }  
      lcd.write(' ');
      delay(lcdClearCharSpeed);
    } // reset cursor
    lcd.setCursor(0, _line);
    if( _reset == 1){ // reset row 0
      rowCount0 = 0;
    }
    if( _reset == 2){ // reset row 1
      rowCount1 = 0;
    }
    if( _reset == 3){ // reset both rows
      rowCount0 = 0;
      rowCount1 = 0;
    }   
    lcdReset = 0; // end reset event
    eventlcdMessage = 0; // end message event
    return; 
  }
  // invalid characters
  if( _char > chrarSize - 3){ 
    return;  
  }
   ///////////////////////////////////////////////////////////////////// line 0
  uint32_t _lastidx = 0;
  uint8_t _trlcount;
  // count row position   
  rowCount0++; 
  // drawing behavior
  if( rowCount0 > lcdCols ){ 
    // store last trailing character
    _lastidx = charBuffer0[0];
    // rearrange trailing characters 
    for(uint8_t _idx = 0; _idx <= lcdCols; _idx++) { 
      charBuffer0[_idx] = charBuffer0[_idx + 1];
    }
    // write display output data
    char tmpchr0;
    char datout0[lcdCols + 1]={0}; // temporary array
    for(uint8_t _idx = 0; _idx <= lcdCols - 1; _idx++) { 
      if( _idx == (lcdCols - 1)){
      	tmpchr0 = lcdChars[_char]; // new character 15th
        strncat(datout0, &tmpchr0, 1);
      } else {
      	uint8_t _tmpidx = charBuffer0[_idx]; // 0-14 from buffer
      	tmpchr0 = lcdChars[_tmpidx]; // convert index position
        strncat(datout0, &tmpchr0, 1);
      }
    } // write to display
    lcd.setCursor(0, _line);
    lcd.write(datout0); 
    // lock trailing behavior on after 15th character
    rowCount0 = lcdCols;       
  } else { 
    // before overflow behavior
    if(rowCount0 != 0){ // stops character drawing after clearing display
      lcd.setCursor(rowCount0 - 1, _line);
      lcd.write(lcdChars[_char]);
    }
  }
  // store each character
  charBuffer0[rowCount0 - 1] = _char;
  /////////////////////////////////////////////////////////////////////// line 1     
  _char = _lastidx; // trailing character index from line 0
  _line = !_line; // invert line
  // count row position    
  rowCount1++; 
  // drawing behavior
  if( rowCount1 > lcdCols ){ 
    // overflow behavior
    for(uint8_t _idx = 0; _idx <= lcdCols; _idx++) { 
      charBuffer1[_idx] = charBuffer1[_idx + 1]; // rearrange characters
    }
    // write display output data
    char tmpchr1;
    char datout1[lcdCols + 1]={0}; // temporary array
    for(uint8_t _idx = 0; _idx <= lcdCols - 1; _idx++) { 
      if( _idx == (lcdCols - 1)){
      	tmpchr1 = lcdChars[_char]; // new character 15th
        strncat(datout1,&tmpchr1,1);
      } else {
      	uint8_t _tmpidx = charBuffer1[_idx]; // 0-14 from buffer
      	tmpchr1 = lcdChars[_tmpidx]; // convert index position
        strncat(datout1,&tmpchr1,1);
      }
    } // write to display
    lcd.setCursor(0, _line);
    lcd.write(datout1); 
    // lock trailing behavior on after 15th character
    rowCount1 = lcdCols;       
  } else {
    // before overflow behavior
    if(rowCount1 != 0){ // stops character drawing after clearing display
      lcd.setCursor(rowCount1 - 1, _line);
      lcd.write(lcdChars[_char]);
    }
  }
  // store each character
  charBuffer1[rowCount1 - 1] = _char;
}

// character delay 
void charDelay(uint32_t _delay) {  
  // prevent dimming while drawing
  lcdDimmer.reset();
  lcdDimmer.start(); 
  // set default speed if not in range
  if ((_delay < 5) || (_delay > 4096)) {
    _delay = 5;
  }  
  // restart character delay timer
  lcdDelayTimer.set(_delay);
  lcdDelayTimer.start();
  for(;;) { // adjustable delay	
    mainEvents(); // keep checking for events during delay
    if(lcdDelayTimer.done()){
      lcdDelayTimer.reset();
      break; // exit loop when timer done
	}
  }    
}

// display progress bar for # of seconds 
void lcdTimedBar(int _sec) { 
  uint32_t _ms = _sec * 6; 
  uint32_t _colcount;
  uint32_t _segcount;
  uint8_t _line = 0;
  uint8_t _rowcount;
  // draw bar on each row
  for(_rowcount = 0; _rowcount < 2; _rowcount++) {
  	// draw bar on each collumn
    for(_colcount = 0; _colcount < lcdCols; _colcount++) {
      // draw bar segments	
      for(_segcount = 0; _segcount < 5; _segcount++) { 	
        lcd.setCursor(_colcount,_line);
        // draw custom segment 
        lcd.write(_segcount + 1);
        delay(_ms);
      }    
    } // increment to next row
    _line++;
  }
}  

// clear display button
void readClearButton() {
  // read pin state from MCP23008 //////////////////
  bool reading = lcd.readPin(clearButtonPin);
  // if switch changed
  if (reading != lastclearButton) {
    // reset the debouncing timer
    clearButtonMillis = millis();
  }
  if ((millis() - clearButtonMillis) > debounceDelay) {
    // if button state has changed
    if (reading != clearButton) {
      clearButton = reading;
      // button change event
      if (clearButton == 1) { 
        // clear both lines
        //lcdReset = 3; 
        lcdDelay = 200;
        eventlcdMessage = 1;
        lcdMessageStart = 1;
        lcdMessageEnd = 4;
        lcdMessage[0] = {'|'};
        lcdMessage[1] = {'B'};
        lcdMessage[2] = {'e'};
        lcdMessage[3] = {'n'};
      }
    } 
  }
  lastclearButton = reading; 
}

// read set button
void readSetButton() {
  bool reading = digitalRead(setButtonPin);
  reading = !reading;
  // if switch changed
  if (reading != setButtonLast) {
    // reset the debouncing timer
    setButtonMillis = millis();
  }
  if ((millis() - setButtonMillis) > debounceDelay) {
    // if button state has changed
    if (reading != setButton) {
      setButton = reading;
      // button change event
      //if (setButton == 1) {
       // trigger message
    
      //}
    } 
  }
  setButtonLast = reading; 
}

// runs in main loop and during character delay
void mainEvents() {
  // read GPIO button
  readClearButton();
  // read serial port data
  readSerial();
  // display dim event
  if(lcdDimmer.done()){
  	debugln("dimming backlight...");
    digitalWrite(lcdBacklight, LOW);
    lcdDimmer.reset();
  }
}

void readSerial() {  
  // check if anything available on serial bus
  if (Serial.available() > 0){
    // read next byte to serial buffer
    char inByte = Serial.read();
    if (inByte != '\n' && (lcdMessageEnd < maxMessage - 1)){
      if (inByte == '~'){
        lcdReset = 3;
        return;
      } else {
        if (eventlcdMessage == 0){  
          // add the incoming byte to our message
	      lcdMessage[lcdMessageEnd] = inByte;
	      lcdMessageEnd++;
	      return;
	    }
	  }
	} else {
	  if (eventlcdMessage == 0){  
	    // full message received
	    eventlcdMessage = 1;
	    lcdDelay = 200;
	    return;
	  }  
	}
  }
}

void loop() {
  // read set button
  //readSetButton();
  if (eventlcdMessage == 1) {  
    lcdDim();
    lcdMessageEvent();
    // reset buffer
    eventlcdMessage = 0;
    lcdMessageStart = 0;
    lcdMessageEnd = 0;
    lcdDelay = 0;
    // send ack to computer
	Serial.println('*');
  }
  // clear display event (main only!)
  if( lcdReset > 0) {
    drawChar(0,0,lcdReset);
    // reset buffer
    lcdMessageStart = 0;
    lcdMessageEnd = 0;
    lcdDelay = 0;
    // send ack to computer
    Serial.println('*');
  }
  // ran in main and during delay loop 
  mainEvents();
}
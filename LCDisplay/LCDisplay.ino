///////////////////////////////////////////////////////////////////////////////
// by Ben Provenzano III
///////////////////////////////////////////////////////////////////////////////

// Libraries //
#include <Arduino.h>
#include <WiFi.h>
#include "LiquidCrystal_I2C.h" // custom for MCP23008-E/P, power button support
#include <Wire.h>
#include "esp_task_wdt.h"

// I2C addresses
#define lcdAddr 0x27

// Wi-Fi Configuration
const char* CONFIG_SSID      = "mach_kernel";
const char* CONFIG_PSK       = "phonics.87.reply.218";
const char* HOSTNAME         = "lcd16x2";
const int   CONFIG_SERIAL    = 115200;
const int   CONFIG_PORT      = 80;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Characters Table
const char lcdChars[]={' ','0','1','2','3','4','5','6','7','8','9','a','b','c','d'\
,'e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x'\
,'y','z','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R'\
,'S','T','U','V','W','X','Y','Z','&',':',',','.','*','/','-','+','=','_','#','@'\
,'!','%','[',']','(',')','~','"','"'};

// 16x2 Display
LiquidCrystal_I2C lcd(lcdAddr);
uint8_t lcdCols = 16; // number of columns in the LCD
uint8_t lcdRows = 2;  // number of rows in the LCD
#define lcdBacklightPin 9 // display backlight pin
// Custom Characters
uint8_t bar1[8] = {0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10};
uint8_t bar2[8] = {0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18};
uint8_t bar3[8] = {0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C};
uint8_t bar4[8] = {0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E};
uint8_t bar5[8] = {0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F};
#define lcdClearCharSpeed 75 // ms delay between drawing each character (clearing display)
const long lcdCharSpeed = 250; // ms delay between drawing each character (message mode)
unsigned long lcdLastTime = 0;
uint32_t lcdCharLimit = 250; // max scrolling characters (clears display after limit)
uint32_t chrarSize = 0;
uint32_t rowCount0 = 0;
uint32_t rowCount1 = 0;
bool eventLCDMessage = 0;
String charBuffer0;
String charBuffer1;
String lastChars;

// RTOS Multi-Core Handle
TaskHandle_t Task1;
TaskHandle_t Task2;

// Power Control
#define clearButtonPin 5 // on MCP chip
uint8_t clearButton = 0;
uint8_t lastclearButton = 0;
uint32_t clearButtonMillis;
uint8_t initStartDelay = 5; // delay on initial cold start
uint8_t debounceDelay = 50; // button debounce delay in ms

// Enable Serial Messages (0 = off)
#define DEBUG 1

#if DEBUG == 1
#define debugstart(x) Serial.begin(x)
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debugstart(x)
#define debug(x)
#define debugln(x)
#endif

// WiFi 
unsigned long WiFiLastMillis = 0;
unsigned long WiFiDownInterval = 30000;
String ipAddress;

// Web Server
WiFiServer server(CONFIG_PORT);
String httpHeader = "Accept: lcd/";
String LCDmessage;
String httpRequest;
unsigned long HTTPcurTime = millis();
unsigned long HTTPlastTime = 0;        
// HTTP timeout
const long timeoutTime = 1000;

// initialization
void setup() {
  // Web task - parallel a task
  xTaskCreatePinnedToCore(
   WebServer,    /* task function. */
   "Task1",     /* name of task. */
   10000,       /* Stack size of task */
   NULL,        /* parameter of the task */
   1,           /* priority of the task */
   &Task1,      /* Task handle to keep track of created task */
   0);          /* pin task to core 0 */  
  delay(500);  
  // LCD task - parallel a task
  xTaskCreatePinnedToCore(
   LCDDraw,     /* task function. */
   "Task2",     /* name of task. */
   10000,       /* Stack size of task */
   NULL,        /* parameter of the task */
   3,           /* priority of the task */
   &Task2,      /* Task handle to keep track of created task */
   1);          /* pin task to core 1 */  
  delay(500);
}

// parallel task 
void WebServer( void * pvParameters ){
  disableCore0WDT(); // disable on core 0 (firmware bug)
  // built-in LED
  pinMode(LED_BUILTIN, OUTPUT);  
  digitalWrite(LED_BUILTIN, HIGH);    
  // display backlight (low)
  pinMode(lcdBacklightPin, OUTPUT);  
  digitalWrite(lcdBacklightPin, LOW);
  // Start serial
  debugstart(CONFIG_SERIAL);
  debug("Web running on core ");
  debugln(xPortGetCoreID());
  // Start WiFi connection
  debug("Connecting to: ");
  debugln(CONFIG_SSID);
  int _tryCount = 0;
  while (WiFi.status() != WL_CONNECTED) {
    _tryCount++;
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
    WiFi.setHostname(HOSTNAME);
    WiFi.begin(CONFIG_SSID, CONFIG_PSK);
    vTaskDelay( 4000 );
    if ( _tryCount == 10 )
    {
      ESP.restart();
    }
  }
  debugln();
  debugln("WiFi connected!");
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  // Print WiFi connection information
  ipAddress = WiFi.localIP().toString();
  debug("  SSID: ");
  debugln(WiFi.SSID());
  debug("  RSSI: ");
  debug(WiFi.RSSI());
  debugln(" dBm");
  debug("  Local IP: ");
  debugln(ipAddress);
  debug("  Port: ");
  debugln(CONFIG_PORT);  
  // Start webserver
  debugln("Starting webserver...");
  server.begin();
  delay(1000);
  debugln("Webserver started!");
  debugln();
  // setup done
  for(;;){ //
  ///////////////////
    // if WiFi is down, try reconnecting
    unsigned long WiFiCurMillis = millis();
    if ((WiFi.status() != WL_CONNECTED) && (WiFiCurMillis - WiFiLastMillis >= WiFiDownInterval)) {
      debug(millis());
      debugln("Reconnecting to WiFi...");
      WiFi.disconnect();
      WiFi.reconnect();
      WiFiLastMillis = WiFiCurMillis;
    }  
    // light web server
    webServer();
  }
}

// parallel task 
void LCDDraw( void * pvParameters ){
  //disableCore1WDT();
  debug("LCD drawing running on core ");
  debugln(xPortGetCoreID());
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
  lcdTimedBar(initStartDelay,1);
  // WiFi status
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("WiFi Connected.");
  lcd.setCursor(0,1);
  lcd.print("IP: " + ipAddress);  
  delay(1500);
  // show splash screen
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Z-Terminal");    
  lcd.setCursor(0,1);
  lcd.print("v1.1");  
  delay(1500);
  lcd.clear();
  lcd.setCursor(0,0);
  // setup done
  for(;;){ // LCD loop
  ///////////////////
    decodeMessage();
    readClearButton();
  }
}

void decodeMessage() {
  // LCD message async trigger
  if (eventLCDMessage == 1) {
    // remove HTTP header & new line characters
    LCDmessage.remove(0, ((LCDmessage.lastIndexOf(httpHeader)) + 12));
    LCDmessage.trim();
    debug("Trimmed HTTP Data: ");
    debugln(LCDmessage);
    // calculate trimmed message length
    int msgLength = LCDmessage.length() + 1;
    // find delimiter positions 
    const char _delimiter = '/';
    int _delsecond = 0;
    int _delfirst = 0;
    int _delcount = 0; // loop character by character
    for(int _msgindex=0; _msgindex < msgLength; _msgindex++ ) {
      char _msgchar = LCDmessage.charAt(_msgindex);
      if (_msgchar == _delimiter){ 
        // store position of first
        if (_delcount == 0){
          _delfirst = _msgindex;
        }
        // store position of second
        if (_delcount == 1){
          _delsecond = _msgindex;
        }
        // stop counting after finding both
        if (_delcount > 1){
          break;
        } // count matches 
        _delcount++;
      }  
    } // Extract line command data
    String _linecmd = LCDmessage.substring(0, _delfirst); 
    int _lineint = _linecmd.toInt();
     // Extract character delay speed data
    String _delaycmd = LCDmessage.substring(_delfirst + 1, _delsecond); 
    int _delayint = _delaycmd.toInt();
    debug("Line Command: ");
    debugln(_lineint);
    debug("Delay Speed: ");
    debugln(_delayint);
    if (_delcount != 2){ // only allow HTTP request with both delimiters
      LCDmessage = "Invalid Data!";
      msgLength = LCDmessage.length() + 1;
    } else {
      // remove control characters
      LCDmessage.remove(0, ((LCDmessage.lastIndexOf(_delimiter)) + 1));
    }
    debug("Parsed HTTP Data: ");
    debugln(LCDmessage);
    // copy to character array
    char char_array[msgLength];
    LCDmessage.toCharArray(char_array, msgLength);
    // send to display
    sendMessage(char_array,_lineint,_delayint);
    // clear buffer
    LCDmessage = "";
    // exit function
    eventLCDMessage = 0;
  }
}

// convert message into character stream
void sendMessage(char _msg[], int _line, int _delay) {
  uint32_t _char; // set default speed if not in range
  if (_delay == 0 || _delay > 4096) { 
    _delay = lcdCharSpeed;
  } // set default line if not in range
  if (_line > 3) { 
    _line = 0;
  } // loop through each character
  for(int i=0; i < strlen(_msg); i++ ) {
    // convert each character into array index positions
    _char = (charLookup(_msg[i]));
    if (_line == 3) {
      clearLCD(1);
      return;
    } // clear display 
    if (_line == 2) {
      clearLCD(0);
      return;
    } // draw each character
    if (_line <= 1) {
      drawChar(_line,_char);
      debugln(_char);
    } // delay between drawing each character
    for(;;) { 
      unsigned long lcdCurTime = millis();
      readClearButton(); // keep reading button state
      if (lcdCurTime - lcdLastTime >= _delay) {
        lcdLastTime = lcdCurTime;
        break; // exit loop when time exceeded 
      }
    }
    if (eventLCDMessage == 0) {
      return; // exit if message was canceled
    }  
  } // draw a space
  drawChar(_line,0);
}
// character search
int charLookup(char _char){
  int _index;
  for (_index=0; _index <= chrarSize; _index++){
    if (lcdChars[_index] == _char){
      break;
    }
  } // return position 
  return _index;
}


// scroll text on display
void drawChar(bool _line, uint32_t _char) {
  if( _char < chrarSize){ // ignore invalid 
    uint32_t _cursor0;
    uint32_t _cursor1;
    // print index position
    debugln(_char);
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
    } 
    // display a single character from the array
    lcd.print(lcdChars[_char]);
    // clear display when character limit exceeded
    if( _line == 0){
      if( rowCount0 > lcdCharLimit){
        lastChars = "";
        rowCount0 = 0;
        charBuffer0 = "";
        for(uint8_t _count = 0; _count < lcdCols + 1; _count++)
        { // draw spaces
          lcd.setCursor(_count,0);
          lcd.print(" ");
          delay(lcdClearCharSpeed);
        }
        lcd.setCursor(0,0);
      }   
    } else {
      if( rowCount1 > lcdCharLimit){
        lastChars = "";
        rowCount1 = 0;
        charBuffer1 = "";
        for(uint8_t _count = 0; _count < lcdCols + 1; _count++)
        { // draw spaces
          lcd.setCursor(_count,1);
          lcd.print(" ");
          delay(lcdClearCharSpeed);
        }  
        lcd.setCursor(0,1);    
      }
    }
  }  
}

// clear display
void clearLCD(uint8_t _line) 
{   // clear both lines
  if (_line > 1) { 
    rowCount0 = 0;
    charBuffer0 = "";
    rowCount1 = 0;
    charBuffer1 = "";
  } // clear top row
  if (_line == 1) { 
    rowCount1 = 0;
    charBuffer1 = "";
  } // clear bottom row  
  if (_line == 0) { 
    rowCount0 = 0;
    charBuffer0 = "";
  } // ignore invalid range
  if (_line < 3) { // loop through all display characters
    for(uint8_t _count = 0; _count < lcdCols + 1; _count++) { 
      // draw spaces
      if (_line > 1) {
        lcd.setCursor(_count, 0);
        lcd.print(" ");
        lcd.setCursor(_count, 1);
        lcd.print(" ");
      } else {
        lcd.setCursor(_count, _line);
        lcd.print(" ");
      }  
      delay(lcdClearCharSpeed);
    }
  }
  if (_line > 1) {
    _line = 0;
  }  
  // reset state
  eventLCDMessage = 0;
  lcd.setCursor(0, _line);
  LCDmessage = "";
  lastChars = "";
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
    delay(25);
  }    
}  

// clear display button
void readClearButton() {
  // read pin state from MCP23008 //////////////////
  int reading = lcd.readPin(clearButtonPin);
  // if switch changed
  if (reading != lastclearButton) {
    // reset the debouncing timer
    clearButtonMillis = millis();
  }
  if ((millis() - clearButtonMillis) > debounceDelay) {
    // if button state has changed
    if (reading != clearButton) {
      clearButton = reading;
      if (clearButton == 1) { 
        // button change event
        clearLCD(2);
      }
    } 
  }
  lastclearButton = reading; 
}


// light HTTP server
void webServer() 
{   
  // Wait for new client
  WiFiClient client = server.available();
  if (client) {
    HTTPcurTime = millis();
    HTTPlastTime = HTTPcurTime;
    debugln("New Client.");                 // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && HTTPcurTime - HTTPlastTime <= timeoutTime) {
      HTTPcurTime = millis();
      // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        httpRequest += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            // transmit example: (curl http://lcd16x2.home/message -H "Accept: lcd/0/message")
            debugln(httpRequest); // print full HTTP request
            // only allow message prefix
            if (httpRequest.indexOf(F("/message")) != -1) {
              // only allow correct HTTP header 
              if(httpRequest.indexOf(httpHeader) >=0) {
                // copy to shared buffer
                LCDmessage = httpRequest;
                // trigger LCD action
                eventLCDMessage = 1;
                // response to client
                client.println("command received.");
              }
            }            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the httpRequestuest variable
    httpRequest = "";
    // Close the connection
    client.stop();
    debugln("Client disconnected.");
    debugln("");
  }
}  


void loop() {
  delay(1000);
}
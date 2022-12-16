 //////////////////////////////////////////////////////////////////////////
// by Ben Provenzano III
//////////////////////////////////////////////////////////////////////////

// Libraries //
#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include "LiquidCrystal_I2C.h" // custom for MCP23008-E/P, power button support
#include <neotimer.h>

//////////////////////////////////////////////////////////////////////////
// Wi-Fi Configuration
const char* CONFIG_SSID      = "mach_kernel";
const char* CONFIG_PSK       = "phonics.87.reply.218";
const char* HOSTNAME         = "lcd16x2";
const int   CONFIG_SERIAL    = 115200;
const int   CONFIG_PORT      = 80;
//////////////////////////////////////////////////////////////////////////

// LCD Characters Table
const char lcdChars[]={' ','0','1','2','3','4','5','6','7','8','9','a','b','c','d'\
,'e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x'\
,'y','z','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R'\
,'S','T','U','V','W','X','Y','Z','&',':',',','.','*','|','-','+','=','_','#','@'\
,'{','%','[',']','(',')','~','"','<','>','?','}'};

// RTOS Multi-Core Handle
TaskHandle_t Task1;
TaskHandle_t Task2;

// 16x2 LCD Display
#define lcdAddr 0x27 // I2C address
LiquidCrystal_I2C lcd(lcdAddr);
uint8_t lcdCols = 16; // number of columns in the LCD
uint8_t lcdRows = 2;  // number of rows in the LCD
#define lcdBacklight 13 // display backlight pin
// Custom Characters (progress bar)
uint8_t bar1[8] = {0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10};
uint8_t bar2[8] = {0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18};
uint8_t bar3[8] = {0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C};
uint8_t bar4[8] = {0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E};
uint8_t bar5[8] = {0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F};
const uint32_t lcdClearCharSpeed = 50; // ms delay between drawing each character (clearing display)
const uint32_t lcdCharSpeed = 250; // ms delay between drawing each character (message mode)
const uint32_t lcdCharLimit = 500; // max scrolling characters (clears display after limit)
Neotimer lcdDim = Neotimer(80000); // 80 second timer
Neotimer lcdDelayTimer = Neotimer(500); 
unsigned long lcdDimLastTime = 0;
bool lcdBacklightDim = 1; 
uint32_t chrarSize = 0;
uint32_t rowCount0 = 0;
uint32_t rowCount1 = 0;
String charBuffer0 = "";
String charBuffer1 = "";
unsigned long lcdLastTime = 0;
String lastChars = "";
uint32_t lcdDelay = 0;
uint8_t lcdLine = 0;
// Shared resources
String lcdMessage = "";
uint8_t clearDisplay = 0;
bool eventlcdMessage = 0;

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
uint8_t startDelay = 5; // delay on initial start in seconds

// Wi-Fi & Web Server
String ipAddress = "";
unsigned long WiFiDownInterval = 30000; // WiFi reconnect timeout (ms)
unsigned long WiFiLastMillis = 0;
WiFiServer server(CONFIG_PORT);
unsigned long HTTPlastTime = 0; 
unsigned long HTTPcurTime = millis(); 
const long timeoutTime = 1000; // HTTP timeout
String httpRequest = ""; 
String httpHeader = "Accept: lcd/";

//////////////////////////////////////////////////////////////////////////
// Enable Serial Messages (0 = off)
#define DEBUG 1
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

//////////////////////////////////////////////////////////////////////////
// parallel task 0
void WebServer( void * pvParameters ){
  disableCore0WDT(); // disable on core 0 (firmware bug)
  // built-in LED
  pinMode(LED_BUILTIN, OUTPUT);  
  digitalWrite(LED_BUILTIN, HIGH);    
  // display backlight (low)
  pinMode(lcdBacklight, OUTPUT);  
  digitalWrite(lcdBacklight, LOW);
  // set button
  pinMode(setButtonPin, INPUT_PULLUP);
  // start serial
  debugstart(CONFIG_SERIAL);
  debug("Web running on core ");
  debugln(xPortGetCoreID());
  // start WiFi connection
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
  debugln("Webserver started.");
  debugln();
  // setup done
  for(;;){ //
  ///////////////////
    // if WiFi is down, try reconnecting
    unsigned long WiFiCurMillis = millis();
    if ((WiFi.status() != WL_CONNECTED) && \
     (WiFiCurMillis - WiFiLastMillis >= WiFiDownInterval)) {
      debug(millis());
      debugln("Reconnecting to WiFi...");
      WiFi.disconnect();
      WiFi.reconnect();
      WiFiLastMillis = WiFiCurMillis;
    }  
    // light web server
    webServer();
    // read set button
    readSetButton();
  }
}

// light HTTP server
void webServer() 
{ // Wait for new client
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
                decodeMessage(httpRequest);
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

// parse and decode incoming HTTP request
void decodeMessage(String _msg) {
  // remove HTTP header & new line characters
  _msg.remove(0, ((_msg.lastIndexOf(httpHeader)) + 12));
  _msg.trim();
  debug("Trimmed HTTP Data: ");
  debugln(_msg);
  // find delimiter positions 
  const char _delimiter = '/';
  int _delsecond = 0;
  int _delfirst = 0;
  int _delcount = 0; // loop character by character
  int _msgLength = _msg.length() + 1;
  for(int _msgindex=0; _msgindex < _msgLength; _msgindex++ ) {
    char _msgchar = _msg.charAt(_msgindex);
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
  } 
  // extract line command data
  String _linecmd = _msg.substring(0, _delfirst); 
  // write integer to shared buffer
  lcdLine = _linecmd.toInt();
  // clear display routines
  if (lcdLine < 5 && lcdLine > 1) {
    clearDisplay = lcdLine;
    eventlcdMessage = 0;
    _msg = "";
    return;
  }
  // clear display if message sent when event still running
  if(eventlcdMessage == 1){
    _msg = "";
    return;
  }  
  // extract character delay speed data
  String _delaycmd = _msg.substring(_delfirst + 1, _delsecond); 
  // write integer to shared buffer
  lcdDelay = _delaycmd.toInt();
  if (_delcount != 2){ // only allow HTTP request with both delimiters
    _msg = "Invalid Data!";
    _msgLength = _msg.length() + 1;
  } else {
    // remove control characters
    _msg.remove(0, ((_msg.lastIndexOf(_delimiter)) + 1));
  }
  // write message to shared buffer
  lcdMessage = _msg;
  // trigger display event 
  eventlcdMessage = 1;
  // clear buffer
  _msg = "";
}

//////////////////////////////////////////////////////////////////////////
// parallel task 1
void LCDDraw( void * pvParameters ){
  debug("LCD drawing running on core ");
  debugln(xPortGetCoreID());
  // enable full brightness
  digitalWrite(lcdBacklight, HIGH);
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
  // WiFi status
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Z-Terminal");   
  lcd.setCursor(0,1);
  lcd.print("IP: " + ipAddress);  
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  // start dimming timer
  lcdDim.reset();
  lcdDim.start();
  // setup done
  for(;;){ // LCD loop
  ///////////////////  
    // display message event
    if (eventlcdMessage == 1) {  
      digitalWrite(lcdBacklight, HIGH);
      lcdDim.reset();
      lcdMessageEvent();
      lcdDim.start();
      eventlcdMessage = 0; 
    }
    // clear display events
    clearEvents();      
  }
}

// runs in main loop and during character delay
void clearEvents() {
  // read GPIO button
  readClearButton();
  // clear display event 
  if (clearDisplay > 0 && clearDisplay < 5) {
    // correct range
    uint8_t _clearMode = clearDisplay - 2; 
    digitalWrite(lcdBacklight, HIGH);   
    lcdDim.reset();
    clearLCD(_clearMode);
    lcdDim.start();
    clearDisplay = 0; 
  } 
  // display dim event
  if(lcdDim.done()){
    digitalWrite(lcdBacklight, LOW);
    lcdDim.reset();
  }
}

// convert message into character stream
void lcdMessageEvent() { // (run only from event timer)
  int _line = lcdLine; // read line data from shared buffer
  if (_line > 1) { // ignore invalid range
    return;
  } // read message data from shared buffer
  String _http = lcdMessage; 
  debug("Parsed HTTP Data: ");
  debugln(_http);
  debug("Line Command: ");
  debugln(_line);
  debug("Delay Speed: ");
  debugln(lcdDelay);
  // convert string to character array
  int _msgLength = _http.length() + 1;
  char _msg[_msgLength];
  _http.toCharArray(_msg, _msgLength);
  _http = "";
  uint32_t _char; // loop through each character
  for(int i=0; i < strlen(_msg); i++ ) {
    // convert each character into array index positions
    _char = (charLookup(_msg[i]));
    // draw each character
    if (_line <= 1) {
      // clear display if character limit exceeded
      if( _line == 0){
        if( rowCount0 > lcdCharLimit){
          clearDisplay = 2;
        }   
      } else {
        if( rowCount1 > lcdCharLimit){
          clearDisplay = 3; 
        }
      }
      drawChar(_line,_char);
      debugln(_char);
    }
    // stop drawing if request canceled 
    if (eventlcdMessage == 0) {
      return;
    }
  }
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
void drawChar(bool _line, uint32_t _char) {
  if( _char < chrarSize){ // ignore invalid input
    uint8_t _cursor0;
    uint8_t _cursor1;
    ////////////////////////////////////// line 0
    // compute each row separately
    if( _line == 0){
      // store each character 	
      charBuffer0 += lcdChars[_char];
      if( rowCount0 > lcdCols ){ // range 0-15
        // overflow behavior
        _cursor0 = lcdCols - 1; // trim character string
        lastChars = charBuffer0.substring(rowCount0 - _cursor0, rowCount0);
        lcd.setCursor(0, _line);
        lcd.print(lastChars);
      } else {
        // before overflow behavior
        _cursor0 = rowCount0;
      }
      rowCount0++; // store row position (must be done here)
      ////////////////////////////////////////////////////
      // draw new character > 16 collumns
      if( rowCount0 >= lcdCols ){ // range 1-16
        if(rowCount0 == lcdCols + 2){ // overflow transition delay 
          charDelay();
        } 
        lcd.setCursor(_cursor0, _line);
        lcd.print(lcdChars[_char]);
        if(rowCount0 != lcdCols + 2){ // overflow transition delay
          charDelay(); 
        } 
      // draw new character < 16 collumns
      } else { // delay then draw, for collumn < max display width
        if(rowCount0 != 1){ // stops delay on first character drawing    
          charDelay();
        } 
        if(rowCount0 != 0){ // stops character drawing after clearing display
          lcd.setCursor(_cursor0, _line);
          lcd.print(lcdChars[_char]);
        }
        if(rowCount0 == lcdCols - 1){ // overflow transition delay 
          charDelay(); 
        }
      }
    } else { ///////////////////////////// line 1
      // store each character 	
      charBuffer1 += lcdChars[_char];
      if( rowCount1 > lcdCols ){
        // overflow behavior
        _cursor1 = lcdCols - 1; // trim character string
        lastChars = charBuffer1.substring(rowCount1 - _cursor1, rowCount1);
        lcd.setCursor(0, _line);
        lcd.print(lastChars); // print trailing characters
      } else {
        // before overflow behavior
        _cursor1 = rowCount1;
      } 
      rowCount1++; // store row position (must be done here)
      ////////////////////////////////////////////////////
      // draw new character > 16 collumns
      if( rowCount1 >= lcdCols ){ // range 1-16
        if(rowCount1 == lcdCols + 2){ // overflow transition delay 
          charDelay();
        } 
        lcd.setCursor(_cursor1, _line);
        lcd.print(lcdChars[_char]);
        if(rowCount1 != lcdCols + 2){ // overflow transition delay
          charDelay(); 
        } 
      // draw new character < 16 collumns
      } else { // delay then draw, for collumn < max display width
        if(rowCount1 != 1){ // stops delay on first character drawing    
          charDelay();
        } 
        if(rowCount1 != 0){ // stops character drawing after clearing display
          lcd.setCursor(_cursor1, _line);
          lcd.print(lcdChars[_char]);
        }
        if(rowCount1 == lcdCols - 1){ // overflow transition delay 
          charDelay(); 
        }
      }
    } ////////////////////////////////////////////////////
  }
}

// character delay 
void charDelay() {
  int _delay = lcdDelay;   
  // set default speed if not in range
  if (_delay < 4096) { 
  	if (_delay > 0 ) {
      // set default line if not in range
      lcdDelayTimer.reset();
      lcdDelayTimer.set(_delay);
      lcdDelayTimer.start();
      for(;;) { // adjustable delay
        if(lcdDelayTimer.done()){
          lcdDelayTimer.reset();
          break;
        } // keep checking for events during delay
        clearEvents(); 
      }
    }  
  }
}

// clear display (run only from event timer)
void clearLCD(uint8_t _line) { 
  // ignore invalid range
  if (_line < 3) { 
    // loop through all display characters
    for(uint8_t _count = 0; _count < lcdCols; _count++) { 
      // draw spaces
      if (_line > 1) {
        // clear both lines 
        lcd.setCursor(_count, 0);
        lcd.print(' ');
        lcd.setCursor(_count, 1);
      } else {
        // clear a single line
        lcd.setCursor(_count, _line);
      }  
      lcd.print(' ');
      delay(lcdClearCharSpeed);
    }
    lcd.setCursor(0, _line);
  }
  // clear both lines
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
  }
  lastChars = "";  
}

// display progress bar for # of seconds 
void lcdTimedBar(int _sec) { 
  uint8_t _rowcount;
  uint32_t _colcount;
  uint32_t _segcount;
  uint8_t _line = 0;
  uint32_t _ms = _sec * 6; // constant (based on CPU speed)
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
      if (clearButton == 1) { 
        // button change event
        eventlcdMessage = 0;
        clearDisplay = 4; // both lines
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
      if (setButton == 1) { 
        // button change event
        if (eventlcdMessage == 0) { 
          // Trigger display
          lcdLine = 0;
          lcdDelay = 0;
          lcdMessage = "PLACEHOLDER....";
          eventlcdMessage = 1;
        }   
      }
    } 
  }
  setButtonLast = reading; 
}

void loop() {
  // main loop disabled
  delay(1000);
}
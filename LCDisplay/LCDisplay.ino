 //////////////////////////////////////////////////////////////////////////
// by Ben Provenzano III
//////////////////////////////////////////////////////////////////////////

// Libraries //
#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include "time.h"
#include <neotimer.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include "LiquidCrystal_I2C.h" // custom for MCP23008-E/P, power button support

//////////////////////////////////////////////////////////////////////////
// Wi-Fi Configuration
const char* CONFIG_SSID   = "mach_kernel";
const char* CONFIG_PSK    = "phonics.87.reply.218";
const char* HOSTNAME      = "lcd16x2";
const int   CONFIG_SERIAL = 115200;
const int   CONFIG_PORT   = 80;
//////////////////////////////////////////////////////////////////////////

// LCD Valid Characters
const char lcdChars[]={" 0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ&:',.*|-+=_#@%[]()<>?{};"};

// Fuzzy Clock Tables
static const char *clock_day_sectors[] =
{
  ("late night"),
  ("early morning"),
  ("morning"),
  ("mid morning"),
  ("noon"),
  ("afternoon"),
  ("evening"),
  ("late evening")
};

static char *clock_hour_sectors[] =
{
  /* %0 will be replaced with the, 
   * current hour %1 with the comming hour */
  "%0 o'clock ",
  "five past %0 ",
  "ten past %0 ",
  "quarter past %0 ",
  "twenty past %0 ",
  "twenty five past %0 ",
  "half past %0 ",
  "twenty five to %1 ",
  "twenty to %1 ",
  "quarter to %1 ",
  "ten to %1 ",
  "five to %1 ",
  "%1 o'clock "
};

static char *clock_hour_names[] =
{
  "one",
  "two",
  "three",
  "four",
  "five",
  "six",
  "seven",
  "eight",
  "nine",
  "ten",
  "eleven",
  "twelve"
};

// Ten-minutes in ms
#define tenMin 600000

// RTOS Multi-Core Support
TaskHandle_t Task1;
TaskHandle_t Task2;

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
uint8_t startDelay = 4; // delay on initial start in seconds

// 16x2 LCD Display
#define lcdAddr 0x27 // I2C address
LiquidCrystal_I2C lcd(lcdAddr);
const uint8_t lcdCols = 16; // number of columns in the LCD
const uint8_t lcdRows = 2;  // number of rows in the LCD
#define lcdBacklight 13 // display backlight pin
// Custom Characters (progress bar)
uint8_t bar1[8] = {0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10};
uint8_t bar2[8] = {0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18};
uint8_t bar3[8] = {0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C};
uint8_t bar4[8] = {0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E};
uint8_t bar5[8] = {0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F};
Neotimer lcdDelayTimer = Neotimer();
Neotimer lcdDimmer = Neotimer(100000); // ms before dimming display backlight
const uint8_t lcdClearCharSpeed = 50; // ms delay between drawing each character (clearing display)
const uint32_t lcdDefaultDelay = 275; // ms delay used when none is specified
uint8_t charBuffer0[20];
uint8_t charBuffer1[20];
uint8_t chrarSize = 0;
uint8_t rowCount0 = 0;
uint8_t rowCount1 = 0;

// Shared resources
#define httpBufferSize 16384 // HTTP request buffer (bytes)
char lcdMessage[httpBufferSize] = {'\0'};
char httpReq[httpBufferSize] = {'\0'};
uint32_t lcdMessageEnd = 0;
bool eventlcdMessage = 0;
uint32_t lcdDelay = 0;
uint8_t lcdReset = 0;  

// Web Server
WiFiServer server(CONFIG_PORT);
char httpHeader[] = {"####?|"}; // API signature
unsigned long HTTPlastTime = 0; 
unsigned long HTTPcurTime = millis(); 
unsigned long httpLineCount = 0;
unsigned long httpReqCount = 0;
const long timeoutTime = 1500; // HTTP timeout (ms)

// Wi-Fi
String ipAddress = "";
unsigned long WiFiDownInterval = tenMin; // WiFi reconnect timeout (ms)
unsigned long WiFiLastMillis = 0;

// NTP
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -18000;
const int daylightOffset_sec = 3600;
Neotimer clockTimer = Neotimer(tenMin); // 10 minute timer
bool eventPrintTime = 0;

// Weather Search
bool weatherNew = 1; // download new data flag
bool weatherTrigger = 0; // event trigger flag
Neotimer owmTimer = Neotimer(tenMin); // 10 minute timer
String weatherData; 

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
   16384,       /* Stack size of task */
   NULL,        /* parameter of the task */
   1,           /* priority of the task */
   &Task1,      /* Task handle to keep track of created task */
   0);          /* pin task to core 0 */  
  delay(500);  
  // LCD task - parallel a task
  xTaskCreatePinnedToCore(
   LCDDraw,     /* task function. */
   "Task2",     /* name of task. */
   16384,       /* Stack size of task */
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
  // Network time protocol
  debugln("Starting NTP...");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  delay(500);
  debugln("NTP started.");
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

// HTTP request server
void webServer() 
{ // wait for new client
  WiFiClient client = server.available();
  if (client) {
    HTTPcurTime = millis();
    HTTPlastTime = HTTPcurTime;
    debugln("New Client.");                 // print a message out in the serial port              
    char curLine[httpBufferSize] = {'\0'};  // make an array to hold incoming data from the client
    while (client.connected() && HTTPcurTime - HTTPlastTime <= timeoutTime) {
      HTTPcurTime = millis();
      // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        // add each character to array 
        if (httpReqCount < httpBufferSize && httpReqCount >= 0){
          httpReq[httpReqCount] = c;
          httpReqCount++;
        } else {
          httpReqCount = 0;   
        } // if the byte is a newline character
        if (c == '\n') { 
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, send a response:
          if (httpLineCount == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            // transmit example: (curl http://lcd16x2.home/message -H "Accept: ####?|0|0|message")
            debugln("HTTP request");
            for(uint32_t _idx = 0; _idx < httpReqCount; _idx++) {
              char _vchr = httpReq[_idx];    
              debug(_vchr); 
            }
            debugln("---------");
            // loop through characters (detect header signature)
            uint32_t _matches = 0;
            uint32_t _charstart = 0;            
            uint8_t _hdrcount = sizeof(httpHeader) - 2;
            for(uint32_t _idx = 0; _idx < httpReqCount; _idx++) {
              // find matching characters
              if (httpReq[_idx] == httpHeader[_matches]) {
                if (_matches >= _hdrcount){
                  // all characters matched
                  client.println("command received.");
                  // pass array start-end positions to message function
                  decodeMessage(_idx + 1, httpReqCount);
                  break;
                } // count matches 
                _matches++;
              }  
            }      
            // the HTTP response ends with another blank line
            client.println();
            break; // break out of the while loop
          } else { // if you got a newline, then clear currentLine
            httpLineCount = 0;  
          }
        } else if (c != '\r') {  // if you have anything else but a carriage return character
          // add it to the end of the current line
          if (httpLineCount < httpBufferSize && httpLineCount >= 0){
            curLine[httpLineCount] = c;
            httpLineCount++;
          } else {
            httpLineCount = 0;   
          }
        }
      }
    }
    // reset the HTTP request array index counter
    httpReqCount = 0;
    // close the connection
    client.stop();
    debugln("client disconnected.");
    debugln("");
  }
}

// decode LCD message and trigger display event
void decodeMessage(uint32_t _startpos, uint32_t _httpcount) { 
  //////////// start and end positions of control characters & message
  uint8_t _maxchars = 8; // max characters for line & delay commands
  uint32_t _linepos = 0;
  char _delimiter = '|';  
  // find second delimiter position
  for(uint32_t _idx = _startpos; _idx < _httpcount; _idx++) {  
    char _vchr = httpReq[_idx];  
    if (_vchr == _delimiter) {
      // store index position
      _linepos = _idx;
      break;
    }
  }
  char _linebuffer[_maxchars];
  uint8_t _linecount = 0;  
  // loop through line characters  
  for(uint32_t _idx = _startpos; _idx < _linepos; _idx++) {
    if (_linecount >= _maxchars) {
      break;
    } // store in new array
    _linebuffer[_linecount] = httpReq[_idx];
    _linecount++;
  }
  // convert to integer, store line value
  uint32_t _cmd1 = atoi(_linebuffer); 
  // find third delimiter position
  uint32_t _count = 0;
  uint32_t _cmd2pos = 0; 
  for(uint32_t _idx = _startpos; _idx < _httpcount; _idx++) {
    char _vchr = httpReq[_idx];   
    if (_vchr == _delimiter) {
      if (_count == 1) {
        // store index position
        _cmd2pos = _idx;
        break;
      }  
      _count++;
    }
  } 
  char _cmd2buffer[_maxchars];
  uint8_t _cmd2count = 0;
  // loop through second command characters
  for(uint32_t _idx = _linepos + 1; _idx < _cmd2pos; _idx++) { 
    if (_cmd2count >= _maxchars) {  
      break;
    } // store in new array 
    _cmd2buffer[_cmd2count] = httpReq[_idx];
    _cmd2count++;
  }
  // convert to integer, store second command value
  uint32_t _cmd2 = atoi(_cmd2buffer); 
  // display the weather
  if (_cmd1 == 6) {
    weatherTrigger = 1;
    return;
  }    
  // display the time
  if (_cmd1 == 5) {
    eventPrintTime = 1;
    return;
  }
  // exit when beyond range
  if (_cmd1 > 4) { 
    return;
  }  
  // clear display trigger (2-4 range)
  if (_cmd1 > 1) {
    // write clear trigger
    lcdReset = _cmd1 - 1; 
    return;
  }
  // write message to shared buffer  
  debugln(" ");
  if (eventlcdMessage == 0){ // only if not drawing
    // store shared message data 
    lcdMessageEnd = (_httpcount - (_cmd2pos + 1)); // position of the end of message
    lcdDelay = _cmd2; // delay between drawing characters in (ms)
    debugln(" ");
    debugln("trimmed request: ");
    uint32_t _lcdidx = 0;
    for(uint32_t _idx = _cmd2pos + 1; _idx < _httpcount; _idx++) { 
      lcdMessage[_lcdidx] = httpReq[_idx]; // write to message array
      _lcdidx++; // increment index
      debug(httpReq[_idx]); // print entire message
    }
    // trigger event
    eventlcdMessage = 1;
  } else {
    debugln("display busy. ");
  } 
}

//////////////////////////////////////////////////////////////////////////
// parallel task 1
void LCDDraw( void * pvParameters ){
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
  lcdTimedBar(startDelay);
  // WiFi status
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Z-Terminal");   
  lcd.setCursor(0,1);
  lcd.print("IP: " + ipAddress);  
  delay(1500);
  lcd.clear();
  lcd.setCursor(0,0);
  // start clock timer
  clockTimer.reset(); 
  clockTimer.start();
  // setup done
  for(;;){ // LCD display main loop
  ///////////////////  
    // display message event (main only!)
    if (eventlcdMessage > 0) {  
      lcdDim();
      lcdMessageEvent();
      eventlcdMessage = 0; 
    } 
    // clear display event (main only!)
    if( lcdReset > 0) {
       drawChar(0,0,lcdReset);
    }
    // ran in main and during delay loop 
    mainEvents();      
  }
}

// runs in main loop and during character delay
void mainEvents() {
  // read GPIO button
  readClearButton();
  // weather event
  weatherEvent();
  // display dim event
  if(lcdDimmer.done()){
  	debugln("dimming backlight...");
    digitalWrite(lcdBacklight, LOW);
    lcdDimmer.reset();
  }
  // show the time every 10-minutes
  if(clockTimer.done()){
    eventPrintTime = 1;
    clockTimer.reset();
    clockTimer.start();
  }  
  // display time event
  if (eventPrintTime == 1) {  
    lcdDim();
    printLocalTime();
    eventPrintTime = 0;
  }     
}

// disable LCD dimming then start dimming timer
void lcdDim(){
  lcdDimmer.reset();
  // enable full brightness
  digitalWrite(lcdBacklight, HIGH);
  // start dimming timer
  lcdDimmer.start();
}

// convert message into character stream 
void lcdMessageEvent() { // (run only from event timer)
  uint32_t _reset = 0;
  uint32_t _end = lcdMessageEnd;
  uint32_t _delay = lcdDelay; 
  debug("delay data: ");
  debugln(_delay);    
  debug("end position: ");
  debugln(_end);
  uint32_t _charidx;
  debugln("character stream: ");
  // loop through each character in the request array (message only)
  for(uint32_t _idx = 0; _idx < _end; _idx++) { 
    // convert each character into array index positions
    _charidx = (charLookup(lcdMessage[_idx]));
    // invalid characters
    if( _charidx > chrarSize - 3){ 
      return; // exit 
    }
    _reset = lcdReset; // check reset state
    // draw each character on display
    drawChar(_charidx,_delay,_reset);
    debug(_charidx);
    debug(',');
    // stop drawing if request canceled 
    if (eventlcdMessage == 0) {
      return;
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
void drawChar(uint8_t _char, uint32_t _delayin, uint8_t _reset) {
  bool _line = 1; // 1 = text flows bottom to top, 0 = top to bottom
  // clear LCD routine
  if( _reset > 0){
    lcdDim(); // disable dimming and reset timer
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
    return; // exit 
  } 
  // calculate trailing delay 
  float trldelay = (_delayin / 100.0) * 5; // % 
  uint32_t _lastidx = 0;
  uint32_t _trldelay = round(trldelay);
  uint32_t _delay = _delayin - _trldelay; // correct delay time
  uint8_t _trlcount;
  /////////////////////////////////////////////////////////////////////// line 0
  // count row position   
  rowCount0++; 
  // drawing behavior
  if( rowCount0 > lcdCols ){ 
    // store last trailing character
    _lastidx = charBuffer0[0];
    // overflow behavior
    for(uint8_t _idx = 0; _idx <= lcdCols; _idx++) { 
      charBuffer0[_idx] = charBuffer0[_idx + 1]; // rearrange characters
    } 
    // print last character
    lcd.setCursor(lcdCols - 1, _line);
    lcd.write(lcdChars[_char]); 
    // draw trailing characters
    _trlcount = lcdCols - 2;
    for(uint8_t _idx = 0; _idx <= lcdCols - 2; _idx++) {
      lcd.setCursor(_trlcount, _line);
      lcd.write(lcdChars[charBuffer0[_trlcount]]);
      charDelay(_trldelay); // ms delay between drawing 
      _trlcount--; // decrement index
    }
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
  _char = _lastidx; // trailing character
  _line = !_line; // invert line
  // count row position    
  rowCount1++; 
  // drawing behavior
  if( rowCount1 > lcdCols ){ 
    // overflow behavior
    for(uint8_t _idx = 0; _idx <= lcdCols; _idx++) { 
      charBuffer1[_idx] = charBuffer1[_idx + 1]; // rearrange characters
    }
    // print last character
    lcd.setCursor(lcdCols - 1, _line);
    lcd.write(lcdChars[_char]);       
    // draw trailing characters
    _trlcount = lcdCols - 2;
    for(uint8_t _idx = 0; _idx <= lcdCols - 2; _idx++) {
      lcd.setCursor(_trlcount, _line);
      lcd.write(lcdChars[charBuffer1[_trlcount]]);
      charDelay(_trldelay); // ms delay between drawing 
      _trlcount--; // decrement index
    }
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
  // character delay
  if (_char != 0){ // no delay on spaces
    charDelay(_delay);
  }
}

// character delay 
void charDelay(int _delay) {  
  // set default speed if not in range
  if (_delay < 4096) {
    if (_delay > 10) {
      // prevent dimming while drawing
      lcdDimmer.reset();
      lcdDimmer.start();
      // restart character delay timer	
  	  lcdDelayTimer.reset();
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
        lcdReset = 3; 
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
      if (setButton == 1) {
        // trigger display of weather
        weatherTrigger = 1;
      }
    } 
  }
  setButtonLast = reading; 
}

// display weather event
void weatherEvent() {
  if (weatherTrigger == 1){
    if (weatherNew == 1){
      // build API URL
      String city = "Buffalo"; 
      String countryCode = "US";
      String openWeatherMapApiKey = "842b247f09feaabbf1176be9d6221469"; // API key
      String openWeatherMapURL = "http://api.openweathermap.org/data/2.5/weather?q=";
	    String serverPath = openWeatherMapURL + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey;    
      // call API server
      String jsonBuffer = httpGETRequest(serverPath.c_str()); 
      // decode JSON response data
      JSONVar weatherObject = JSON.parse(jsonBuffer);
      // parse weather data
      if (JSON.typeof(weatherObject) == "undefined") {
      	debugln("Weather data download failed.");
        weatherData = "Parsing input failed!";
      } else {
      	weatherData = "";
      	debugln("Downloading weather data...");
      	// parse temperature and convert K to F 
        String _tempstr0 = JSON.stringify(weatherObject["main"]["temp"]);
        float _tmpnum = KtoF(_tempstr0.toFloat());
        _tempstr0 = String(_tmpnum); // convert back to strings
        _tempstr0.remove(_tempstr0.length()-1,1); // remove last decimal
        String _tempstr1 = JSON.stringify(weatherObject["main"]["feels_like"]);
        _tmpnum = KtoF(_tempstr1.toFloat());
        _tempstr1 = String(_tmpnum); // convert back to strings
        _tempstr1.remove(_tempstr1.length()-1,1); // remove last decimal
        // weather description 
        String _desc = JSON.stringify(weatherObject["weather"][0]["description"]);
        _desc.remove(_desc.indexOf('"'),1); 
        _desc.remove(_desc.lastIndexOf('"'),1); 
        // build message 
        weatherData = "Weather in " + city + " " + _desc + " " + _tempstr0 + "F feels like " + _tempstr1 + "F ";
        _tempstr0 = "";
        _tempstr1 = "";
        _desc = "";
      }
      // clear buffers
      serverPath = "";
      jsonBuffer = "";
      // only allow new data to be pulled every 5-min
      weatherNew = 0;
      owmTimer.reset();
      owmTimer.start();      
    } else { // reset after 5-min, show last data
      debugln("Showing last weather data..."); 	
    }
    if (eventlcdMessage == 0){ // only if not drawing
      // store message data
      lcdDelay = lcdDefaultDelay;
      int _len = weatherData.length() + 1; 
      weatherData.toCharArray(lcdMessage, _len);
      lcdMessageEnd = _len;
      // display message event
      eventlcdMessage = 1;
    } else {
      debugln("display busy. ");
    }
    // end event
  	weatherTrigger = 0;
  } // reset timer   
  if(owmTimer.done()){
  	weatherData = "";
  	owmTimer.reset();
    weatherNew = 1;
  }    
}

// Kelvin to Fahrenheit
float KtoF(float _kel) {
  float _f = (9.0 / 5) * (_kel - 273.15) + 32;	
  return _f;
}

// Send a GET request
String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
  // Your Domain name with URL path or IP address with path
  http.begin(client, serverName);
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  // Build output
  String payload = "{}"; 
  // Check response code
  if (httpResponseCode>0) {
    debug("HTTP Response code: ");
    debugln(httpResponseCode);
    payload = http.getString();
  }
  else {
    debug("Error code: ");
    debugln(httpResponseCode);
  }
  // Free resources
  http.end();
  // 
  return payload;
}

// Time-date using NTP 
void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    debugln("Failed to obtain time");
    return;
  } /* get the month label*/
  char timeMonth[10];
  strftime(timeMonth,10, "%b", &timeinfo);
  debug("timeMonth: ");
  debugln(timeMonth);  
  /* get the day of the month */
  char timeDay[3];
  strftime(timeDay,3, "%d", &timeinfo);
  int x = atoi(timeDay);
  debug("timeDay: ");
  debugln(x);
  /* get the year */
  char timeYear[5];
  strftime(timeYear,5, "%Y", &timeinfo);
  debug("timeYear: ");
  debugln(timeYear);
  /* get the current second */
  char timeSec[3];
  strftime(timeSec,3, "%S", &timeinfo);
  debug("timeSec: ");
  debugln(timeSec);
  /* get the day of the week */
  char timeWeekDay[10];
  strftime(timeWeekDay,10, "%A", &timeinfo);
  debug("timeWeekDay: ");
  debugln(timeWeekDay);
  /* get the hour and minute */
  char timeHour[3];
  strftime(timeHour,3, "%H", &timeinfo);
  int hour = atoi(timeHour);
  debug("timeHour: ");
  debugln(timeHour);
  char timeMin[3];
  strftime(timeMin,3, "%M", &timeinfo);
  int minute = atoi(timeMin);
  debug("timeMin: ");
  debugln(timeMin);
  /* get the time of the day */
  String _tod = (clock_day_sectors[hour / 3]);
  /* get the hour sector */
  int sector = 0;
  if (minute > 6) {
    sector = ((minute - 7) / 15 + 1) * 3;
  }
  /* translated time string */
  String time_format = (clock_hour_sectors[sector]);
  /* detect am/pm */
  bool is_pm = (hour >= 12 && hour != 24);
  /* convert military time to 12-hour */
  if (hour % 12 > 0)
    hour = hour % 12 - 1;
  else
    hour = 12 - hour % 12 - 1;
  /* replace %0 with current hour label */ 
  String curr_hour = (clock_hour_names[hour]);
  time_format.replace("%0",curr_hour);
  /* get the next hour */ 
  int _next;
  if (hour == 11)
     _next = 0;
  else
    _next = hour + 1;
  /* replace %1 with next hour label */ 
  String next_hour = (clock_hour_names[_next]);
  time_format.replace("%1",next_hour);
  time_format = _tod + " " + time_format;
  if (eventlcdMessage == 0){ // only if not drawing  
    // store message data
    lcdDelay = lcdDefaultDelay;
    uint32_t _len = time_format.length() + 1; 
    time_format.toCharArray(lcdMessage, _len);
    lcdMessageEnd = _len;
    // display event
    eventlcdMessage = 1;
  } else {
    debugln("display busy. ");
  }
}

void loop() {
  // main loop disabled
  delay(1000);
}
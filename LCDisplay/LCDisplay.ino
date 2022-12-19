 //////////////////////////////////////////////////////////////////////////
// by Ben Provenzano III
//////////////////////////////////////////////////////////////////////////

// Libraries //
#include <stdio.h>
#include <string.h>
#include <iostream>
using namespace std;
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
const uint8_t lcdCols = 16; // number of columns in the LCD
const uint8_t lcdRows = 2;  // number of rows in the LCD
#define lcdBacklight 13 // display backlight pin
// Custom Characters (progress bar)
uint8_t bar1[8] = {0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10};
uint8_t bar2[8] = {0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18};
uint8_t bar3[8] = {0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C};
uint8_t bar4[8] = {0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E};
uint8_t bar5[8] = {0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F};
const uint32_t lcdClearCharSpeed = 75; // ms delay between drawing each character (clearing display)
const uint32_t lcdCharLimit = 500; // max scrolling characters (clears display after limit)
Neotimer lcdDim = Neotimer(80000); // ms before dimming display backlight
Neotimer lcdDelayTimer = Neotimer();
unsigned long lcdDimLastTime = 0;
unsigned long lcdLastTime = 0;
bool lcdBacklightDim = 1; 
uint32_t chrarSize = 0;
uint32_t rowCount0 = 0;
uint32_t rowCount1 = 0;
uint8_t charBuffer0[100];
uint8_t charBuffer1[100];

// Shared resources
uint8_t lcdReset = 0;   
bool eventlcdMessage = 0;
String lcdMessage = " ";
uint32_t lcdDelay = 0;
uint8_t lcdLine = 0;

// Weather Search
String jsonBuffer;
bool weatherNew = 1; // download new data flag
bool weatherTrigger = 0; // event trigger flag
Neotimer owmTimer = Neotimer(600000); // 10 minute timer
String openWeatherMapApiKey = "842b247f09feaabbf1176be9d6221469"; // API key
String openWeatherMapURL = "http://api.openweathermap.org/data/2.5/weather?q=";
String countryCode = "US";
String city = "Buffalo";
String weatherData; 

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

// NTP
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000;
const int   daylightOffset_sec = 3600;

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
  _linecmd = "";
  // clear display trigger (only 2-4 range)
  if (lcdLine < 5 && lcdLine > 1) {
    lcdReset = lcdLine - 1;
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
  _delaycmd = "";
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
  for(;;){ // LCD display main loop
  ///////////////////  
    // display message event (main only!)
    if (eventlcdMessage > 0) {  
      digitalWrite(lcdBacklight, HIGH);
      lcdDim.reset();
      lcdMessageEvent();
      lcdDim.start();
      eventlcdMessage = 0; 
    }
    // clear display event (main only!)
    if( lcdReset > 0) {
       drawChar(0,0,0);
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
  if(lcdDim.done()){
  	debugln("Dimming backlight...");
    digitalWrite(lcdBacklight, LOW);
    lcdDim.reset();
  }
}

// convert message into character stream
void lcdMessageEvent() { // (run only from event timer)
  int _delay = lcdDelay;
  int _line = lcdLine;
  // read message data from shared buffer
  debug("Parsed HTTP Data: ");
  debugln(lcdMessage);
  debug("Line Command: ");
  debugln(lcdLine);
  debug("Delay Speed: ");
  debugln(lcdDelay);
  // convert string to character array
  int _msgLength = lcdMessage.length() + 1;
  char _msg[_msgLength];
  lcdMessage.toCharArray(_msg, _msgLength);
  lcdMessage = "";
  uint32_t _char; // loop through each character
  for(int i=0; i < strlen(_msg); i++ ) {
    // convert each character into array index positions
    _char = (charLookup(_msg[i]));
    // draw each character
    if (lcdLine <= 1) {
      drawChar(_line,_char,_delay);
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
void drawChar(bool _line, uint32_t _char, uint32_t _delay) {
  // clear LCD routine
  if( lcdReset > 0){
    // full brightness backlight
    lcdDim.reset();
    digitalWrite(lcdBacklight, HIGH);   
    lcdDim.start();
    // loop through all display characters
    for(uint8_t _count = 0; _count < lcdCols; _count++) { 
      // draw spaces
      if (lcdReset > 1) {
        // clear both lines 
        lcd.setCursor(_count, 0);
        lcd.print(' ');
        lcd.setCursor(_count, 1);
      } else {
        // clear a single line
        lcd.setCursor(_count, lcdReset - 1);
      }  
      lcd.print(' ');
      delay(lcdClearCharSpeed);
    } // reset cursor
    lcd.setCursor(0, _line);
    if( lcdReset == 1){ // reset row 0
      rowCount0 = 0;
    }
    if( lcdReset == 2){ // reset row 1
      rowCount1 = 0;
    }
    if( lcdReset == 3){ // reset both rows
      rowCount0 = 0;
      rowCount1 = 0;
    }   
    lcdReset = 0; // end reset event
    eventlcdMessage = 0; // end message event
    return; // exit function 
  }
  // ignore invalid input  
  if( _char < chrarSize){ 
    /////////////////////////////////////////////////////////////////////// line 0
    // compute each row separately
    if( _line == 0){
      // store row position (first)     
      rowCount0++; 
      // drawing behavior
      if( rowCount0 > lcdCols ){ 
        // overflow behavior
        for(int _idx = 0; _idx <= lcdCols; _idx++) { 
          charBuffer0[_idx] = charBuffer0[_idx + 1];
        } // draw trailing characters
        for(int _idx = 0; _idx <= lcdCols; _idx++) {
          lcd.setCursor(_idx, _line); 
          uint8_t _tmpchr0 = charBuffer0[_idx];
          lcd.print(lcdChars[_tmpchr0]);
        }
        // overflow transition delay 
        if(rowCount0 == lcdCols){
          charDelay(_delay);
        }  
        // print new character 15
        lcd.setCursor(lcdCols - 1, _line);
        lcd.print(lcdChars[_char]); 
        // overflow transition delay 
        if(rowCount0 != lcdCols){ 
          charDelay(_delay); 
        }   
        // reset
        rowCount0 = lcdCols;       
      } else { 
        // before overflow behavior
        if(rowCount0 != 0){ // stops character drawing after clearing display
          lcd.setCursor(rowCount0 - 1, _line);
          lcd.print(lcdChars[_char]);
          charDelay(_delay); 
        }
      }
      // store each character (last)
      charBuffer0[rowCount0 - 1] = _char;   
    } else {
      /////////////////////////////////////////////////////////////////////// line 1      
      // store row position (first)     
      rowCount1++; 
      // drawing behavior
      if( rowCount1 > lcdCols ){ 
        // overflow behavior
        for(int _idx = 0; _idx <= lcdCols; _idx++) { 
          charBuffer1[_idx] = charBuffer1[_idx + 1];
        } // draw trailing characters
        for(int _idx = 0; _idx <= lcdCols; _idx++) {
          lcd.setCursor(_idx, _line); 
          uint8_t _tmpchr1 = charBuffer1[_idx];
          lcd.print(lcdChars[_tmpchr1]);
        }
        // overflow transition delay 
        if(rowCount1 == lcdCols){
          charDelay(_delay);
        }  
        // print new character 15
        lcd.setCursor(lcdCols - 1, _line);
        lcd.print(lcdChars[_char]); 
        // overflow transition delay 
        if(rowCount1 != lcdCols){ 
          charDelay(_delay); 
        }   
        // reset
        rowCount1 = lcdCols;       
      } else {
        // before overflow behavior
        if(rowCount1 != 0){ // stops character drawing after clearing display
          lcd.setCursor(rowCount1 - 1, _line);
          lcd.print(lcdChars[_char]);
          charDelay(_delay); 
        }
      }
      // store each character (last)
      charBuffer1[rowCount1 - 1] = _char;         
    }
  }
}

// character delay 
void charDelay(int _delay) {  
  // set default speed if not in range
  if (_delay < 4096) {
    if (_delay < 5) {
      _delay = 5;	// min speed limit
    } // prevent dimming while drawing
    lcdDim.reset();
    lcdDim.start();
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
      if (clearButton == 1) { 
        // button change event
        lcdReset = 3; // both lines
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
          // Trigger display of weather
          weatherTrigger = 1;
        }   
      }
    } 
  }
  setButtonLast = reading; 
}

// display weather event
void weatherEvent() {
  if (weatherTrigger == 1){
    if (weatherNew == 1){
	  String serverPath = openWeatherMapURL + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey;    
      jsonBuffer = httpGETRequest(serverPath.c_str()); // call API server
      JSONVar weatherObject = JSON.parse(jsonBuffer); // receive JSON
      if (JSON.typeof(weatherObject) == "undefined") {
      	debugln("Weather data download failed.");
        weatherData = "Parsing input failed!";
      } else {
      	weatherData = "";
      	debugln("Downloading weather data...");
      	// parse temperature and convert K to F 
        String _tempstr0 = JSON.stringify(weatherObject["main"]["temp"]);
        String _tempstr1 = JSON.stringify(weatherObject["main"]["feels_like"]);
        float _temp0 = KtoF(_tempstr0.toFloat());
        float _temp1 = KtoF(_tempstr1.toFloat());
        _tempstr0 = String(_temp0); // convert back to strings
        _tempstr1 = String(_temp1);
        _tempstr0.remove(_tempstr0.length()-1,1); // remove last decimal
        _tempstr1.remove(_tempstr1.length()-1,1);
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
    // display event
	  lcdLine = 0;
    lcdDelay = 350;
    lcdMessage = weatherData;
    eventlcdMessage = 1;
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
  }

  char timeMonth[10];
  strftime(timeMonth,10, "%b", &timeinfo);
  debug("timeMonth: ");
  debugln(timeMonth);  

  char timeDay[3];
  strftime(timeDay,3, "%d", &timeinfo);
  int x = atoi(timeDay);
  debug("timeDay: ");
  debugln(x);

  char timeYear[5];
  strftime(timeYear,5, "%Y", &timeinfo);
  debug("timeYear: ");
  debugln(timeYear);

  char timeSec[3];
  strftime(timeSec,3, "%S", &timeinfo);
  debug("timeSec: ");
  debugln(timeSec);

  char timeMin[3];
  strftime(timeMin,3, "%M", &timeinfo);
  debug("timeMin: ");
  debugln(timeMin);

  char timeHour12[3];
  strftime(timeHour12,3, "%I", &timeinfo);
  debug("timeHour12: ");
  debugln(timeHour12);  

  char timeHour[3];
  strftime(timeHour,3, "%H", &timeinfo);
  debug("timeHour: ");
  debugln(timeHour);

  char timeWeekDay[10];
  strftime(timeWeekDay,10, "%A", &timeinfo);
  debug("timeWeekDay: ");
  debugln(timeWeekDay);

  debugln();
}

void loop() {
  // main loop disabled
  delay(1000);
}
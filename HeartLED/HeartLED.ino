#include <BlockNot.h>   

// LED driver globals
// 2-11 = LED drive pins
// 12 = LED bit-shift pin
#define outPinLow 2
#define outPinHigh 12
#define ledsCount 19 
#define drawRate 75 // ms
#define refreshRate 50 // us
uint8_t blankRate = refreshRate / 2;
BlockNot drawTimer(refreshRate, MICROSECONDS); 
BlockNot blankingTimer(blankRate, MICROSECONDS); 
uint8_t _upperHalf = (ledsCount / 2) + 1;
uint8_t ledTranslate[] = {13,3,15,5,8,18,11,1,6,16,14,4,17,7,12,2,0,10,19,9};
bool ledData[ledsCount + 1] = {0};
// LED effect globals
BlockNot effectTimer(drawRate, MILLISECONDS);
uint8_t ledIncrement = 0;
bool ledCountState = 0;

void setup() {
  //Serial.begin(115200);
  // initialize LED output pins 
  for(uint8_t _count = outPinLow; _count <= outPinHigh; _count++) {
    pinMode(_count, OUTPUT);
    digitalWrite(_count, LOW);
  }
  // initalize LED data array
  clearLEDdata();
}

void clearLEDdata() {
  uint8_t _count;
  for(_count = 0; _count <= ledsCount; _count++) {
    ledData[_count] = 0; 
  }
}

void showLEDdata() {
 for(uint8_t _count = 0; _count <= ledsCount; _count++) {
   Serial.print(ledData[_count]);
   Serial.print(",");
 }
 Serial.print(ledIncrement);
 Serial.println(" ");
}

void writeLEDs() {
  // translated LEDs state data
  bool _ledStates[ledsCount + 1] = {0};
  // re-draw all LEDs
  if (drawTimer.TRIGGERED_ON_DURATION) {
    // draw each LED one-at-a-time
  	for(uint8_t _curLED = 0; _curLED <= ledsCount; _curLED++) {
  	  // translate data to physical LED layout
  	  uint8_t _led = ledTranslate[_curLED];
  	  _ledStates[_led] = ledData[_curLED];
  	  // toggle shift register
      uint8_t _statesIndex = 0;
  	  if (_led >= _upperHalf){
        // enable LEDs 10-19
  	    digitalWrite(outPinHigh, HIGH);
  	    _statesIndex = _upperHalf;
      } else {
        // enable LEDs 0-9
        digitalWrite(outPinHigh, LOW);
  	    _statesIndex = 0;
  	  }
  	  // translate LED layout to actual I/O pin
      bool _state = 0;
      uint8_t _ledPin;
  	  for(_ledPin = outPinLow; _ledPin <= (outPinHigh - 1); _ledPin++) {
        // search array for selected LEDs on/off state
  	    _state = _ledStates[_statesIndex];
  	    if (_led == _statesIndex) {
          break;
        }
  	    _statesIndex++;
  	  }
      // turn-on LED
  	  digitalWrite(_ledPin, _state);
      // LED on-interval
      blankingTimer.RESET;  
      for (;;) {
        // keep reading input data
        readInputs();
        if (blankingTimer.FIRST_TRIGGER) {
          // turn-off LED after on-interval
          digitalWrite(_ledPin, 0);
          break;
        }
      }
  	}
  }
}

void effectTrailA() {
  // add/remove values from array
  ledData[0] = 1;
  if (ledCountState == 1) {
    ledData[ledIncrement] = 0;
    ledIncrement--;
  } else {
    ledIncrement++;
    ledData[ledIncrement] = 1;
  }
  if (ledIncrement == 0) {
    ledCountState = 0;
  }
  if (ledIncrement >= ledsCount) {
    ledData[ledIncrement] = 1;
    ledCountState = 1;
  }
}

void effectTrailB() {
 if (ledIncrement > ledsCount) {
   ledIncrement = 0;
   clearLEDdata();
 } else{
   ledData[ledIncrement] = 1; 
   ledIncrement++;
 }
}

void readInputs() {
  
}

void loop() {
  readInputs();
  if (effectTimer.TRIGGERED_ON_DURATION) {
    // update LEDs
    effectTrailA();
    //showLEDdata();
  }
  writeLEDs();
}

#include <BlockNot.h>   

// LED driver globals
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
  bool _state = 0;
  uint8_t _ledOut = 0;  
  uint8_t _statesIndex = 0;
  bool _ledStates[ledsCount + 1] = {0};

  if (drawTimer.TRIGGERED_ON_DURATION) {
    // draw each LED one-at-a-time
  	for(uint8_t _count = 0; _count <= ledsCount; _count++) {
  	  // translate data to physical LED layout
  	  uint8_t _led = ledTranslate[_count];
  	  _ledStates[_led] = ledData[_count];
  	  // toggle shift register
  	  if (_led >= _upperHalf){
  	    digitalWrite(outPinHigh, HIGH);
  	    _statesIndex = _upperHalf;
        } else {
  	    _statesIndex = 0;
  	    digitalWrite(outPinHigh, LOW);
  	  }
  	  // translate LED layout to actual I/O pin
  	  for(_ledOut = outPinLow; _ledOut <= outPinHigh - 1; _ledOut++) {
  	    _state = _ledStates[_statesIndex];
  	    if (_led == _statesIndex) {
         break;
        }
  	    _statesIndex++;
  	  }
  	  digitalWrite(_ledOut, _state);
      // LED on-interval
      blankingTimer.RESET;  
      for (;;) {
        readInputs();
        if (blankingTimer.FIRST_TRIGGER) {
          digitalWrite(_ledOut, 0);
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
  // set counter up/down
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

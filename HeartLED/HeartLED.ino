#include <BlockNot.h>

/////////////////// HeartLED ////////////////////
/// by Ben Provenzano III & Nicolas Rodriguez ///
/////////////////////////////////////////////////

// LED driver globals
// 2-11 = LED drive pins
// 12 = LED shift register pin
#define outPinLow 2
#define outPinHigh 12
#define ledsCount 19 
#define refreshRate 50 // LED on-time (us)
uint8_t blankRate = refreshRate / 2;
BlockNot drawTimer(refreshRate, MICROSECONDS); 
BlockNot blankingTimer(blankRate, MICROSECONDS); 
uint8_t _upperHalf = (ledsCount / 2) + 1;
uint8_t ledTranslate[] = {16,6,15,5,1,11,18,8,2,12,14,4,13,3,17,7,9,19,10,0};
bool ledStates[ledsCount + 1] = {0};
bool ledData[ledsCount + 1] = {0};
// LED effect globals
#define drawRate 75 // LED update rate (ms) 
BlockNot effectTimer(drawRate, MILLISECONDS);
bool calibrateLEDs = 0; // calibrate layout mode
uint32_t calibrateSpeed = 3500; // ms
uint8_t ledIncrement = 0;
bool ledCountState = 0;

void setup() {
  Serial.begin(9600);
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

void writeLEDs(bool _calMode) {
  // re-draw all LEDs
  if (drawTimer.TRIGGERED_ON_DURATION) {
    // draw each LED one-at-a-time
  	for(uint8_t _curLED = 0; _curLED <= ledsCount; _curLED++) {
      uint8_t _led;
      if (_calMode == 0) {
        // translate data to physical LED layout
        _led = ledTranslate[_curLED];
      } else {
        // configure layout mode
        _led = _curLED;
      }
  	  ledStates[_led] = ledData[_curLED];
  	  // toggle shift register
      uint8_t _statesIndex = 0;
  	  if (_led >= _upperHalf){
        // enable LEDs 10-19
  	    digitalWrite(outPinLow, HIGH);
  	    _statesIndex = _upperHalf;
      } else {
        // enable LEDs 0-9
        digitalWrite(outPinLow, LOW);
  	    _statesIndex = 0;
  	  }
  	  // translate LED layout to actual I/O pin
      bool _state = 0;
      uint8_t _ledPin = outPinLow + 1;
  	  for(_ledPin; _ledPin <= (outPinHigh); _ledPin++) {
        // search array for selected LEDs on/off state
  	    _state = ledStates[_statesIndex];
  	    if (_led == _statesIndex) {
          break;
        }
  	    _statesIndex++;
  	  }
      if (_calMode == 0) {
        // turn-on LED
  	    digitalWrite(_ledPin, _state);
        // LED on-interval
        blankingTimer.RESET;
        for (;;) {
          // keep reading input data
          readInputs();
          if (blankingTimer.FIRST_TRIGGER) {
            // turn-off LED after on-interval
            digitalWrite(_ledPin, LOW);
            break;
          }
        }
      } else {
        // print current LED
        Serial.print(",");
        Serial.print(_led);
        // flash LED in calibrate mode 
  	    digitalWrite(_ledPin, 1);
        delay(calibrateSpeed);
        digitalWrite(_ledPin, 0);
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

void effectTrailC() {
  clearLEDdata();
  ledData[ledIncrement] = 1;
  if (ledIncrement >= ledsCount) {
    ledIncrement = 0;
  } else{
    ledIncrement++;
  }
}

void readInputs() {
  
}

void loop() {
  readInputs();
  if (calibrateLEDs == 0) {
    if (effectTimer.TRIGGERED_ON_DURATION) {
      // update LEDs
      effectTrailC();
      //showLEDdata();
    }
    writeLEDs(false);
  } else {
    // calibrate mode
    writeLEDs(true);
  }
}


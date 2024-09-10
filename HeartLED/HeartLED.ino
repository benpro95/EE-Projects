#include <BlockNot.h>

/////////////////// HeartLED ////////////////////
/// by Ben Provenzano III & Nicolas Rodriguez ///
/////////////////////////////////////////////////

// LED driver globals
// 2 = LED shift register pin
// 3-12 = LED drive pins
#define outPinLow 2
#define outPinHigh 12
#define ledsCount 19
bool ledCountState;
uint8_t ledIncrement;
int selectedEffect = 0;
uint8_t ledTranslate[] = {16,6,15,5,1,11,18,8,2,12,14,4,13,3,17,7,9,19,10,0};
bool ledData[ledsCount + 1] = {0};
bool ledStates[ledsCount + 1] = {0};
uint8_t _upperHalf = (ledsCount / 2) + 1;
BlockNot ledRefreshTimer(50, MICROSECONDS); // LED refresh interval (us)
BlockNot ledOnIntervalTimer(50, MICROSECONDS); // LED on-time interval (us)
BlockNot effectDrawRateTimer(75, MILLISECONDS);  // LED effect update rate
BlockNot effectChangeTimer(15, SECONDS);  // rate to cycle to next effect
BlockNot calibrateTimer(3, SECONDS); // rate to cycle thru LEDs in calibrate mode
bool calibrateLEDs = 0; // 1 = enable layout calibration mode
bool showLEDdata = 0; // 1 = print LED data to serial port
int heartDataIn = 0;
BlockNot readPulseTimer(15, MILLISECONDS);

void setup() {
  Serial.begin(9600);
  // initialize LED output pins 
  for(uint8_t _count = outPinLow; _count <= outPinHigh; _count++) {
    pinMode(_count, OUTPUT);
    digitalWrite(_count, LOW);
  }
  // initalize LEDs
  resetLEDs();
}

void writeLEDs(bool _calMode) {
  // re-draw all LEDs
  if (ledRefreshTimer.TRIGGERED_ON_DURATION) {
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
        ledOnIntervalTimer.RESET;
        for (;;) {
          // keep reading input data
          readInputs();
          if (ledOnIntervalTimer.FIRST_TRIGGER) {
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
        // keep LED on for duration
        calibrateTimer.RESET;
        for (;;) {
          // keep reading input data
          readInputs();
          if (calibrateTimer.FIRST_TRIGGER) {
            // turn-off LED
            digitalWrite(_ledPin, LOW);
            break;
          }
        }
      }  
  	}
  }
}

// reset LEDs state
void resetLEDs() {
  clearLEDdata();
  ledIncrement = 0;
  ledCountState = 0;
}

// clear the LED data array
void clearLEDdata() {
  uint8_t _count;
  for(_count = 0; _count <= ledsCount; _count++) {
    ledData[_count] = 0; 
  }
}

// print LED data on serial port
void LEDdata() {
  if (showLEDdata == 1) {
    for(uint8_t _count = 0; _count <= ledsCount; _count++) {
      if (ledData[_count] == 1) {
        Serial.print("#");
      } else {
        Serial.print(" ");
      }
      Serial.print(",");
    }
    Serial.print(ledIncrement);
    Serial.println(" ");
  }
}

void effectTrailA() {
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
  if (ledIncrement >= ledsCount) {
    ledIncrement = 0;
    clearLEDdata();
  } else {
    ledIncrement++;
  }
  ledData[ledIncrement] = 1;
}

void effectTrailC() {
  clearLEDdata();
  ledData[ledIncrement] = 1;
  if (ledIncrement >= ledsCount) {
    ledIncrement = 0;
  } else {
    ledIncrement++;
  }
}

void readInputs() {
  if (readPulseTimer.TRIGGERED_ON_DURATION) {
    heartDataIn = analogRead(A0);
  }
}

void loop() {
  // read input data
  //readInputs();
  // draw LED effects
  if (calibrateLEDs == 0) {
    if (effectDrawRateTimer.TRIGGERED_ON_DURATION) {
      if (selectedEffect == 0) {
        effectTrailA();
      }
      if (selectedEffect == 1) {
        effectTrailB();
      }
      if (selectedEffect == 2) {
        effectTrailC();
      }
      if (effectChangeTimer.TRIGGERED_ON_DURATION) {
        if (selectedEffect >= 2) {
          selectedEffect = -1;
        }
        selectedEffect++;
        resetLEDs();
      }
      LEDdata();
    }
    writeLEDs(false);
  } else {
    // calibrate layout mode
    writeLEDs(true);
  }
}


#include <BlockNot.h>
#include <RCSwitch.h>

/////////////////// HeartLED ////////////////////
/// by Ben Provenzano III & Nicolas Rodriguez ///
/////////////////////////////////////////////////

// LED driver globals
// 2 = LED shift register pin
// 3-12 = LED drive pins
RCSwitch mySwitch = RCSwitch();
#define shiftPin 13
#define outPinLow 3
#define outPinHigh 12
#define ledsCount 19
bool cycleEffects = true;
bool disableLEDs = false;
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
BlockNot RFLockout(400, MILLISECONDS);  // deadtime between receiving RF commands
bool RFLock = 0; // RF lock flag
bool calibrateLEDs = 0; // 1 = enable layout calibration mode
bool showLEDdata = 0; // 1 = print LED data to serial port
int heartDataIn = 0;

void setup() {
  Serial.begin(9600);
  // initialize input/output pins
  mySwitch.enableReceive(0);  // Receiver on int 0 => that is [pin #2]
  for(uint8_t _count = outPinLow; _count <= outPinHigh; _count++) {
    pinMode(_count, OUTPUT);
    digitalWrite(_count, LOW);
  }
  pinMode(shiftPin, OUTPUT);
  digitalWrite(shiftPin, LOW);
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
  	    digitalWrite(shiftPin, HIGH);
  	    _statesIndex = _upperHalf;
      } else {
        // enable LEDs 0-9
        digitalWrite(shiftPin, LOW);
  	    _statesIndex = 0;
  	  }
  	  // translate LED layout to actual I/O pin
      bool _state = 0;
      uint8_t _ledPin = outPinLow;
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

// turn all LEDs on
void allLEDsOn() {
  uint8_t _count;
  for(_count = 0; _count <= ledsCount; _count++) {
    ledData[_count] = 1; 
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

void setRFLock() {
  RFLockout.RESET; 
  RFLock = 1;
}

void readInputs() {
  if (RFLockout.FIRST_TRIGGER) {
    RFLock = 0;
  }
  if (mySwitch.available()) {
    unsigned long value = mySwitch.getReceivedValue();
    if (RFLock == 0) {
      if (value == 732101)
      {
        selectedEffect = 0;
        cycleEffects = false;
        disableLEDs = true;
        resetLEDs();
        Serial.println("LEDs off");
        setRFLock();
      }
      if (value == 732102) 
      {
        selectedEffect = 0;
        cycleEffects = true;
        disableLEDs = false;
        Serial.println("LEDs cycle");
        resetLEDs();
        setRFLock();
      } 
      if (value == 732103) 
      {
        selectedEffect = 0;
        cycleEffects = false;
        disableLEDs = false;
        Serial.println("LEDs A");
        resetLEDs();
        setRFLock(); 
      } 
      if (value == 732104) 
      {
        selectedEffect = 1;
        cycleEffects = false;
        disableLEDs = false;
        resetLEDs();
        Serial.println("LEDs B");
        setRFLock(); 
      }
      if (value == 732105) 
      {
        selectedEffect = 2;
        cycleEffects = false;
        disableLEDs = false;
        resetLEDs();
        Serial.println("LEDs C");
        setRFLock();
      }
      if (value == 732106) 
      {
        selectedEffect = -1;
        cycleEffects = false;
        disableLEDs = false;
        resetLEDs();
        allLEDsOn();
        Serial.println("LEDs on");
        setRFLock();
      }
    }
    mySwitch.resetAvailable();
  }
}

void runMode() {
  // print data to serial port
  LEDdata();
  // write data to LEDs
  writeLEDs(false);
  if (disableLEDs == 1) {
    return;
  }
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
    if (cycleEffects == true) {
      if (effectChangeTimer.TRIGGERED_ON_DURATION) {
        if (selectedEffect >= 2) {
          selectedEffect = -1;
        }
        selectedEffect++;
        resetLEDs();
      }
    }  
  }
}

void loop() {
  // read input data
  readInputs();
  if (calibrateLEDs == 1) {
    // calibrate layout mode
    writeLEDs(true);
  } else {
    // draw LED effects
    runMode();
  }
}


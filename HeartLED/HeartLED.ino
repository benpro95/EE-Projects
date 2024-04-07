
uint8_t outPinLow = 2;
uint8_t outPinHigh = 12;
#define ledsCount 19
uint8_t ledTranslate[ledsCount + 1] = {9,13,3,15,5,8,18,11,1,6,16,14,4,17,7,12,2,0,10,19};
// LED bank B start position
uint8_t _upperHalf = (ledsCount / 2) + 1;

void setup() {
  //Serial.begin(115200);
  uint8_t _count;
  // initialize LED output pins 
  for(_count = outPinLow; _count <= outPinHigh; _count++) {
    pinMode(_count, OUTPUT);
    digitalWrite(_count, LOW);
  }
}

void writeLED(uint8_t _ledin) {
  bool _state = 0;
  uint8_t _ledOuts = 0;  
  uint8_t _stateCount = 0;
  bool _ledStates[ledsCount + 1];
  // translate to actual LED position
  uint8_t _led = ledTranslate[_ledin];
  // write LED state array
  for(uint8_t _count = 0; _count <= ledsCount; _count++) {
    if (_count == _led){ 
      _ledStates[_count] = 1;   
    } else {
      _ledStates[_count] = 0; 
    }
  }
  // toggle shift register
  if (_led >= _upperHalf){
    digitalWrite(outPinHigh, HIGH);
    _stateCount = _upperHalf;
  } else {
    _stateCount = 0;
    digitalWrite(outPinHigh, LOW);
  }
  // write out bank A (0-9) or bank B (10-19) LEDs
  for(_ledOuts = outPinLow; _ledOuts <= outPinHigh - 1; _ledOuts++) {
    _state = _ledStates[_stateCount];
    digitalWrite(_ledOuts, _state);
    _stateCount++;
  }
}

void loop() {
  for(int x = 0; x <= ledsCount; x++) {
    writeLED(x);
    delay(150);
  }
}

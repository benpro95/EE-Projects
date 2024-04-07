#include <neotimer.h> 

#define outPinHigh 12
#define outPinLow 2
#define ledsCount 19
uint8_t ledTranslate[] = {9,13,3,15,5,8,18,11,1,6,16,14,4,17,7,12,2,0,10,19};
uint8_t _upperHalf = (ledsCount / 2) + 1;
Neotimer effectTimer = Neotimer(250);
Neotimer ledRefresh = Neotimer(0.10);
bool ledData[ledsCount + 1];
uint8_t ledIncrement = 0;
bool ledCountState = 0;

void clearLEDdata() {
  uint8_t _count;
  for(_count = 0; _count <= ledsCount; _count++) {
    ledData[_count] = 0; 
  }
}

void setup() {
  Serial.begin(115200);
  uint8_t _count;
  // initialize LED output pins 
  for(_count = outPinLow; _count <= outPinHigh; _count++) {
    pinMode(_count, OUTPUT);
    digitalWrite(_count, LOW);
  }
  // initalize LED data array
  clearLEDdata();
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

void effectTrailA() {
  uint8_t _count;
  // add/remove values from array
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
    ledData[0] = 1;
  } else {
    ledData[0] = 0;
  }
}

void effectTrailB() {
 uint8_t _count;
 if (ledIncrement > ledsCount) {
   ledIncrement = 0;
   clearLEDdata();
 } else{
   ledData[ledIncrement] = 1; 
   ledIncrement++;
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

void loop() {
  // re-draw all LEDs
  if(ledRefresh.repeat()){
    for(uint8_t led = 0; led <= ledsCount; led++) {
      if (ledData[led] == 1){
        writeLED(led);
      }
    }
  }
  // update LEDs
  if(effectTimer.repeat()){
    effectTrailA();
    //showLEDdata();
  }
}

///////////////////////////////////////////////////////////////////////////////
// Preamp Controller v1.0 
// by Ben Provenzano III
// 09/28/2022
///////////////////////////////////////////////////////////////////////////////


// Libraries //
#include "LiquidCrystal_I2C.h" // custom for MCP23008-E/P, power button support
#include <IRremote.hpp> // v3+
#include <Arduino.h>
#include <Wire.h> 

// I2C addresses
#define inputResetAddr 0x3A
#define inputSetAddr 0x3B
#define volResetAddr 0x3E
#define volSetAddr 0x3F
#define lcdAddr 0x27

// 16x2 Display
LiquidCrystal_I2C lcd(lcdAddr);
uint8_t lcdCols = 16; // number of columns in the LCD
uint8_t lcdRows = 2;  // number of rows in the LCD
#define lcdBacklightPin 9 // display backlight pin
uint8_t lcdOffBrightness = 175; // standby-off LCD brightness level ***
uint8_t lcdOnBrightness = 245; // online LCD brightness level ***
// custom characters
uint8_t bar1[8] = {0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10};
uint8_t bar2[8] = {0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18};
uint8_t bar3[8] = {0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C};
uint8_t bar4[8] = {0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E};
uint8_t bar5[8] = {0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F};
uint8_t speaker[8] = {B00001,B00011,B01111,B01111,B01111,B00011,B00001,B00000};
uint8_t sound[8] = {B00001,B00011,B00101,B01001,B01001,B01011,B11011,B11000};

// IR
int IRpin = 8; // receiver in
bool irCodeScan = 0; // enable IR codes on the terminal (1)
uint8_t debounceIR = 75; // IR receive max rate (ms)
uint32_t irRecv;

// Input selector
String inputSelected;
String inputName1 = "DIGITAL   "; //***
String inputName2 = "AUX       "; //***
String inputName3 = "PHONO     "; //***
String inputName4 = "AIRPLAY   "; //***
String inputName5 = "PC        "; //*** 
uint8_t inputRelayCount = 3; 

// Volume control
uint8_t volCoarseSteps = 6; // volume steps to skip for coarse adjust ***
uint8_t volRelayCount = 8; // number of relays on volume attenuator board ***
uint8_t volMaxLimit = 90; // max percentage of volume range (100%) when in limit mode ***
uint8_t volMinLimit = 50; // min percentage of volume range (0%)
#define volControlDown 1
#define volControlUp 2
#define volControlSlow 1 
#define volControlFast 2 
#define volLimitPin 12 // volume limit switch
bool volLimitFlag = 1;
uint8_t volLastLevel = 0;
uint8_t volLevel = 0;
uint8_t volSpan;
uint8_t volMax;
uint8_t volMin;
bool volMute;

// 50Hz high-pass filter 
#define hpfRelayPin 10 // HPF relay pin
bool hpfState = 1; // HPF default startup state 0=on, 1=off ***

// Motor Pot
#define motorInit 1 // motor initialization
#define motorSettled 2 // pot at resting state
#define motorInMotion 3 // pot is moving right now
#define motorCoasting 4 // pot just passed read value
#define potThreshold 4 // pot value change threshold ***
#define potRereads 6 // amount of pot readings ***
#define potAnalogPin 1 // pot reading pin A1
#define motorPinCW 16 // motor H-bridge CW pin
#define motorPinCCW 17 // motor H-bridge CCW pin
#define potMinRange 30  // lowest pot reading
#define potMaxRange 1023 // highest pot reading
int potValueLast; // range from 0-1023
uint8_t volPotLast;
uint8_t potState;

// Power
#define powerRelayPin 7
#define powerButtonPin 5
uint8_t lastPowerButton = 0;
uint8_t powerButton = 0;
bool powerCycle = 0;
bool powerState = 0;
int startDelay = 10; // startup delay in seconds, unmutes after ***
int shutdownTime = 5; // shutdown delay before turning off aux power ***
int initStartDelay = 3; // delay on initial cold start
uint8_t debounceDelay = 50; // button debounce delay in ms
uint32_t powerButtonMillis;


///////////////////////////////////////////////////////////////////////////////


// reverse byte function 
int reverseByte(uint8_t _inByte) 
{
  uint8_t _revByte;
  for (int _revBit = 0; _revBit < 8; _revBit++) {
    if (bitRead(_inByte, _revBit) == 1) {
      bitWrite(_revByte, (7 - _revBit), 1);
    } else {
      bitWrite(_revByte, (7 - _revBit), 0);
    }
  }
  return _revByte;
}


// mapping function (fixed)
long l_map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min + 1) /
         (in_max - in_min + 1) + out_min;
} 


// read the pot, smooth out the data
int readPotWithSmoothing(uint8_t analog_port_num, uint8_t reread_count)
{
  int sensed_port_value = 0;
  for (uint8_t i = 0; i < reread_count; i++) {
    sensed_port_value += analogRead(analog_port_num);
    // delayMicroseconds(200);
  }
  if (reread_count > 1) {
    sensed_port_value /= reread_count;
  }
  return sensed_port_value;
}


// read the pot, and clip to keep any stray values in that range
int readPotWithClipping(int sensed_pot_value)
{
  int temp_volume;
  temp_volume = l_map(sensed_pot_value,
      0, // potMinRange,
      potMaxRange,
      volMin, volMax);
  return temp_volume;
}


// runs when pot state changes
void potValueChanges(void)
{
  uint8_t old_vol;
  uint8_t temp_volume;
  int sensed_pot_value;
  // read pot smoothed output
  sensed_pot_value = readPotWithSmoothing(
    potAnalogPin, potRereads
  );  // to smooth it out
  // read an average pot value
  if (abs(sensed_pot_value - potValueLast) > potThreshold) {
    // 1-5 is a good value to ignore noise
    // convert the pot raw value into our correct volume min..max range
    old_vol = volLevel; // the setting before user touched the pot
    // read pot clipped output
    temp_volume = readPotWithClipping(
      sensed_pot_value
    );
    if (temp_volume == old_vol) {
      // don't update if there was no effective change
      return;
    }
    // volume state changed    
    volLevel = temp_volume;
    if (temp_volume > old_vol) {
      if (volMute == 1) {
        // tell the system we out of mute mode
        volMute = 0;
        volUpdate(volLevel, 1);
      }
      else {
        // not in mute mode, set volume up
        volUpdate(temp_volume, 0);
      }
    }
    else {
      // not in mute mode, set volume down
      volUpdate(temp_volume, 0);
    }
    // real change, save the changed state
    volPotLast = volLevel;
    potValueLast = sensed_pot_value;
  }
}


// motor pot PID control loop
void motorControlLoop(void)
{
  int target_pot_wiper_value;
  int pot_pid_value;
  // given the 'IR' set volume level, find out what wiper value to compare
  target_pot_wiper_value = l_map(volLevel, volMin, volMax,
          potMinRange,
          potMaxRange);
  // this is the value of the pot, from the a/d converter
  pot_pid_value = readPotWithSmoothing(
    potAnalogPin, potRereads
  );
  // average out the values
  if (abs(target_pot_wiper_value - pot_pid_value) <= 8) {
    // stop the motor!
    digitalWrite(motorPinCCW, LOW);  // stop turning left
    digitalWrite(motorPinCW,  LOW);  // stop turning right
    potState = motorCoasting;
    delay(5);
    return;
  }
  else {
    // not at target volume yet
    if (pot_pid_value < target_pot_wiper_value) {
      // turn clockwise
      digitalWrite(motorPinCCW, LOW);
      digitalWrite(motorPinCW,  HIGH);
    }
    else if (pot_pid_value > target_pot_wiper_value) {
      // turn counter-clockwise
      digitalWrite(motorPinCW,  LOW);
      digitalWrite(motorPinCCW, HIGH);
    }
  }
} 


// motor pot state-driven dispatcher
void motorPot(void)
{
  int pot_pid_value = 0;
  static int motor_stabilized;
  if (powerState == 0)
	return;  
  // If max_vol == min_vol then don't run the motor
  if (volSpan == 0)
	return;  
// action states
#if 0
  if (volMute == 1)
    return; // don't spin the motor just muted
#endif
// action states
  switch (potState) {
  case motorInit:
    // initial state, just go to 'settled' from here
    potState = motorSettled;
    volPotLast = volLevel;
    break;
//---------------//
  case motorSettled:
    /*
     * if we are 'settled' and the pot wiper changed,
     * it was via a human.  this doesn't affect our
     * motor-driven logic.
     */
    // if the volume changed via the user's IR, this should
    // trigger us to move to the next state
    if (volLevel != volPotLast) {
      potState = motorInMotion;
      potValueLast = readPotWithSmoothing(
        potAnalogPin, potRereads
      );
    }
    volPotLast = volLevel;
    break;
//---------------//
  case motorInMotion:
    /*
     * if the motor is moving, we are looking for our target
     * so we can let go of the motor and let it 'coast' to a stop
     */
    motor_stabilized = 0;
    motorControlLoop();
    break;
//---------------//
  case motorCoasting:
    /*
     * we are waiting for the motor to stop
     * (which means the last value == this value)
     */
    delay(20);
    pot_pid_value = readPotWithSmoothing(
      potAnalogPin, potRereads
    );
    if (pot_pid_value == potValueLast) {
      if (++motor_stabilized >= 5) {
        // yay! we reached our target
        potState = motorSettled;
      }
    }
    else {
      // we found a value that didn't match,
      // so reset our 'sameness' counter
      motor_stabilized = 0;
    }
//---------------//
    // this is the operating value of the pot,
    // from the a/d converter
    potValueLast = pot_pid_value;
    break;
//
  default:
    break;
  }
}


// setup volume range
void volRange()
{ // read volume limit switch 
  int _limitpin = digitalRead(volLimitPin);
  if (_limitpin == 1) { // pin high=no limit, low=limited
    volLimitFlag = 1;
  } else {
    volLimitFlag = 0;
  }  // for 7 relays, this would be 128-1 = 127
  uint8_t max_byte_size = (1 << volRelayCount) - 1;
  volMin = (max_byte_size * volMinLimit) / 100;
  if (volLimitFlag == 1) {
    volMax = max_byte_size; 
  } else {
  	volMax = (max_byte_size * volMaxLimit) / 100;
  }
  volSpan = abs(volMax - volMin);
}


// increment volume up/down slow/fast
void volIncrement (uint8_t dir_flag, uint8_t speed_flag)
{
  if (dir_flag == volControlUp) {
    if (volLevel >= volMax)
      return;
  } 
  else if (dir_flag == volControlDown) {
    if (volLevel <= volMin)
      return;
  }
  if (volMute == 1 ) {
    volMute = 0;
    volUpdate(volLevel, 1);
    return;
  }
  if (dir_flag == volControlUp) {
    if (speed_flag == volControlSlow) {    
      // volume up-slow
      if (volLevel < volMax &&
         (volLevel + 1) < volMax) {
        volLevel += 1;
      } 
      else {
        volLevel = volMax;
      }
      volUpdate(volLevel, 0);  
    } 
    else if (speed_flag == volControlFast) {
      // volume up-fast
      if (volLevel < (volMax - volCoarseSteps)) {
        volLevel += volCoarseSteps;
      } 
      else {
        volLevel = volMax;
      }
      volUpdate(volLevel, 0); 
    }
  }
  else if (dir_flag == volControlDown) {
    if (speed_flag == volControlSlow) {
      // volume down-slow
      if (volLevel > volMin &&
         (volLevel - 1) > volMin) {
        volLevel -= 1;
      } 
      else {
        volLevel = volMin;
      }
      volUpdate(volLevel, 0); 
    } 
    else if (speed_flag == volControlFast) {
      // volume down-fast
      if (volLevel > (volMin + volCoarseSteps)) {
        volLevel -= volCoarseSteps;
      } 
      else {
        volLevel = volMin;
      }
      volUpdate(volLevel, 0); 
    }
  }
}


// set a specific volume level
void volUpdate(uint8_t _vol, uint8_t _force) 
{
// set volume relays
  if (volMute == 0) {
    setRelays(volSetAddr, volResetAddr, _vol, volRelayCount, _force);    
    volLastLevel = volLevel;
    // update display
    lcdVolume(_vol); // redraw progress bar and percentage
    inputUpdate(0); // just redraw input 
  }  
}


// mute system (0=mute 1=unmute 2=toggle)
void muteSystem(uint8_t _state)
{	
  if (_state >= 3) {  // invalid 
    return;
  } 
  if (_state == 2) {  // mute toggle 
    if (volMute == 0) {
      _state = 0;
    } 
    else {
      _state = 1;
    }
  } 
  if (_state == 0) {  // mute on  
    if (volSpan == 0) { // return if min_vol == max_vol
      return;
    } // stop motor turning
    digitalWrite(motorPinCW, LOW);   
    digitalWrite(motorPinCCW, LOW);
    // set volume to min
    volUpdate(volMin, 1);
    // mute status on display
    lcd.setCursor(2,1);
    lcd.print("          "); 
    lcd.setCursor(0,0);
    lcd.print("----------------"); 
    lcd.setCursor(12,1);
    lcd.print("MUTE"); 
    lcd.createChar(0, speaker);
    lcd.setCursor(0,1);
    lcd.print(char(0));
    // set mute flag
    volMute = 1;	
  }
  if (_state == 1) {	// unmute
    volMute = 0;  
    lcd.setCursor(0,0);
    lcd.print("                "); 
    volUpdate(volLevel, 1);
    // music icon    
    lcd.createChar(0, sound);
    lcd.setCursor(0,1);
    lcd.print(char(0));	
  }
}


// set a specific audio input
void inputUpdate (uint8_t _input) 
{
 if (volMute == 0) {
	bool _lcdonly = 0;	
	uint8_t _state; 
	String _name; 
	// select input
	if (_input == 0) {  // just update display
	   _lcdonly = 1;
	}  
	if (_input == 1) {  // input #1 
	  _state = B00000001;
	  _name = inputName1;
	}    
	if (_input == 2) {  // input #2
	  _state = B00000010;
	  _name = inputName2;
	}  
	if (_input == 3) {  // input #3
	  _state = B00000100;
	  _name = inputName3;
	}
  if (_input == 4) {  // input #4 (display only)
    _lcdonly = 1;
    inputSelected = inputName4;
  }  
  if (_input == 5) {  // input #5 (display only)
    _lcdonly = 1;
    inputSelected = inputName5;
  }    
	if (_input >= 6) {  // invalid setting
	  return;
	}
	if (_lcdonly == 0) {
	  // set input relays
	  setRelays(inputSetAddr, inputResetAddr, _state, inputRelayCount, 1);  
	  inputSelected = _name;	  
	} else {
	    _name = inputSelected;
	}
	// update display
	lcd.setCursor(2,1);
	lcd.print(_name); 
 }	  
}


// set a specific audio input
void hpfControl (uint8_t _state) 
{
  if (volMute == 0) {
	if (_state == 0) {  // HPF on
	  hpfState = 0;
	}  
	if (_state == 1) {  // HPF off
	  hpfState = 1;
	}    
	if (_state == 2) {  // toggle state
      hpfState =! hpfState;
	}  
	if (_state >= 3) {  // invalid 
	  return;
	}
    if (hpfState == 0) {
	  lcd.setCursor(2,1);
	  lcd.print("HPF Off    ");     
	  digitalWrite(hpfRelayPin, HIGH);	
    } else {
      lcd.setCursor(2,1);
	  lcd.print("HPF On   ");   
	  digitalWrite(hpfRelayPin, LOW);	 
    }	
    delay(2000);
    inputUpdate(0); // only update display
  }	  
}


// set a relay controller board (volume or inputs)
void setRelays(uint8_t pcf_a, uint8_t pcf_b,  // first pair of i2c addr's
      uint8_t vol_byte,                    // the 0..255 value to write
      uint8_t installed_relay_count,    // how many bits are installed
      uint8_t forced_update_flag)  // forced or relative mode (1=forced)
{
  int bitnum;
  uint8_t mask_left;
  uint8_t mask_right;
  uint8_t just_the_current_bit;
  uint8_t just_the_previous_bit;
  uint8_t shifted_one_bit;
  // this must to be able to underflow to *negative* numbers
  // walk the bits and just count the bit-changes and save into left and right masks
  mask_left = mask_right = 0;
  //
  // this loop walks ALL bits, even the 'mute bit'
  for (bitnum = (installed_relay_count-1); bitnum >= 0 ; bitnum--) {
    //
    // optimize: calc this ONLY once per loop
    shifted_one_bit = (1 << bitnum);
    //
    // this is the new volume value; and just the bit we are walking
    just_the_current_bit = (vol_byte & shifted_one_bit);
    //
    // logical AND to extra just the bit we are interested in
    just_the_previous_bit = (volLastLevel & shifted_one_bit);
    //
    // examine our current bit and see if it changed from the last run
    if (just_the_previous_bit != just_the_current_bit ||
        forced_update_flag == 1) {
      //	
      // latch the '1' on the left or right side of the relays
      if (just_the_current_bit != 0) {
        // a '1' in this bit pos
        // (1 << bitnum);
        mask_left |= ((uint8_t)shifted_one_bit);
      } 
      else { // (1 << bitnum);
        mask_right |= ((uint8_t)shifted_one_bit);
      }
    } // the 2 bits were different
  } // for each of the 8 bits
// set upper relays
  PCFexpanderWrite(pcf_b, mask_right);
  PCFexpanderWrite(pcf_a, 0x00);
  resetPCF(pcf_a, pcf_b);
// set lower relays
  PCFexpanderWrite(pcf_a, mask_left);
  PCFexpanderWrite(pcf_b, 0x00);
  resetPCF(pcf_a, pcf_b);
}


// reset all pins PCF8574A I/O expander
void resetPCF(uint8_t pcf_a, uint8_t pcf_b)
{
  // let them settle before we unlatch them
  delayMicroseconds(3700);
  // do the unlatch (relax) stuff
  // left side of relay coil
  PCFexpanderWrite(pcf_a, B00000000);
  // right side of relay coil
  PCFexpanderWrite(pcf_b, B00000000);
  // let the relay hold for a while
  delayMicroseconds(3700);
}


// read a byte from PCF8574A I/O expander
uint8_t PCFexpanderRead(int address) 
{
 uint8_t _data;
 Wire.requestFrom(address, 1);
 if(Wire.available()) {
   _data = Wire.read();
 }
 return _data;
}


// write a byte to PCF8574A I/O expander
void PCFexpanderWrite(uint8_t address, uint8_t _data ) 
{
 Wire.beginTransmission(address);
 Wire.write(_data);
 Wire.endTransmission(); 
}


// volume status on display
void lcdVolume(int _level) {
// update percentage  
  int _lcdlevel = map(_level, volMin, volMax, 0, 100);
  if (_lcdlevel == 0) {
    return;
  } else {
  	if (_lcdlevel == 100) {
      lcd.setCursor(12,1);
      lcd.print("    "); 
      lcd.setCursor(13,1);
      lcd.print("MAX"); 
    } else {
      lcd.setCursor(12,1);
      lcd.print("    "); 
      lcd.setCursor(13,1);
      lcd.print(String(_lcdlevel) + String("%"));
    }
  // draw volume progress bar if unmuted
  _lcdlevel = map(_level, volMin, volMax, 0, 1024);
  lcdBar(0,_lcdlevel,0,1024);
  }
}   


// display a progress bar
void lcdBar (int row, int var, int minVal, int maxVal)
{
  int block = map(var, minVal, maxVal, 0, 16); // Block represent the current LCD space
  int line = map(var, minVal, maxVal, 0, 80); // Line represent the lines to be displayed
  int bar = (line-(block*5)); // Bar represent the actual lines that will be printed
  // Print all the filled blocks
  for (int x = 0; x < block; x++)                       
  {
    lcd.setCursor (x, row);
    lcd.write (1023);
  }
  // Set the cursor at the current block and print the numbers of line needed
  lcd.setCursor (block, row);                           
  if (bar != 0) lcd.write (bar);
  if (block == 0 && line == 0) lcd.write (1022); // Unless there is nothing to print, show blank
  // Print all the blank blocks
  for (int x = 16; x > block; x--) 
  {
    lcd.setCursor (x, row);
    lcd.write (1022);
  }
}


// display progress bar for # of seconds 
void lcdTimedBar(uint8_t _sec, bool _doublebar)
{ // seconds to loops
  int _loops = _sec * 32;
  for ( int _incr = 0; _incr < _loops; _incr++ ) {
  	if (_doublebar == 1) {
      lcdBar(0,_incr,0,_loops);
    }
    lcdBar(1,_incr,0,_loops);
    // also reset PCF expanders
    resetPCF(volSetAddr,volResetAddr);
    resetPCF(inputSetAddr,inputResetAddr);
    _incr = _incr + 4;
    delay(40);
  }    
}  


// display in standby mode
void lcdStandby()
{ 
  analogWrite(lcdBacklightPin, lcdOffBrightness);	
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("ProHiFi");
  lcd.setCursor(0,1);
  lcd.print("Preamp v5.0");
  lcd.setCursor(10,0);  
  lcd.createChar(0, speaker);
  lcd.setCursor(15,1);
  lcd.print(char(0));
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
}  


// receive IR remote commands 
// Philips SRP9232D/27 Universal Remote
// programmed with code '1376' (Onkyo Audio)
// commented codes are for Xmit
void irReceive() 
{
  if (IrReceiver.decode()) {
  digitalWrite(LED_BUILTIN, HIGH); 
  // code detected
  if (powerState == 1) {
    if (IrReceiver.decodedIRData.address == 0x6DD2) { // address      
	  if (IrReceiver.decodedIRData.command == 0x3) { // volume down fast (VOL-) 1270267967
	    volIncrement(1,2);       	
	  }      
	  if (IrReceiver.decodedIRData.command == 0x2) { // volume up fast (VOL+) 1270235327
	    volIncrement(2,2);
	  }	  
	  if (IrReceiver.decodedIRData.command == 0x5) { // mute toggle (MUTE) 1270259807
	    muteSystem(2);
	  }   
	}	      
    if (IrReceiver.decodedIRData.address == 0xACD2) { // address      
	  if (IrReceiver.decodedIRData.command == 0xE) { // input (1) 1261793423
	    inputUpdate(1); 
	  }      
	  if (IrReceiver.decodedIRData.command == 0xF) { // input (2) 1261826063
	    inputUpdate(2); 
	  }    
	  if (IrReceiver.decodedIRData.command == 0x10) { // input (3) 1261766903
     	inputUpdate(3); 
	  } 
      if (IrReceiver.decodedIRData.command == 0x11) { // input (4) just displays AIRPLAY
        inputUpdate(4); 
      }  
      if (IrReceiver.decodedIRData.command == 0x12) { // input (5) just displays PC
        inputUpdate(5); 
      }            
	  if (IrReceiver.decodedIRData.command == 0x17) { // force mute (0) 1261824023
       	muteSystem(0); 
	  } 	         	  	    
	}	                                    
    if (IrReceiver.decodedIRData.address == 0x6CD2) { // address      
	  if (IrReceiver.decodedIRData.command == 0x9B) { // volume down slow (DOWN) 1261885734
	    volIncrement(1,1);
	  }      
	  if (IrReceiver.decodedIRData.command == 0x9A) { // volume up slow (UP) 1261853094
	    volIncrement(2,1);
	  }    
	  if (IrReceiver.decodedIRData.command == 0x8D) { // HPF control (PLAY) 1261875534
	 	hpfControl(2);
	  }         	  	    
	}	           
  }    
  if (IrReceiver.decodedIRData.address == 0x6DD2) { // address
    if (IrReceiver.decodedIRData.command == 0x4) { // power toggle (POWER) 1270227167
      powerState = !powerState;  
      powerCycle = 1;
      delay(100);
    }  
  }    
  if (IrReceiver.decodedIRData.address == 0x6CD2) { // address    
    if (IrReceiver.decodedIRData.command == 0x99) { // power on (HOME) 1261869414
      if (powerState == 0) {
        powerState = 1;  
        powerCycle = 1;
      }  
    }  
    if (IrReceiver.decodedIRData.command == 0x8E) { // power off (STOP) 1261859214
      if (powerState == 1) {
        powerState = 0; 
        powerCycle = 1;
      }  
    }  
  }       
  // display IR codes on terminal
    if (irCodeScan == 1) {   
	  // display IR codes on terminal   
	  Serial.println("--------------");   
	  if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
	    IrReceiver.printIRResultRawFormatted(&Serial, true);  
	  } else {
	    IrReceiver.printIRResultMinimal(&Serial); 
	  }  
	  Serial.println("--------------"); 
	  irRecv = IrReceiver.decodedIRData.decodedRawData;
	  uint32_t irtype = IrReceiver.decodedIRData.protocol;
	  // display v3 format codes
	  Serial.print("IR protocol: "); 
	  Serial.println(irtype);       
	  int32_t irRecv_2s = -irRecv;
	  irRecv_2s = irRecv_2s * -1;
	  Serial.print("IR v3 decimal: "); 
	  Serial.println(irRecv, DEC); 
	  Serial.print("IR v3 (2's-compliment) decimal: "); 
	  Serial.println(irRecv_2s, DEC);  
	  Serial.print("IR v3 hex: "); 
	  Serial.println(irRecv, HEX); 
	  // convert 32-bit code into 4 seperate bytes (pointer)
	  uint8_t *irbyte = (uint8_t*)&irRecv;
	  Serial.print("IR v3 address: "); 
	  Serial.println(irbyte[3], HEX); 
	  Serial.print("IR v3 command: "); 
	  Serial.println(irbyte[2], HEX); 
	  Serial.println("--------------"); 
	  // reverse each byte (v3 format to v2 format)
	  uint8_t irbyte0 = reverseByte(irbyte[0]); 
	  uint8_t irbyte1 = reverseByte(irbyte[1]); 
	  uint8_t irbyte2 = reverseByte(irbyte[2]); 
	  uint8_t irbyte3 = reverseByte(irbyte[3]); 
	  // assemble 4 reversed bytes into 32-bit code
	  uint32_t irv2 = irbyte0; // shift in the first byte
	  irv2 = irv2 * 256 + irbyte1; // shift in the second byte
	  irv2 = irv2 * 256 + irbyte2; // shift in the third byte
	  irv2 = irv2 * 256 + irbyte3; // shift in the last byte
	  // display v2 format codes
	  int32_t irv2_2s = -irv2;
	  irv2_2s = irv2_2s * -1;
	  Serial.print("IR v2 decimal: "); 
	  Serial.println(irv2, DEC); 
	  Serial.print("IR v2 (2's-compliment) decimal: "); 
	  Serial.println(irv2_2s, DEC);  
	  Serial.print("IR v2 hex: "); 
	  Serial.println(irv2, HEX);
	  Serial.println(" "); 
	  Serial.println("v2 2's-comp is used by Automate/Xmit");
	  Serial.println("ignore decimal values with Sony codes, use hex");
	  Serial.println(" ");
    }
  delay(debounceIR);
  IrReceiver.resume();      
  digitalWrite(LED_BUILTIN, LOW);   
  }
}


// power on/off
void setPowerState() {
  // read pin state from MCP23008
  int reading = lcd.readPin(powerButtonPin);
  // if switch changed
  if (reading != lastPowerButton) {
    // reset the debouncing timer
    powerButtonMillis = millis();
  }
  if ((millis() - powerButtonMillis) > debounceDelay) {
    // if the button state has changed:
    if (reading != powerButton) {
      powerButton = reading;
      // power state has changed!
      if (powerButton == 1) { 
        powerState = !powerState;  
        powerCycle = 1;
      }
    }
  }
  lastPowerButton = reading; 
  // power state actions  
  if (powerCycle == 1){  
	// calculate volume limits
	volRange();   	  
	// reset display
	lcd.clear();
	// one-shot triggers
    if (powerState == 1){
      /// runs once on boot ///
      startup();
    } else {  
      /// runs once on shutdown ///
      shutdown();
    }  
  powerCycle = 0;  
  }  
}

void shutdown() {
  // shutdown routines
  muteSystem(0); // mute    	
  // shutdown animation
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Shutting Down..."); 	
  lcdTimedBar(shutdownTime,0);	
  lcdStandby();
  // turn off preamp power here
  digitalWrite(powerRelayPin, LOW);  	
}

// startup routines
void startup() 
{ 
  analogWrite(lcdBacklightPin, lcdOnBrightness);
  lcd.setCursor(0,0);
  lcd.print("Starting Up...");
  // turn on preamp power here
  digitalWrite(powerRelayPin, HIGH);  
  delay(650);
  // loading bar 
  lcd.clear();
  if (volLimitFlag == 0){
    lcd.setCursor(0,0);
    lcd.print("Volume Limit On");
    lcdTimedBar(startDelay,0);
  } else {
    lcdTimedBar(startDelay,1);
  }
  lcd.clear();
  // music icon
  lcd.createChar(0, sound);
  lcd.setCursor(0,1);
  lcd.print(char(0));
  // set volume from pot
  volMute = 0;
  potState = motorInit;
  volUpdate(volLevel, 1);
  // set audio input
  inputUpdate(1);
}

///////////////////////////////////////////////////////////////////////////////
// initialization 
void setup() 
{ 	

  // 16x2 display (calls Wire.begin)
  lcd.begin(lcdCols,lcdRows); 	
  // reset expanders
  resetPCF(volSetAddr,volResetAddr);
  resetPCF(inputSetAddr,inputResetAddr);  
  // LED status LED
  pinMode(LED_BUILTIN, OUTPUT);  
  digitalWrite(LED_BUILTIN, HIGH);    
  // motor pot
  pinMode(potAnalogPin, INPUT);   
  pinMode(motorPinCW, OUTPUT);
  pinMode(motorPinCCW, OUTPUT);
  digitalWrite(motorPinCW, LOW);
  digitalWrite(motorPinCCW, LOW);
  // display backlight
  pinMode(lcdBacklightPin, OUTPUT);  
  analogWrite(lcdBacklightPin, lcdOffBrightness);
  // preamp power control
  pinMode(powerRelayPin, OUTPUT);  
  digitalWrite(powerRelayPin, LOW);  
  // HPF control
  pinMode(hpfRelayPin, OUTPUT);  
  digitalWrite(hpfRelayPin, hpfState);
  hpfState =! hpfState;
  // volume limit switch 
  pinMode(volLimitPin, INPUT_PULLUP);   
  // calculate volume limits
  volRange();   
  // IR remote
  IrReceiver.begin(IRpin);  
  // initial boot display
  lcd.createChar(1, bar1);
  lcd.createChar(2, bar2);
  lcd.createChar(3, bar3);
  lcd.createChar(4, bar4);
  lcd.createChar(5, bar5);
  lcdTimedBar(initStartDelay,1);
  // serial support
  if (irCodeScan == 1){  
    Serial.begin(9600);
  } // startup complete
  lcdStandby();
}

///////////////////////////////////////////////////////////////////////////////
// superloop
void loop()
{
  // IR remote
  irReceive();  
  // motor potentiometer
  if (powerState == 1) {
    if (potState == motorSettled ||
        potState == motorInit) {
      potValueChanges();
    }
    motorPot();
  }
  // power management
  setPowerState();	  
}

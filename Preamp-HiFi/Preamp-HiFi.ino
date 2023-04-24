///////////////////////////////////////////////////////////////////////////////
// Preamp Controller v2.0 
// by Ben Provenzano III
// 04/03/2023
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

// Configuration (0=Ben, 1=Seth)
#define preampType 0

// 16x2 Display
LiquidCrystal_I2C lcd(lcdAddr);
uint8_t lcdCols = 16; // number of columns in the LCD
uint8_t lcdRows = 2;  // number of rows in the LCD
#define lcdBacklightPin 9 // display backlight pin
uint8_t lcdOffBrightness = 140; // standby-off LCD brightness level ***
uint8_t lcdOnBrightness = 250; // online LCD brightness level ***
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
uint32_t irRecv = 0;

// Input selector
uint8_t inputSelected = 1;
uint8_t inputRelayCount = 3;
char *inputTitle[] = {"          "};
static const char *inputNames0[] =
{
  ("DIGITAL   "),
  ("AUX       "),
  ("PHONO     "),
  ("AIRPLAY   "),
  ("OPTICAL   ")
};
static const char *inputNames1[] =
{
  ("AIRPLAY   "),
  ("OPTICAL   "),
  ("AUX       "),
  ("N/A       "),
  ("N/A       ")
};

// Volume control
#if (preampType == 0)
  uint8_t volCoarseSteps = 6; // volume steps to skip for coarse adjust ***
  uint8_t volMaxLimit = 90; // max percentage of volume range (100%) when in limit mode ***
  uint8_t volMinLimit = 40; // min percentage of volume range (0%)
#else
  uint8_t volCoarseSteps = 4;
  uint8_t volMaxLimit = 85; 
  uint8_t volMinLimit = 30; 
#endif
uint8_t volRelayCount = 8; // number of relays on volume attenuator board ***
bool volLimitFlag = 1; // limit on by default
#define volControlUp 2
#define volControlDown 1
#define volControlSlow 1 
#define volControlFast 2 
uint8_t volLastLevel = 0;
uint8_t volLevel = 0;
uint8_t volSpan = 0;
uint8_t volMax = 0;
uint8_t volMin = 0;
bool volMute = 0;

// 50Hz high-pass filter 
#define hpfRelayPin 10 // HPF relay pin
bool hpfState = 0;

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
int potValueLast = 0; // range from 0-1023
uint8_t volPotLast = 0;
uint8_t potState = 0;

// Toggle Switch
#define toggleSwitchPin 12 // pin 
uint32_t toggleSwitchMillis;
uint8_t lastToggleSwitch = 0;
uint8_t toggleSwitch = 0;

// Power
#define powerRelayPin 7
#define powerButtonPin 5
#if (preampType == 0)
  int startDelay = 5; // startup delay in seconds, unmutes after ***
#else
  int startDelay = 3;
#endif
int shutdownTime = 3; // shutdown delay before turning off aux power ***
int initStartDelay = 2; // delay on initial cold start ***
uint8_t lastPowerButton = 0;
uint8_t powerButton = 0;
bool powerCycle = 0;
bool powerState = 0;
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
    lcd.setCursor(0,1); // music icon
    lcd.print(char(0));
    lcd.setCursor(1,1);
    lcd.print(" ");
    inputUpdate(0); // just redraw input 
  }  
}


// mute system (0=mute 1=unmute 2=toggle)
void muteSystem(uint8_t _state)
{	
  if (_state > 2) {  // invalid 
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
    lcd.setCursor(0,1);
    lcd.print(char(6));
    // set mute flag
    volMute = 1;	
  }
  if (_state == 1) {	// unmute, init
    lcd.setCursor(0,0); // clear top of screen
    lcd.print("                ");
    volMute = 0;
    volUpdate(volLevel, 1);
  }
}


// set a specific audio input
// (0=update display only, 99=last input)
void inputUpdate (uint8_t _input) 
{
  if (volMute == 0) {
    uint8_t _state = 0;
    bool _draw = 1;
    bool _set = 0;
    // set last input selected
    if (_input == 99) { 
      _input = inputSelected;
      _draw = 0;
    } else {
      if (_input != 0) { 	
        // save input state  
        inputSelected = _input; 
      }  
    } // exit on invalid input
    if (_input > 5) { 
      return;
    }
    // select input title
    if (_input > 0) { 
      if (preampType == 0) {	
        inputTitle[0] = inputNames0[_input - 1];
      } else {
        inputTitle[0] = inputNames1[_input - 1];
      }  
    }
  	// select input relay
  	if (_input == 1 || _input > 3) {  // input #1
  	  _state = B00000001;
  	  _set = 1;
  	}    
  	if (_input == 2) {  // input #2
  	  _state = B00000010;
  	  _set = 1;
  	}  
  	if (_input == 3) {  // input #3
  	  _state = B00000100;
  	  _set = 1;
  	}
    // set input relays
  	if (_set == 1 && preampType == 0) {
      setRelays(inputSetAddr, inputResetAddr, _state, inputRelayCount, 1);   
  	}
  	// update display
  	if (_draw == 1) { 
  	  lcd.setCursor(2,1);
  	  lcd.print(inputTitle[0]); 
  	}  
  }	  
}


// high-pass filter control
void hpfControl (uint8_t _state) 
{
  if (preampType != 0) {
  	return;
  }
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
	  lcd.print("HPF Off   ");  
	  digitalWrite(hpfRelayPin, HIGH);	
    } else {
      lcd.setCursor(2,1);
	  lcd.print("HPF On    ");
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
  if (_lcdlevel <= 0) {
    return;
  } else {
  	if (_lcdlevel >= 100) {
      lcd.setCursor(12,1);
      lcd.print("    "); 
      lcd.setCursor(13,1);
      lcd.print("MAX"); 
    } else {	
      lcd.setCursor(12,1);
      lcd.print("    "); 
      lcd.setCursor(13,1);
      lcd.print(_lcdlevel);
      lcd.setCursor(15,1);
      lcd.print("%");
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
void lcdTimedBar(int _sec, uint8_t _single) { 
  uint32_t _ms = _sec * 6; 
  uint32_t _colcount;
  uint32_t _segcount;
  uint8_t _rowcount;
  uint8_t _line = 1;
  if (_single == 0) {
    return;
  }
  // draw bar on each row
  for(_rowcount = 0; _rowcount < _single; _rowcount++) {
    // draw bar on each collumn
    for(_colcount = 0; _colcount < lcdCols; _colcount++) {
      // draw bar segments  
      for(_segcount = 0; _segcount < 5; _segcount++) {  
        lcd.setCursor(_colcount,_line);
        // draw custom segment 
        lcd.write(_segcount + 1);
        // also reset PCF expanders
        resetPCF(volSetAddr,volResetAddr);
        resetPCF(inputSetAddr,inputResetAddr);     
        // pause  
        delay(_ms);
      }    
    } // decrement to next row
    _line--;
  }
}  


// receive IR remote commands 
// Philips SRP9232D/27 Universal Remote
// programmed with code '1376' (Onkyo Audio)
// commented codes are for Xmit
void irReceiveA() 
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
        if (IrReceiver.decodedIRData.command == 0x11) { // airplay, input (1)
          inputUpdate(4); 
        }  
        if (IrReceiver.decodedIRData.command == 0x12) { // toggle volume limiter
          volRange(0);
        }            
    	if (IrReceiver.decodedIRData.command == 0x17) { // optical, input (1)
          inputUpdate(5); 
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
    irScan(); // scan for IR codes
    delay(debounceIR);
    IrReceiver.resume();      
    digitalWrite(LED_BUILTIN, LOW);   
  }
}


// receive IR remote commands 
// FireTV Remote (Seth's Preamp)
void irReceiveB() 
{
  if (IrReceiver.decode()) {
  digitalWrite(LED_BUILTIN, HIGH); 
  // code detected
  if (powerState == 1) {      
    if (IrReceiver.decodedIRData.address == 0x7D02) { // address 
      if (IrReceiver.decodedIRData.command == 0x4D) { // volume down fast (VOL-) 
        volIncrement(1,2);        
      }      
      if (IrReceiver.decodedIRData.command == 0x48) { // volume up fast (VOL+) 
        volIncrement(2,2);
      }   
      if (IrReceiver.decodedIRData.command == 0x4C) { // mute toggle (MUTE) 
        muteSystem(2);
      }  
      if (IrReceiver.decodedIRData.command == 0x16) { // input (1) 
        inputUpdate(1); 
      }      
      if (IrReceiver.decodedIRData.command == 0x5B) { // input (2) 
        inputUpdate(2); 
      }    
      if (IrReceiver.decodedIRData.command == 0x17) { // input (3) 
        inputUpdate(3); 
      }                                                                 
      if (IrReceiver.decodedIRData.command == 0x19) { // volume down slow (DOWN) 
        volIncrement(1,1);
      }      
      if (IrReceiver.decodedIRData.command == 0xC) { // volume up slow (UP) 
        volIncrement(2,1);
      }    
      if (IrReceiver.decodedIRData.command == 0x4A) { // toggle volume limit
        volRange(0);
      }                   
    }          
  }    
  if (IrReceiver.decodedIRData.address == 0x7D02) { // address
    if (IrReceiver.decodedIRData.command == 0x46) { // power toggle (POWER)
      powerState = !powerState;  
      powerCycle = 1;
      delay(100);
    }      
  }         
  irScan(); // scan for IR codes
  delay(debounceIR);
  IrReceiver.resume();      
  digitalWrite(LED_BUILTIN, LOW);   
  }
}


// display IR codes on terminal
void irScan() 
{    
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
}


// setup volume range
void volRange(bool _boot)
{ 
  // mute
  muteSystem(0); 
  // toggle volume limit
  if (_boot == 0) { 
    volLimitFlag = !volLimitFlag; 
  } 
  // loading bar
  lcd.clear();
  lcd.setCursor(0,0);
  if (volLimitFlag == 1){
    lcd.print("Volume Limit On");
  } else {
    lcd.print("Volume Limit Off");
  }
  if (_boot == 1) {
  	delay(500);
    lcd.clear();
    lcd.setCursor(0,0); 	
    lcd.print("Starting Up...");
  }
  lcdTimedBar(startDelay,1);
  lcd.clear();
  // calculate volume min/max
  uint8_t max_byte_size = (1 << volRelayCount) - 1;
  volMin = (max_byte_size * volMinLimit) / 100;
  // volume limiter
  if (volLimitFlag == 0) { 
    volMax = max_byte_size; 
  } else {
    volMax = (max_byte_size * volMaxLimit) / 100;
  }
  // calculate volume span
  volSpan = abs(volMax - volMin);
  // initialize volume system
  muteSystem(1); // unmute
  delay(500); // allow relays to settle
  potState = motorInit; // read from pot
  // set last selected input
  inputUpdate(99);  
}


// power toggle switch
void readToggleSwitch() {
  // read pin state from switch
  int reading = digitalRead(toggleSwitchPin);
  // if switch changed
  if (reading != lastToggleSwitch) {
    // reset the debouncing timer
    toggleSwitchMillis = millis();
  }
  if ((millis() - toggleSwitchMillis) > debounceDelay) {
    // if the button state has changed:
    if (reading != toggleSwitch) {
      toggleSwitch = reading;
      // power state has changed!
      lcd.setCursor(0,0);
      if (toggleSwitch == 1) {
        // runs on boot if switch on, and when toggled on
        hpfControl(0);
      }
      if (toggleSwitch == 0) { 
        // runs when toggled off
        hpfControl(1);
      }
    }
  }
  lastToggleSwitch = reading; 
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


// shutdown routines
void shutdown() {  	
  // mute
  muteSystem(0);
  // shutdown animation
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Shutting Down..."); 	
  lcdTimedBar(shutdownTime,1);	
  lcdStandby();
  // turn off analog stages
  digitalWrite(powerRelayPin, LOW);
  delay(150);
}


// startup routines
void startup() 
{ // set display brightness
  analogWrite(lcdBacklightPin, lcdOnBrightness);
  // turn on analog stages
  digitalWrite(powerRelayPin, HIGH);  
  delay(150); 
  // initialize volume system (I)
  volRange(1);   
}


// display in standby mode
void lcdStandby()
{ 
  analogWrite(lcdBacklightPin, lcdOffBrightness);	
  lcd.clear();
  lcd.setCursor(0,0);
  if (preampType == 0) {
    lcd.print("ProDesigns");
  } else {
    lcd.print("HiFi");
  }
  lcd.setCursor(0,1);
  lcd.print("Preamp v3");
  lcd.setCursor(15,1);
  lcd.print(char(6));
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
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
  if (preampType == 0) {
    // HPF control
    pinMode(hpfRelayPin, OUTPUT);  
    digitalWrite(hpfRelayPin, HIGH);
    // HPF switch 
    pinMode(toggleSwitchPin, INPUT_PULLUP);
  }  
  // IR remote
  IrReceiver.begin(IRpin);
  // icons
  lcd.createChar(0, sound);
  lcd.createChar(6, speaker);
  // initial boot display
  lcd.createChar(1, bar1);
  lcd.createChar(2, bar2);
  lcd.createChar(3, bar3);
  lcd.createChar(4, bar4);
  lcd.createChar(5, bar5);
  lcdTimedBar(initStartDelay,2);
  // serial support
  if (irCodeScan == 1){  
    Serial.begin(9600);
  }
  // startup complete
  lcdStandby();
}


///////////////////////////////////////////////////////////////////////////////
// superloop
void loop()
{
  // power management
  setPowerState(); 
  // IR remote
  if (preampType == 0) {
    irReceiveA();
  } else {
    irReceiveB();
  }  
  if (powerState == 1) {
  	// motor potentiometer
    if (potState == motorSettled ||
        potState == motorInit) {
      potValueChanges();
    }
    motorPot();
    // toggle switch
    if (preampType == 0) {
      readToggleSwitch();
    }
  }
}

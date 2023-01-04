 /***************************************************************************************************
 * HIFIDUINO Code: Supports the latest ES9018K2M DAC chip (revision V.)
 * Follow Arduino hardware hookup guide here:
 * http://hifiduino.wordpress.com/introduction-and-guide-to-hifiduino/
 ************************************** LICENSE *****************************************************
 *                        http://creativecommons.org/licenses/by/3.0/
 *                        Attribution
 *                        CC BY
 *
 ****************************************************************************************************
 * Change log:
 * v. 05  06/29/14:  Code tested with Amanero Interface connected to DIYINHK ES9018K2M DAC with PCM 
                     and DSD files. Added chip identification during start up. Fixed SPDIF selection
                     (SPDIF input supported through GPIO2 line -which is supported by the DAC board).
                     Fixed calculation of sample rate to correctly display DSD frequencies.
                     General display format fixes.
 * V. 04  06/10/14:  Code cleanup and fixes, moved OSF bypass as part of FIR selection,
                     added IIR filter bypass, changed labels for DPLL settings,
 * v. 03  05/08/14:  Added Balance Control. Changes and fixes for DUAL MONO code
 * v. 02  05/05/14:  Bug fixes.
 * v. 01  05/02/14:  Based on code version B1.1f. Supports most features of the DAC.
 ***************************************************************************************************/
 
// LIBRARIES
#include <LiquidCrystal.h>  // For LCD
// Initialize the library with the numbers of the interface pins
// See the LCD section below for more info
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);
#include <Wire.h> // For I2C communication with DAC
// Use eeprom to store values (DAC settings)
#include <EEPROM.h>


/******************* Code Customization Section *********************/

/* First: Choose the clock frequency you have and comment the other */

#define USE80MHZ  
//#define USE100MHZ

/* Second: Choose stereo or mono

   | CONFIGURATION       | #define DUALMONO | #define STEREO   |
   |---------------------|------------------|------------------|
   | Dual mono in-phase  | un-comment       | comment          |
   | Stereo              | comment          | un-comment       | 
   |---------------------|------------------|------------------|    */
   
#define STEREO
//#define DUALMONO

/* Third, optionally choose the number of inputs. 6 is the max without 
   modifying the code. You could lower the number of input choices
   here. for example if you only want to see 2 choices, modify the
   code like this: #define ICHO 2                                   */

#define ICHO 2

/* Fourth, optionally change the name of the inputs. Keep 6 characters
   Use blanks if necessary. If you use less number of inputs, only the
   top ones matter.
   */

char no0[] = "INPT-A";
char no1[] = "INPT-B";
char no2[] = "INPT-C";
char no3[] = "INPT-D";
char no4[] = "INPT-E";
char no5[] = "INPT-F";

/* These inputs choices can be virtual or real. In the ES9018 there
   were 8 data lines. One could simultanously connect one I2S/DSD input
   plus 3 additional SPDIF input (thus 4 physical inputs). In addition
   one could choose different parameters -such as the DPLL bandwidh or
   filter selection- when playing I2S or when playing DSD(thus 2 virtual
   inputs). With the ES9018K2M there are fewer data lines so there is
   no need for so many input selections. But the flexibility remains
                                                                    */
   
/* Fifth, adjust the interrupt routine to match your rotary encoder by
   adjusting the mode parameter in the following routine
   (search for it in the code):
   
   "attachInterrupt(0, rotEncoder, LOW);"
    
   The mode parameter defines when the interrupt should be triggered:
       LOW to trigger the interrupt whenever the pin is low,
       CHANGE to trigger the interrupt whenever the pin changes value
       RISING to trigger when the pin goes from low to high,
       FALLING for when the pin goes from high to low. 
   
   You can also read the following link:
   http://hifiduino.wordpress.com/2011/09/12/problems-with-rotary-encoders/
                                                                    */

/***************** End Code Customization Section *******************/   

/************************** CHIP ADDRESS ****************************/

/*
   for stereo: the address is specified as 0x90
   for dual mono: Specified as 0x90 for mono left channel DAC 
                  Specified as 0x92 for mono right channle DAC
   These chip addresses are the same (at least) for:
       ES9018
       ES9018K2M 
   
   Since Arduino uses 7-bit address, and the 8th R/W bit is added
   automatically depending on whether you use the write call
   [beginTransmission()] or the read call [requestFrom()].
   Therefore, use the 7 most significant bits of the 8-bit address.
   
   To convert the addresses to 7-bit remove the least significant bit
   0x90: 10010000 (we eliminate the rightmost bit to get I2C address)
   0x48: 1001000
   0x92: 10010010 (we eliminate the rightmost bit to get I2C address)
   0x49: 1001001
   
   Thus when writing code:
       Use Address 0x48 for Stereo or mono left
       Use Address 0x49 for mono right                                  */


#define DEFAULTATTNU 0x64 //-50 dB this is 50x2=100 or 0x64. Sabre32 is 0 to -127dB in .5dB steps
#define MAXATTNU 0xC6     //-99dB this is 99X2=198 or 0xC6 -Can go higher but LCD shows 2 digits
#define MINATTNU 0x00     //-0 dB -Maximum volume is -0 dB
#define DIM 0x8C          //-70 dB this is 70x2=140 or 0x8C. Dim volume a kind of soft mute
#define RAMP 10           // Additional msec delay per 1 db for ramping up to volume after dim

#define ROTPINA 4         // Button to increase volume or RotEnc A terminal
#define ROTPINB 2         // Button to decrease volume or RotEnc B terminal
#define SELECTPIN 5       // Switch to select function
// #define BRIPIN 6         // Pin to control the LCD brightness with analogWrite
#define REMOTEPIN 3       // Pin for IR receiver (remote control)

#define INTERVAL_SAMPLE 2          // Time interval in SECONDS for refreshing the sample rate
#define INTERVAL_BOUNCE 2          // Time in milliseconds to debounce the rotary encoder
#define INTERVAL_SWITCHBOUNCE 250  // Time in milliseconds to debounce switch
#define INTERVAL_SELECT 5          // Time in sec to exit select mode when no activity

#define B 0xFF  // The character for a completely filled box (NOT the character "B")
#define A 0x20  // The character for blank (NOT the character "A")

// The following is used for parameter selection and parameter storage
// This is the order in which they are are selected when you press the selection switch

#define VOL 0  // Volume
#define INP 1  // Input selection
#define INF 2  // Input format selection
#define FIR 3  // FIR filter selection
#define IIR 4  // IIR filter selection
#define LLI 5  // DPLL bandwidth for I2S selection
#define LLD 6  // DPLL bandwidth for DSD selection
#define DEE 7  // De-emphasis selection
#define BAL 8  // Balance setting
#define SRF 9  // Sample Rate display format: "nominal" or "exact"

// Order of settings in the array for each input (for those saved in array)

#define FORMATVAL 0
#define FIRVAL 1
#define IIRVAL 2
#define I2SDPLLVAL 3
#define DSDDPLLVAL 4
#define DEEMVAL 5
#define BALVAL 6

/*
   MAXPARAM is the total number of parameters to keep track for each input.
   The 7 listed above plus the current input choice which is remembered. This makes 8.
   (OSF bypass is now part of FIR filter selection).
   VOL is also not tracked per input as it always starts with the default volume.
   SRF is also not tracked per input as it applies to all inputs.
   
   There are a total of 10 parameters that can be adjusted: 8 parameters are per input selection
   and can be saved in EEPROM, 2 parameters apply accross all input selections AND always start
   in their default value upon power-on.
*/
#define MAXPARAM 8 

/*
   Number of valid choices for each parameter. This is used as the modulo of the value stored
   in the eeprom. We need this because the value stored in the eeprom could be a multiple of the
   the selection choice
*/

#define FORMATCHO 5   // SPDIF, I2S/DSD, etc.
#define FIRCHO 4      // Fast, slow, minimum, bypass
#define IIRCHO 5      // 47k, 50k, 60k, 70k, bypass
#define I2SDPLLCHO 16 // Lowest to Highest, OFF: 16 values
#define DSDDPLLCHO 16 // Lowest to Highest, OFF: 16 values
#define DEEMCHO 6     // Off, auto, 32k, 44k and 48k and "reserved"
#define BALCHO 39     // 39 possible values for balance settings
// There is one more: ICHO which is already defined above in the customization section.
// Choices SRF are two, so we just use a toggle variable and not remembered.
// Vol selection is seprate and not remembered.

// VARIABLE DECLARATION

// Register variables: used for registers that have different functions. They are initialized here
// with valid values, but some of them will eventually be overwritten by the values in EEPROM.
// Values for registers that have a single function/option are not kept in a variable and are
// used directly in the subroutines.

byte reg0=0x00;  // System settings. Default value of register 0
byte reg4=0x00;  // Automute time. Default = disabled
byte reg5=0x68;  // Automute level. Default is some level, but in reg4 default has automute disabled
byte reg7=0x80;  // General settings. Default value fast fir, pcm iir and unmuted
byte reg8=0x81;  // GPIO configuration. GPIO1 set to DPLL Lock; GPIO2 set to input (for SPDIF)
byte reg10=0x05; // Master Mode Control. Default value: master mode off
byte reg12=0x5A; // DPLL Settings. Default= one level above lowest for I2S and two levels above 
                 // mid setting for DSD
byte reg14=0x8A; // Soft Start Settings
byte reg21=0x00; // Oversampling filter setting and GPIO settings. Default: oversampling ON
// reg11 will need a R and V variable in order to support MONO
byte reg11S=0x02; // Channel Mapping. "S" means stereo 
                  // Default stereo is Ch1=left, Ch2=right
byte reg11L=0x00; // Default value for MONO LEFT
byte reg11R=0x03; // Defuult value for MONO RIGHT
byte reg66=0;
byte reg67=0;
byte reg68=0;
byte reg69=0;

unsigned long displayMillis = 0;   // Stores last recorded time for display interval
unsigned long debounceMillis = 0;  // Stores last recorded time for switch debounce interval
unsigned long selectMillis = 0;    // Stores last recorded time for being in select mode
unsigned long sr = 0;              // Holds the value for the incoming sample rate

byte input=0;                 // The current input to the DAC
byte currAttnu=DEFAULTATTNU;  // Variable to hold the current attenuation value

byte select;             // To record current select position (FIL, VOL, DPLL, etc)
boolean selectMode;      // To indicate whether in select mode or not 
boolean SRExact=true;    // Display exact sample rate value; false = display nominal value
boolean dimmed=false;    // To indicate dim (mute) or not
byte pulse=0;            // Used by the "heartbeat" display
byte chipStatus=0;       // To hold the value of chip status register

byte lBal=0;
byte rBal=0;
  
// The following variables for the remote control feature
int duration;         // Duration of the IR pulse
int mask;             // Used to decode the remote signals 
int c1;               // Byte 1 of the 32-bit remote command code
int c2;               // Byte 2 of the 32-bit remote command code
int c3;               // Byte 3 of the 32-bit remote command code
int c4;               // Byte 4 of the 32-bit remote command code
int IRkey;            // The unique code (Byte 3) of the remote key
int previousIRkey;    // The previous code (used for repeat)

/*
 The array that holds the parameters for each input. The size of the array is the number of inputs
 multiplied by the number of parameters (each input definition remembers its own parameter settings).
 The current input selection is recorded after each change into the array in the eeprom)
*/
byte settings[ICHO][MAXPARAM];  // Array to hold parameter values

// Function Declaration

// Routines for reading and writing from and to the EEPROM
void writeSettings(){
  for(byte i=0;i<ICHO;i++) {
    for (byte j=0;j<MAXPARAM;j++) {
      if(EEPROM.read((i*MAXPARAM)+j)!=settings[i][j]) // Check an see if there are any changes
        EEPROM.write((i*MAXPARAM)+j,settings[i][j]);  // Write the changes in eeprom
    }
  } 
  EEPROM.write(ICHO*MAXPARAM,input); // Write the current input in a variable location after the array
  EEPROM.write(ICHO*MAXPARAM+1,SRExact); // Write the value of SRExact
}

void readSettings(){
  for(byte i=0;i<ICHO;i++) {
    for (byte j=0;j<MAXPARAM;j++) {
      settings[i][j]=EEPROM.read((i*MAXPARAM)+j);
    }
  }
  input=EEPROM.read(ICHO*MAXPARAM);      // Read the last saved input setting
  SRExact=EEPROM.read(ICHO*MAXPARAM+1);  // Read the last saved setting for SRExact
}
  
/*
LCD
 
 For the LCD, I am using a "standard" HD44780 20x4 display, and I am using the official Arduino
 LiquidCrystal library that comes with the standard installation. This is as standard as it can be
 and this type of LCD is available everywhere.
 
 Note: you can improved the performance of driving the LCD by using a different library, check
 http://hifiduino.wordpress.com/2012/02/05/new-liquidcrystal-arduino-library/
 
 The pin assignment is as follows:
 
 * LCD RS pin to digital pin 12
 * LCD Enable (E) pin to digital pin 11
 * LCD D4 (DB4) pin to digital pin 10
 * LCD D5 (DB5) pin to digital pin 9
 * LCD D6 (DB6) pin to digital pin 8
 * LCD D7 (DB7) pin to digital pin 7
 
 You can also check:
 http://hifiduino.wordpress.com/2010/10/13/arduino-code-for-buffalo-ii-dac-lcd-connection/
 */

/*
ROTARY ENCODER
 
 The rotary encoder is connected to digital pin 4 (A) and digital pin 2 (B). pin 2 (and also pin 3)
 is an interrupt line in Arduino. It does not matter which terminal in the rotary encoder is
 connected to which pin. The third terminal of the rotary encoder is connected to GND. At each cycle,
 the rotary encoder will pull the pins LOW or HIGH and the transition is detected by the interrupt
 line. The interrupt service routine is specified below.
 More info:
 http://hifiduino.wordpress.com/2010/10/21/arduino-code-for-buffalo-ii-dac-rotary-encoder-connections/
 
 DEBOUNCE
 In this version, the code implements debouncing by adding a few millisecond delay that pauses 
 the code from evaluating the state of the pins. Typically, all the switching noise –the “bouncing” 
 is generated in the first few milliseconds after the switch event. The code is optimized by calling 
 the delay only if there is any activity in the rotary encoder. Activity in the rotary encoder is 
 detected with an interrupt. The interrupt service routine is minimalist in design and determines if 
 there is any motion in the rotary encoder.
 
 In addition, in order to minimize spurious interrupts –those generated by the switching noise, 
 hardware debounce can be optionally implemented in the rotary encoder. Although the code will ignore
 all the interrupts generated during the debounce delay time, interrupts are still generated and it
 is still a good idea to minimize those spurious interrupts. There is just a slight change in the
 software if H/W debouncing is implemented, namely the pull up resistors in the pins are to be
 disabled.
 
 Further, the rotary encoder has an on-off switch, and the debouncing of the switch is also done in
 s/w (again the same h/w debouncing can be implemented, but it is optional). Switch detection is not
 based on interrupts and due to the fact that pushing the switch manually will generate not only
 noise during the switching, but also the switch can remain pressed for 100 milliseconds or more
 because one cannot lift the finger that fast, the debounce time needs to be longer.In this
 implementation and with my way of pushing down the switch, 200 msec is an appropriate value.
 */
 
/*
INTERRUPT SERVICE ROUTINE FOR ROTARY ENCODER
 
 The interrupt service routine has been designed to be minimalist in nature. The code just sets a 
 flag indicating that some activity has been detected in the rotary encoder.
 
 4/28/14: I made the change from static to volatile as recommended by a reader and this post:
 http://www.gammon.com.au/forum/?id=11488
 */
 
volatile boolean rotating=false;
void rotEncoder()
{
  rotating=true; // If motion is detected in the rotary encoder, set the flag to true
}

/*
READING THE SAMPLE RATE
 
 The sample rate can be calculated by reading the DPLL 32-bit register. The datasheet gives the
 formula: Sample Rate = (DPLL-number*Clock-Frequency)/2^32. (unlike the older ESS DAC, the ES9018, 
 the Sample Rate calculation does not depend whether the input signal is SPDIF or I2S)
 
 The Crystal frequency can be 80MHz 0r 100MHz In Arduino (and other small microprocessors) it is NOT
 advisable to do floating point math because "it is very slow"; therefore integer math will be used
 to calculate the sample rate. Note: I don't know what is the impact of this "slow operation" but
 this is the way I implemented this since the beginning and find no reason to change it.
 
 The value of 2^32/80,000,000 is 53.687091 (which is a floating point number). If we use the 
 integer part (53 or 54) we get the following results for a 44.1K sample rate signal:  divided by 53 
 the result is 44.677K; divided by 54, the result is 43.849K. Clearly there are large errors from 
 being confined to integer math. The actual result, if we use floating point math and use all the 
 significant digits is 44,105 Hz (the 5 Hz deviation from ideal 44100 Hz is within the specification
 of SPDIF and the tolerances of the crystals and clocks involved)
 
 In order to increase the accuracy of the integer calculation, we can use more of the significant 
 digits of the divisor. I did some evaluation of the DPLL register values for sample rates ranging 
 from 44.1K to 192K and noticed that I could multiply the value of the DPLL number by up to 
 400 without overflowing the 32-bits. Therefore, since we have 32 bit number to work with, we 
 can multiply the DPLL number by  up to 400 and then divide by 400X53.687091=21475. If we do this, 
 we obtain 44.105K which is the same as the exact value.
 
 I used a spreadsheet to calculate the multipliers for both 80Mhz and 100Mhz
 
 80Mhz:  x80, %4295
 100Mhz: x20, %859
 */

// Sample rate reading routines

volatile unsigned long DPLLNum; // Variable to hold DPLL value

byte readRegister(byte regAddr) {
  Wire.beginTransmission(0x48); // Hard coded the Sabre device address
  Wire.write(regAddr);          // Queues the address of the register
  Wire.endTransmission();       // Sends the address of the register
  Wire.requestFrom(0x48,1);     // Hard coded to device, request one byte from address
                                // specified with Wire.write()/wire.endTransmission()
  //while(!Wire.available()) {  // Wait for byte to be available on the bus
  if(Wire.available())          // Wire.available indicates if data is available
    return Wire.read();         // Wire.read() reads the data on the wire
  else
    return 0;                   // In no data in the wire, then return 0 to indicate error
                                // Zero is valid error because the ID register for K2M is not 0
}

unsigned long sampleRate() {
  DPLLNum=0;
  // Reading the 4 registers of DPLL one byte at a time starting with LSB (reg 66)
  reg66=readRegister(66);
  reg67=readRegister(67);
  reg68=readRegister(68);
  reg69=readRegister(69);
  
  DPLLNum|=reg69;
  DPLLNum<<=8;
  DPLLNum|=reg68;
  DPLLNum<<=8;
  DPLLNum|=reg67;
  DPLLNum<<=8;
  DPLLNum|=reg66;
  DPLLNum>>=1;       // Get rid of LSB to allow for integer operation below to avoid overflow  
  
  #ifdef USE80MHZ
    DPLLNum*=16;     // Calculate SR for 80MHz part
    DPLLNum/=859;    // Calculate SR for 80MHz part
    DPLLNum+=1;
    DPLLNum*=2 ;
  #endif
  #ifdef USE100MHZ
    DPLLNum*=20;     // Calculate SR for 100MHz part
    DPLLNum/=859;    // Calculate SR for 100MHz part
    DPLLNum*=2
  #endif
 
 /*
  if (DPLLNum < 90000) // Adjusting because in integer operation, the residual is truncated
                       // This is just cosmetic...
    DPLLNum+=1;
    else
      if (DPLLNum < 190000)
        DPLLNum+=2;
      else
        if (DPLLNum < 350000)
          DPLLNum+=3;
        else
          DPLLNum+=4;
  */  
  /*
  if(bypassOSF)      // When OSF is bypassed, the magnitude of DPLL is reduced by a factor of 64
                     // This is no longer needed for the K2M. It reports the sample rate correctly
                     // whether it is in NOS or nonNOS
    DPLLNum*=64;
  */
      
  return DPLLNum;
}

// CONTROLLING THE DIGITAL ATTENUATION (VOLUME) -and other registers IN THE DAC
// Chip addresses are hardcoded. The default mode for the address 0x48 is for be the left chip

void writeSabreLeftReg(byte regAddr, byte regVal)
{
  Wire.beginTransmission(0x48); // Hard coded to the the DAC device address for stereo
                                // or mono left. For stereo same as writeSabreReg()
  Wire.write(regAddr);          // Specifying the address of register
  Wire.write(regVal);           // Writing the value into the register
  Wire.endTransmission();
}

// In dual mono, sometimes different values are written to L and R chips so here is the routine
// to write to registers for the "right" DAC.

#ifdef DUALMONO
void writeSabreRightReg(byte regAddr, byte regVal)
{
  Wire.beginTransmission(0x49); //Hard coded to the the ESS DAC device address
  Wire.write(regAddr); // Specifying the address of register
  Wire.write(regVal); // Writing the value into the register
  Wire.endTransmission();
}
#endif

// The following routine writes to both chips in dual mono mode. With some exceptions, one only needs
// to set one of the chips to be the right channel after all registers are modified.

void writeSabreReg(byte regAddr, byte regVal)
{
  // By default the chip with addres 0x48 is the left channel
  writeSabreLeftReg(regAddr, regVal);
  
  #ifdef DUALMONO // If dual mono write also to the other chip. 
  writeSabreRightReg(regAddr, regVal);
  
  // Set the left channel chip (addr 0x48) to be the left channel and the right chip (addr 0x49)
  // to be the right channel after modifying both L and R with same values. 
  if (regAddr==0x0B){  // Reset the channel mapping register if writing to this register
    reg11L=reg11S;
    reg11R=reg11S;
    bitClear(reg11L,0);
    bitClear(reg11L,1);
    bitSet(reg11R,0);
    bitSet(reg11R,1);
    writeSabreLeftReg(0x0B,reg11S);
    writeSabreRightReg(0x0B,reg11R);
  }
  #endif
}

void setSabreVolume(byte regVal)
{
 // lBal and rBal are for adjusting for Balance for left and right channels
 #ifdef STEREO 
  writeSabreLeftReg(15, regVal+lBal); // set up volume in Channel 1 (Left)
  writeSabreLeftReg(16, regVal+rBal); // set up volume in Channel 2 (Right)
 #endif

 // For DUALMONO, the lBal is for both channel in chip 0x48 and rBal is for both chanenls in chip 0x49
 #ifdef DUALMONO 
  writeSabreLeftReg(15, regVal+lBal); // set up volume in Channel 1 (Left dualmono)
  writeSabreLeftReg(16, regVal+lBal); // set up volume in Channel 2 (Left dualmono)
  writeSabreRightReg(15, regVal+rBal); // set up volume in Channel 1 (Right dualmono)
  writeSabreRightReg(16, regVal+rBal); // set up volume in Channel 2 (Right dualmono)
 #endif
}

void rampUp()
{
  byte i=(DIM-currAttnu); 
  for(byte dimval=DIM;dimval>currAttnu;dimval--){
    setSabreVolume(dimval);
    printTwoNumber(13,dimval/2);
    delay((RAMP)*(1+(10/i*i)));
    i--;
  }
}

/*
 The following function prints a bar at the left most column to indicate that we are in "select"
 mode. One can choose any character as the "bar" For example, I used the solid square character
 */
void printSelectBar(byte value){
  lcd.setCursor(0,0);
  lcd.write(value);
  lcd.setCursor(0,1);
  lcd.write(value);
  lcd.setCursor(0,2);
  lcd.write(value);
  lcd.setCursor(0,3);
  lcd.write(value);
  lcd.setCursor(7,0);
  lcd.write(value);
  lcd.setCursor(7,1);
  lcd.write(value);
  lcd.setCursor(7,2);
  lcd.write(value);
  lcd.setCursor(7,3);
  lcd.write(value);
  lcd.setCursor(12,0);
  lcd.write(value);
}

void setAndPrintI2sDPLL(byte value){ // Set the DPLL Mode for I2S
  lcd.setCursor(1,2);
  lcd.print("DL");
  lcd.write(0xA5);              // This is the dot (.) character
  reg12=reg12 & 0b00001111;     // Clear the top nibble (upper 4 bits)
  switch(value){                // Here we set the DPLL values for I2S (upper 4 bits)
  case 0:
    reg12=reg12 | 0x00;  
    writeSabreReg(0x0C,reg12);
    lcd.print("OFF");
    break;
  case 1:
    reg12=reg12 | 0x10; 
    writeSabreReg(0x0C,reg12);
    lcd.print("LST");
    break;
  case 2:
    reg12=reg12 | 0x20; 
    writeSabreReg(0x0C,reg12);
    lcd.print("002");
    break;
  case 3:
    reg12=reg12 | 0x30; 
    writeSabreReg(0x0C,reg12);
    lcd.print("003");
    break;
  case 4:
    reg12=reg12 | 0x40; 
    writeSabreReg(0x0C,reg12);
    lcd.print("004");
    break;
  case 5: // Default setting
    reg12=reg12 | 0x50; 
    writeSabreReg(0x0C,reg12);
    lcd.print("DEF");  // Default setting
    break;
  case 6:
    reg12=reg12 | 0x60; 
    writeSabreReg(0x0C,reg12);
    lcd.print("006");
    break;
  case 7:
    reg12=reg12 | 0x70; 
    writeSabreReg(0x0C,reg12);
    lcd.print("007");
    break;
  case 8:
    reg12=reg12 | 0x80; 
    writeSabreReg(0x0C,reg12);
    lcd.print("008");
    break;
  case 9:
    reg12=reg12 | 0x90; 
    writeSabreReg(0x0C,reg12);
    lcd.print("009");
    break;
  case 10:
    reg12=reg12 | 0xA0; 
    writeSabreReg(0x0C,reg12);
    lcd.print("010");
    break;
  case 11:
    reg12=reg12 | 0xB0; 
    writeSabreReg(0x0C,reg12);
    lcd.print("011");
    break;   
  case 12:
    reg12=reg12 | 0xC0; 
    writeSabreReg(0x0C,reg12);
    lcd.print("012");
    break; 
  case 13:
    reg12=reg12 | 0xD0; 
    writeSabreReg(0x0C,reg12);
    lcd.print("013");
    break; 
  case 14:
    reg12=reg12 | 0xE0; 
    writeSabreReg(0x0C,reg12);
    lcd.print("014");
    break; 
  case 15:
    reg12=reg12 | 0xF0; 
    writeSabreReg(0x0C,reg12);
    lcd.print("HST");
    break;
  }
}

void setAndPrintDsdDPLL(byte value){ // Set the DPLL Mode for DSD -lower 4 bits
  lcd.setCursor(8,2);
  reg12=reg12 & 0b11110000;     // Clear the bottom nibble (lower 4 bits)
  switch(value){                // Here we set the DPLL values for DSD (lower 4 bits)
  case 0:
    reg12=reg12 | 0x00;  
    writeSabreReg(0x0C,reg12);
    lcd.print("OFF");
    break;
  case 1:
    reg12=reg12 | 0x01; 
    writeSabreReg(0x0C,reg12);
    lcd.print("LST");
    break;
  case 2:
    reg12=reg12 | 0x02; 
    writeSabreReg(0x0C,reg12);
    lcd.print("002");
    break;
  case 3:
    reg12=reg12 | 0x03; 
    writeSabreReg(0x0C,reg12);
    lcd.print("003");
    break;
  case 4:
    reg12=reg12 | 0x04; 
    writeSabreReg(0x0C,reg12);
    lcd.print("004");
    break;
  case 5:
    reg12=reg12 | 0x05; 
    writeSabreReg(0x0C,reg12);
    lcd.print("005");
    break;
  case 6:
    reg12=reg12 | 0x06; 
    writeSabreReg(0x0C,reg12);
    lcd.print("006");
    break;
  case 7:
    reg12=reg12 | 0x07; 
    writeSabreReg(0x0C,reg12);
    lcd.print("007");
    break;
  case 8:
    reg12=reg12 | 0x08; 
    writeSabreReg(0x0C,reg12);
    lcd.print("008");
    break;
  case 9:
    reg12=reg12 | 0x09; 
    writeSabreReg(0x0C,reg12);
    lcd.print("009");
    break;
  case 10: // Default Setting
    reg12=reg12 | 0x0A; 
    writeSabreReg(0x0C,reg12);
    lcd.print("DEF");
    break;
  case 11:
    reg12=reg12 | 0x0B; 
    writeSabreReg(0x0C,reg12);
    lcd.print("011");
    break;   
  case 12:
    reg12=reg12 | 0x0C; 
    writeSabreReg(0x0C,reg12);
    lcd.print("012");
    break; 
  case 13:
    reg12=reg12 | 0x0D; 
    writeSabreReg(0x0C,reg12);
    lcd.print("013");
    break; 
  case 14:
    reg12=reg12 | 0x0E; 
    writeSabreReg(0x0C,reg12);
    lcd.print("014");
    break; 
  case 15:
    reg12=reg12 | 0x0F; 
    writeSabreReg(0x0C,reg12);
    lcd.print("HST");
    break;
  }
}

void setAndPrintSRFormat(){          // This is a toggle function for selecting SR display format
  if(SRExact==true){                 // Currently set to display exact sample rate
    SRExact=false;                   // Set to Nominal
    lcd.setCursor(13,0);
    lcd.print("NOMINAL");            // Indicate NOMINAL mode
  }
  else {
    SRExact=true;                    // Set to display exact sample rate
    lcd.setCursor(13,0);
    lcd.print(" EXACT ");            // Indicate EXACT mode
  }
}

void setAndPrintFirFilter(byte value){
  lcd.setCursor(1,1);
  lcd.print("Fi");
  lcd.write(0xA5);              // dot character (.)
  lcd.setCursor(4,1);
  switch(value){
  case 0:                       // Slow FIR
    bitSet(reg7,5);             // x 0 1 x x x x x 
    bitClear(reg7,6);           // x 0 1 x x x x x 
    bitClear(reg21,0);          // Use OSF: x x x x x x x 0
    writeSabreReg(0x0E,reg7);
    writeSabreReg(0x15,reg21);  // Reg21
    lcd.print("SLW");
    break;  
  case 1:                       // Fast FIR (Sharp) -Default
    bitClear(reg7,5);           // x 0 0 x x x x x 
    bitClear(reg7,6);           // x 0 0 x x x x x
    bitClear(reg21,0);          // Use OSF: x x x x x x x 0
    writeSabreReg(0x0E,reg7);
    writeSabreReg(0x15,reg21);  // Reg21
    lcd.print("SHR");
    break;
  case 2:                       // Minimum phase filter (Sharp)
    bitClear(reg7,5);           // x 1 0 x x x x x          
    bitSet(reg7,6);             // x 1 0 x x x x x 
    bitClear(reg21,0);          // Use OSF: x x x x x x x 0
    writeSabreReg(0x0E,reg7);
    writeSabreReg(0x15,reg21);  // Reg21
    lcd.print("MIN");
    break;
  case 3:                       // Bypass oversampling filter
    bitSet(reg21,0);            // Bypass OSF: x x x x x x x 1
    writeSabreReg(0x15,reg21);  // Reg21
    lcd.print("NOS");
    break;
  }
}

void setAndPrintIirFilter(byte value){
  lcd.setCursor(8,1);
  switch(value){
  case 0:                        // IIR Bandwidth: Normal 47K (for PCM)
    bitClear(reg7,2);            // x x x x 0 0 x x 
    bitClear(reg7,3);
    bitClear(reg21,2);           // Use IIR: x x x x x 0 x x    
    writeSabreReg(0x0E,reg7);
    writeSabreReg(0x15,reg21);   // Reg21
    lcd.print("PCM");
    break;
  case 1:                        // IIR Bandwidth: 50k (for DSD) (D)
    bitSet(reg7,2);              // x x x x 0 1 x x 
    bitClear(reg7,3);
    bitClear(reg21,2);           // Use IIR: x x x x x 0 x x     
    writeSabreReg(0x0E,reg7);
    writeSabreReg(0x15,reg21);   // Reg21
    lcd.print("50K");
    break;
  case 2:                        // IIR Bandwidth: 60k (for DSD)
    bitSet(reg7,3);              // x x x x 1 0 x x 
    bitClear(reg7,2);
    bitClear(reg21,2);           // Use IIR: x x x x x 0 x x     
    writeSabreReg(0x0E,reg7);
    writeSabreReg(0x15,reg21);   // Reg21
    lcd.print("60K");
    break;
  case 3:                        // IIR Bandwidth: 70k (for DSD)
    bitSet(reg7,2);              // x x x x 1 1 x x 
    bitSet(reg7,3);
    bitClear(reg21,2);           // Use IIR: x x x x x 0 x x     
    writeSabreReg(0x0E,reg7);
    writeSabreReg(0x15,reg21);   // Reg21
    lcd.print("70K");
    break;
  case 4:                        // IIR OFF
    bitSet(reg21,2);             // Bypass IIR: x x x x x 1 x x
    writeSabreReg(0x15,reg21);   // Reg21
    lcd.print("OFF");
    break;
  }
}

void setAndPrintDeemphasis(byte value){ // register 6
  lcd.setCursor(1,3);
  lcd.print("De");
  lcd.write(0xA5);               // dot character (.)

  switch(value){
  case 0:                        // OFF: 0 1 0 0 1 0 1 0 = 0x4A
    writeSabreReg(0x06,0x4A);
    lcd.print("OFF");
    break;
  case 1:                        // AUTO: 1 0 0 0 1 0 1 0 = 0x8A
    writeSabreReg(0x06,0x8A);
    lcd.print("AUT");
    break;
  case 2:                        // MANUAL 32K: 0 0 0 0 1 0 1 0 = 0x0A
    writeSabreReg(0x06,0x0A);
    lcd.print("32K");
    break;
  case 3:                        // MANUAL 44K: 0 0 0 1 1 0 1 0 = 0x1A
    writeSabreReg(0x06,0x1A);
    lcd.print("44K");
    break; 
  case 4:                        // MANUAL 48K: 0 0 1 0 1 0 1 0 = 0x2A
    writeSabreReg(0x06,0x2A);
    lcd.print("48K");
    break;
  case 5:                        // MANUAL RESERVED: 0 0 1 1 1 0 1 0 = 0x3A (for fun)
    writeSabreReg(0x06,0x3A);
    lcd.print("RES");
    break;
  }
}
/*
  Adjusting Balance. The balance can be adjusted up to 9.5 dB to the right channel or to the left
  channel. The limit of 9.5 dB is just so that the value fits in the display. In theory you can
  completely turn-off one or the other channel. The way it works is to increase the attenuation of
  one channel or the other channel. If the Balance is to the right channel (turning the knob
  clockwise), then the display will indicate how many dBs is the left channel attenuated - or how
  much louder is the right channel compared with the left channel
*/

void setAndPrintBalance(byte value){
  lcd.setCursor(8,3);
  if (value==19){  // Mid point
    lBal=0;
    rBal=0;
    lcd.print("BAL ");
  }
  else{
    if (value>19){                   // Adjusting balance to right channel
      rBal=0;                        // No additional attenuation for right channel
      lBal=value-19;                 // Attenuate left channel
      lcd.print(lBal/2);             // Print whole dB value
      if(lBal%2) lcd.print(".5");    // Print fraction dB value
      else lcd.print(".0");
      lcd.print("R");
    }
    
    else{                            // Adjusting balance to left channel
      lBal=0;                        // No additional attenuation for left channel
      rBal=19-value;                 // Attenuate right channel
      lcd.print(rBal/2);             // Print whole dB value
      if(rBal%2) lcd.print(".5");    // Print fraction dB value
      else lcd.print(".0");
      lcd.print("L");
    }
  }
  
// Adjust volume based on the current balance settings
  setSabreVolume(currAttnu); 
} 

void setAndPrintInputFormat(byte value){
  lcd.setCursor(8,0);
  muteDacs();                       // Mute DACs before switching inputs to avoid potential noise
  
  // Note: SPDIF default settings uses DATA_CLK line for data source
  switch(value){
  case 0:                           // Auto detect which supports "32bit I2S, DSD or SPDIF"
                                    // This is default conf. SPDIF can be connected to DATA_CLK line
    bitClear(reg11S,4);             // Default setting for SPDIF
    bitClear(reg11S,5);             // Default setting for SPDIF
    bitClear(reg11S,6);             // Default setting for SPDIF
    writeSabreReg(0x0B,reg11S);     // Reg 11: x 0 0 0 x x x x
    writeSabreReg(0x01,0x8C);       // Set to auto mode: 1 0 0 0 1 1 0 0
    lcd.print("AUT");
    break;
  case 1:                           // Auto detect between I2S and DSD but not SPDIF
                                    // This case is useful for supporting interfaces that support
                                    // both I2S and DSD (throught the same lines)
    bitClear(reg11S,4);             // Default setting for SPDIF
    bitClear(reg11S,5);             // Default setting for SPDIF
    bitClear(reg11S,6);             // Default setting for SPDIF
    writeSabreReg(0x0B,reg11S);     // Reg 11: x 0 0 0 x x x x
    writeSabreReg(0x01,0x84);       // Set to manual input i2s and DSD: 1 0 0 0 0 1 0 0
    lcd.print("I/D");
    break;
  case 2:                           // Manual SPDIF Input 2 (GPIO2 line instead of DATA_CLK)
    reg8&=B00001111;                // Reprogram GPIO2 assignment just in case. Clear upper bits
    bitSet(reg8,7);                 // Set bit 7: GPIO2 is set as input
    writeSabreReg(0x08,reg8);
    bitClear(reg11S,4);
    bitClear(reg11S,5);
    bitSet(reg11S,6);
    writeSabreReg(0x0B,reg11S);     // Select GPIO2 line for SPDIF input: x 1 0 0 x x x x
    writeSabreReg(0x01,0x81);       // Set to manual input and set to spdif: 1 0 0 0 0 0 0 1
    lcd.print("SPd");
    break;
  case 3:                           // Manual I2S input with data length=16 bit
    bitClear(reg11S,4);             // Default setting for SPDIF
    bitClear(reg11S,5);             // Default setting for SPDIF
    bitClear(reg11S,6);             // Default setting for SPDIF
    writeSabreReg(0x0B,reg11S);     // Reg 11: x 0 0 0 x x x x
    writeSabreReg(0x01,0x00);       // Set to manual input and set to I2S, 16 bit: 0 0 0 0 0 0 0 0
    lcd.print("I16");
    break;
  case 4:                           // Manual LJ input with data length=16 bit
    bitClear(reg11S,4);             // Default setting for SPDIF
    bitClear(reg11S,5);             // Default setting for SPDIF
    bitClear(reg11S,6);             // Default setting for SPDIF
    writeSabreReg(0x0B,reg11S);     // Reg 11: x 0 0 0 x x x x
    writeSabreReg(0x01,0x10);       // Set to manual input and set to LJ, 16 bit: 0 0 0 1 0 0 0 0 
    lcd.print("L16");
    break;
 /*
 case 5:                           // Manual SPDIF Input 1 (GPIO1 line -instead fo DATA_CLK)
    bitSet(reg11S,4);
    bitSet(reg11S,5);               
    bitClear(reg11S,6);               
    writeSabreReg(0x0B,reg11S);     // Select GPIO1 line for SPDIF input: x 0 1 1 x x x x
    writeSabreReg(0x01,0x81);       // Set to manual input and set to spdif: 1 0 0 0 0 0 0 1                            
    lcd.print("SP1");
    break;
  case 6:                           // Manual I2S Input (already covered in case 1. Useful for debug)
    bitClear(reg11S,4);             // Default setting for SPDIF
    bitClear(reg11S,5);             // Default setting for SPDIF
    bitClear(reg11S,6);             // Default setting for SPDIF
    writeSabreReg(0x0B,reg11S);     // Reg 11: x 0 0 0 x x x x
    writeSabreReg(0x01,0x80);       // Set to manual input and set to i2s: 1 0 0 0 0 0 0 0
    lcd.print("I2S");
    break;
  case 7:                           // Manual DSD Input (already covered in case 1. Useful for debug)
    bitClear(reg11S,4);             // Default setting for SPDIF
    bitClear(reg11S,5);             // Default setting for SPDIF
    bitClear(reg11S,6);             // Default setting for SPDIF
    writeSabreReg(0x0B,reg11S);     // Reg 11: x 0 0 0 x x x x
    writeSabreReg(0x01,0x83);       // Set to manual input and set to dsd: 1 0 0 0 0 0 1 1
    lcd.print("DSD");
    break;
  */ 
  }
  unmuteDacs();                     // Unmute DACs
}

void setAndPrintInput(byte value){
  // read settings for the selected input and set and display parameters
  setAndPrintInputFormat(settings[input][FORMATVAL]%FORMATCHO);  // Setup input format value
  setAndPrintFirFilter(settings[input][FIRVAL]%FIRCHO);          // Setup FIR filter value
  setAndPrintIirFilter(settings[input][IIRVAL]%IIRCHO);          // Setup IIR filter value
  setAndPrintI2sDPLL(settings[input][I2SDPLLVAL]%I2SDPLLCHO);    // Setup I2S DPLL value
  setAndPrintDeemphasis(settings[input][DEEMVAL]%DEEMCHO);       // Setup de-emphasis value
  setAndPrintBalance(settings[input][BALVAL]%BALCHO);            // Setup balance value
  setAndPrintDsdDPLL(settings[input][DSDDPLLVAL]%DSDDPLLCHO);    // Setup DSD DPLL value
  lcd.setCursor(1,0);
  switch (value){
  case 0:
    lcd.print(no0);
    break;
  case 1:
    lcd.print(no1);
    break;
  case 2:
    lcd.print(no2);
    break;
  case 3:
    lcd.print(no3);
    break;
  case 4:
    lcd.print(no4);
    break;
  case 5:
    lcd.print(no5);
    break;
  }
}
    
/*
 Custom characters: I am using custom characters to build large size characters. These characters are
 9 times (3x3) the regular size. When you are far away using the remote control, you might need
 large characters to see what is being displayed.
 */

// Define 8 custom characters

byte cc0[8] = {     // Custom Character 0
  B00000,
  B00111,
  B01111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};

byte cc1[8] = {     // Custom Character 1
  B11100,
  B11110,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};

byte cc2[8] = {    // Custom Character 2
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B00000,
  B00000,
  B00000
};

byte cc3[8] = {    // Custom Character 3
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};

byte cc4[8] = {   // Custom Character 4
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B01111,
  B00111
};

byte cc5[8] = {    // Custom Character 5
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B00000,
  B00000
};

byte cc6[8] = {    // Custom Character 6
  B00000,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B00000,
  B00000
};

byte cc7[8] = {     // Custom Character 7
  B00000,
  B11100,
  B11110,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};

// Function to send custom characters to the display's RAM
void DefineCustomChar(){
  lcd.createChar(0,cc0);  // cc0 becomes character 0
  lcd.createChar(1,cc1);  // cc1 becomes character 1
  lcd.createChar(2,cc2);  // cc2 becomes character 2
  lcd.createChar(3,cc3);  // cc3 becomes character 3
  lcd.createChar(4,cc4);  // cc4 becomes character 4
  lcd.createChar(5,cc5);  // cc5 becomes character 5
  lcd.createChar(6,cc6);  // cc6 becomes character 6
  lcd.createChar(7,cc7);  // cc7 becomes character 7
}

// Array index into parts of big numbers. Each number consists of 9 custom characters
// in a 3x3 matrix. To print a number, you use the array index corresponding to the number
// times 3. For example to print the number 5, you will print bn1[15], bn1[16] and bn1[17]
// for the first row of the large number, and then bn2[15], bn2[16] and bn2[17] and so on.

// Keep in mind also that "A" and "B" are not the characters A and B but defined as special
// characters "space" and "solid block"

//            0      1      2      3      4      5      6      7      8      9    
char bn1[]={B,2,1, 2,1,A, 2,2,1, 2,2,1, 0,A,B, B,2,2, B,2,2, 2,2,B, B,2,1, B,2,1};
char bn2[]={B,A,B, A,B,A ,0,6,5, A,2,1, 5,6,B, 2,2,1, B,6,7, A,0,5, B,6,B, 5,6,B};
char bn3[]={4,3,B, 3,B,3, B,3,3, 4,3,B, A,A,B, 4,3,B, 4,3,B, A,B,A, 4,3,B, A,A,B};

// Functions for printing two large digits. Value is the column number
// and number is the number to print. Works from 00-99

void printTwoNumber(byte value, byte number){

  lcd.setCursor(value,1);  // Printing line 1 of the two-digit number
  lcd.write(bn1[(number/10)*3]);
  lcd.write(bn1[(number/10)*3+1]);
  lcd.write(bn1[(number/10)*3+2]);
  lcd.write(A); // Blank, not the character A
  lcd.write(bn1[(number%10)*3]);
  lcd.write(bn1[(number%10)*3+1]);
  lcd.write(bn1[(number%10)*3+2]);

  lcd.setCursor(value,2);  // Printing line 2 of the two-digit number
  lcd.write(bn2[(number/10)*3]);
  lcd.write(bn2[(number/10)*3+1]);
  lcd.write(bn2[(number/10)*3+2]);
  lcd.write(A); // Blank, not the character A
  lcd.write(bn2[(number%10)*3]);
  lcd.write(bn2[(number%10)*3+1]);
  lcd.write(bn2[(number%10)*3+2]);

  lcd.setCursor(value,3);  // Printing line 3 of the two-digit number
  lcd.write(bn3[(number/10)*3]);
  lcd.write(bn3[(number/10)*3+1]);
  lcd.write(bn3[(number/10)*3+2]);
  lcd.write(A); // Blank, not the character A
  lcd.write(bn3[(number%10)*3]);
  lcd.write(bn3[(number%10)*3+1]);
  lcd.write(bn3[(number%10)*3+2]);
}

/*
The following function returns the code from the Apple Aluminum remote control. The Apple remote is
 based on the NEC infrared remote protocol. Of the 32 bits (4 bytes) coded in the protocol, only the
 third byte corresponds to the keys. The function also handles errors due to noise (returns 255) and
 the repeat code (returs zero)
 
 The Apple remote returns the following codes:
 
   Up key:     238 135 011 089
   Down key:   238 135 013 089
   Left key:   238 135 008 089
   Right key:  238 135 007 089
   Center key: 238 135 093 089 followed by 238 135 004 089 (See blog for why there are two codes)
   Menu key:   238 135 002 089
   Play key:   238 135 094 089 followed by 238 135 004 089
   
 (update) The value of the third byte varies from remote to remote. It turned out that the 8th bit
 is not part of the command, so if we only read the 7 most significant bits, the value is the same
 for all the remotes, including the white platic remote.
 
 The value for the third byte when we discard the least significant bit is:
 
   Up key:     238 135 005 089
   Down key:   238 135 006 089
   Left key:   238 135 004 089
   Right key:  238 135 003 089
   Center key: 238 135 046 089 followed by 238 135 002 089 (See blog for why there are two codes)
   Menu key:   238 135 001 089
   Play key:   238 135 047 089 followed by 238 135 002 089
  
 More info here: http://hifiduino.wordpress.com/apple-aluminum-remote/
 */

int getIRkey() {
  
  c1=0;
  c2=0;
  c3=0;
  c4=0;
  duration=1;
  while((duration=pulseIn(REMOTEPIN, HIGH, 15000)) < 2000 && duration!=0)
  {
    // Wait for start pulse
  }
  if (duration == 0)         // This is an error no start or end of pulse
    return(255);             // Use 255 as Error

  else if (duration<3000)    // This is the repeat
    return (0);              // Use zero as the repeat code

  else if (duration<5000){   // This is the command get the 4 byte
    mask = 1;				    
    for (int i = 0; i < 8; i++){	             // get 8 bits
      if(pulseIn(REMOTEPIN, HIGH, 3000)>1000)        // If "1" pulse
        c1 |= mask;			             // Put the "1" in position
      mask <<= 1;				     // shift mask to next bit
    }
    mask = 1;				    
    for (int i = 0; i < 8; i++){	             // get 8 bits
      if(pulseIn(REMOTEPIN, HIGH, 3000)>1000)        // If "1" pulse
        c2 |= mask;			             // Put the "1" in position
      mask <<= 1;				     // shift mask to next bit
    }
    mask = 1;				    
    for (int i = 0; i < 8; i++){	             // get 8 bits
      if(pulseIn(REMOTEPIN, HIGH, 3000)>1000)        // If "1" pulse
        c3 |= mask;			             // Put the "1" in position
      mask <<= 1;				     // shift mask to next bit                                         // Now discard least significant bit
    }
    mask = 1;				    
    for (int i = 0; i < 8; i++){	             // get 8 bits
      if(pulseIn(REMOTEPIN, HIGH, 3000)>1000)        // If "1" pulse
        c4 |= mask;			             // Put the "1" in position
      mask <<= 1;				     // shift mask to next bit
    }
     //Serial.println(c1, HEX); //For debugging
     //Serial.println(c2, HEX); //For debugging
     //Serial.println(c3, HEX); //For debugging
     //Serial.println(c4, HEX); //For debugging
     
    c3>>= 1;                                         // Discard the least significant bit
    return(c3);
  }
  return(255);
}

// The following 2 routine sets up the registers in the DAC at startup

// First a couple of functions to mute and unmute the DACs

void muteDacs(){
  bitSet(reg7,0);               // Mute Channel 1
  bitSet(reg7,1);               // Mute Channel 2
  writeSabreReg(0x07,reg7);     // Mute DACs
}

void unmuteDacs(){
  bitClear(reg7,0);             // Unmute Channel 1
  bitClear(reg7,1);             // Unmute Channel 2
  writeSabreReg(0x07,reg7);     // Unmute DACs
}

void startDac1(){               // Set registers not requiring display info
  muteDacs();                   // Mute DACs
  muteDacs();                   // Redundant mute DACs
  writeSabreReg(0x00,reg0);     // System Settings
  writeSabreReg(0x04,reg4);     // Automute
  writeSabreReg(0x05,reg5);     // Automute Level
  writeSabreReg(0x08,reg8);     // GPIO default configuration
  writeSabreReg(0x0A,reg10);    // Master Mode. Default: OFF
  writeSabreReg(0x0E,reg14);    // Soft Start Settings
  writeSabreReg(0x0B,reg11S);   // stereo, etc (if mono, writeSabreReg() will take care of it)
  setSabreVolume(currAttnu);    // Startup volume level  
}

void startDac2(){
  readSettings();
  setAndPrintInput(input);
  lcd.setCursor(12,1);
  setSabreVolume(currAttnu);    // Set volume level just in case...
  printTwoNumber(13,currAttnu/2);    // Print default volume level
  unmuteDacs();                      // Unmute DACs
}

/************************ MAIN PROGRAM ************************************************************/

void setup() {

  Wire.begin();        // Join the I2C bus as a master
  startDac1();         // First part of starting the DAC

  // Attach Interrupts
  attachInterrupt(0, rotEncoder, LOW);  // ISR for rotary encoder on pin 2
  // Serial.begin(9600);  // This is here for debugging

  // Set up the pin modes
  pinMode(ROTPINA, INPUT);        // Button switch or Encoder pin for volume up
  digitalWrite(ROTPINA, HIGH);    // Enable pull-up resistor

  pinMode(ROTPINB, INPUT);        // Button switch or Encoder pin for volume down
  digitalWrite(ROTPINB, HIGH);    // Enable pull-up resistor

  pinMode(REMOTEPIN, INPUT);      // Pin for IR sensor
  digitalWrite(REMOTEPIN, HIGH);  // Enable pull-up resistor

  pinMode(SELECTPIN, INPUT);      // Button switch or pin for encoder built-in switch 
  digitalWrite(SELECTPIN, HIGH);  // Enable pull-up resistor 

  pinMode(6, OUTPUT);             // Controls the brightness of LCD through a transistor
  
  lcd.begin(20, 4);    // Set up the LCD's number of columns and rows:
  DefineCustomChar();  // Create the custom characters
  
  // Print the welcome message and other labels to the LCD. Coordinates: (column,row)

  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("H I F I D U I N O");
  lcd.setCursor(1,1);
  lcd.print("v.K2m05  -STEREO-");
  #ifdef DUALMONO
  lcd.setCursor(9,1);
  lcd.print("DUAL-MONO");
  #endif
  lcd.setCursor(4,2);
  lcd.print("JUN 29 2014");
  lcd.setCursor(2,3);
  chipStatus=readRegister(64);                    // Read chip status register
  if((chipStatus&B00011100)==16){                 // This is the chip ID of the K2M
    lcd.print("ES9018K2M ");                      // Print chip ID
    if(chipStatus&B00100000) lcd.print("rev V");  // Print version
    else lcd.print("rev W");
  }
  else lcd.print("   -NOT K2M-");                 // Cannot identify chip or nothing attached
   
  delay(4000);
  
  lcd.clear();
  startDac2();  // prints all the DAC settings in the screen; reprogram volume level which will
                // help prevent the potential startup at full volume that happens if the code
                // executes before the registers of the DAC are ready to accept data.
                // This does not catch the case where the DAC is restarted/reset without restarting
                // Arduino. This case should be prevented.

}  // End setup()

// -------------------------------------loop section------------------------------------------
void loop() {

  // Print the sample rate, input and lock (once every "INTERVAL_SAMPLE" time)
  if((millis() - displayMillis > INTERVAL_SAMPLE*1000)&&!selectMode) {
    displayMillis = millis(); // Saving last time we display sample rate
    chipStatus=readRegister(64); // Read status register
      
    // Update the sample rate display if there is a lock (otherwise, "no lock")
    lcd.setCursor(8,0);         // Clear the old sample rate reading
    lcd.print("            ");
    lcd.setCursor(8,0);
    if(chipStatus&B00000001){    // There is a lock in a signal
      sr=sampleRate();           // Get the sample rate from register
      if(sr>2822000)             // Determine of DSD or PCM
          lcd.print("DSD ");
        else
          lcd.print("PCM ");
      if(SRExact==true){
        lcd.print(" ");      
        lcd.print(sr, DEC);      // Print sample rate in exact format
      }
        else                     // Print sample rate in nominal format
          if(sr>6143000)
            lcd.print(" 6.1 MHz");
          else
            if(sr>5644000)
              lcd.print(" 5.6 MHz");
            else
              if(sr>3071000)
                lcd.print(" 3.0 MHz");
              else
                if(sr>2822000)
                  lcd.print(" 2.8 MHz");
                else
                  if(sr>383900)
                    lcd.print(" 384 KHz");
                  else
                    if(sr>352700)
                      lcd.print(" 352 KHz");
                    else
                      if(sr>191900)
                        lcd.print(" 192 KHz");
                      else
                        if(sr>176300)
                          lcd.print(" 176 KHz");
                        else
                          if(sr>95900)
                            lcd.print(" 96.0KHz");
                          else
                            if(sr>88100)
                              lcd.print(" 88.2KHz");
                            else
                              if(sr>47900)
                                lcd.print(" 48.0KHz");
                              else
                                if(sr>44000)
                                  lcd.print(" 44.1KHz");
                                else
                                  lcd.print(" UNKNOWN");
      }
    else {                       // There is no lock
      lcd.print("     No Lock");
    }
    // Heartbeat to show that the software is still running...
    lcd.setCursor(0,1);
    if(pulse++%2)lcd.write(0xDE);  // Print a "pulse" to indicate the controller is working
    else lcd.write(0xEB);
  }
  
    /*
   The following code is for the remote control. It can handle all the codes generated by the Apple
   remote control except code "0" has been designated (by me) for "repeat" and code "255" designated
   (by me)as an error code. The current Apple remote does not generate these codes so it is safe to
   designate them for other things.
   
   In addition, NEW: the code ignores the repeat code by default, except if designated for repeat. In
   this case the keys for up and down volume have been designated for repeat.
   
   */
   
  while(digitalRead(REMOTEPIN)==LOW){
    // Serial.println("hi");
    if((IRkey=getIRkey())==255){
      // Do nothing
    }
    else { 
      if(IRkey==0) {            // Repeat code                           
        if(previousIRkey==5||previousIRkey==6) { // Specify key=5 and key=6 as repeatable keys
                                                 // specify other keys here to make them repeatable
          IRkey=previousIRkey;  // Repeat code
        }
        else {
          // Do nothing. No repeat for the rest of the keys
        }
      }
      else {                    // Not a repeat code, it is a new command
        previousIRkey=IRkey;    // Remember the key in case we want to use the repeat code
      }
    }
      
    switch(IRkey){
    // case 0 and 255 are "valid" cases from the code, but we do nothing in this switch statement
    case 5:  // 5 is the up key, we will use for volume up
      if (currAttnu>MINATTNU)                // Check if not already at minimum attenuation
      {
        if(dimmed) {
          rampUp();                          // Disengage dim
          dimmed=false;
        }
        
        currAttnu-=2;                      // Decrease attenuation 1 dB (increase volume 1 db)
        setSabreVolume(currAttnu);         // Write value into registers
        printTwoNumber(13,currAttnu/2);    // Divide by 2 to print in whole dBs
        
      }
      break;
    case 6:  // 6 is the down key, we will use for volume down
      if (currAttnu<MAXATTNU)              // Check if not already at maximum attenuation
      {
        if(dimmed) {
          rampUp();                        // Disengage dim
          dimmed=false;
        }
        
        currAttnu+=2;                      // Increase 1 dB attenuation (decrease volume 1 db)
        setSabreVolume(currAttnu);         // Write value into registers
        printTwoNumber(13,currAttnu/2);    // Divide by 2 to print in whole dBs
        
      }
      break;
    case 2:  // 4 is the center key. This is the soft mute -dim feature
      if(dimmed){                          // Back to previous volume level 1 db at a time
        rampUp();                          // Disengage dim
        dimmed=false;
      }
      else {
        if(DIM>=currAttnu) {               // only DIM if current attenuation is lower
          setSabreVolume(DIM);             // Dim volume
          printTwoNumber(13,DIM/2); 
          dimmed=true;
        }
      }
      break;
    }
  } // End of remote control code

  /*
  To debounce the switch, we detect the first instance of the "click" and then ignore
   the switch: first during the switch "bouncing" time and then while the switch remains
   pressed because one cannot lift the finger so fast.
   We want to register a single "click" per push of the switch
   */

  if((digitalRead(SELECTPIN)==LOW)&&((millis()-debounceMillis)>INTERVAL_SWITCHBOUNCE)) {
    selectMode=true;          // Now we are in select mode
    printSelectBar(255);      // Indicate we are in select mode
    debounceMillis=millis();  // Start debounce timer
    selectMillis=millis();    // Start being-in-select-mode timer

    select++;  // Move to next select option
    // First read the current setting for input format since it is not normally displayed
    // Tnis was the case in the ES9080 where the Status register tells you the format of the
    // incoming data. Here in the k2m version, there is no status on the format so format
    // selection is the only thing we can display. This code is optional but doesn't hurt.
    lcd.setCursor(8,0);
    switch(settings[input][FORMATVAL]%FORMATCHO){
    case 0:
      lcd.print("AUT");
      break;
    case 1:
      lcd.print("I/D");
      break;
    case 2:
      lcd.print("SPd");
      break;
    case 3:
      lcd.print("I16");
      break;
    case 4:
      lcd.print("L16");
      break;
      }
      
    // Then read the sample rate display format which is not normally displayed
    lcd.setCursor(13,0);
    if(SRExact==true)                      
      lcd.print(" EXACT ");
    else
      lcd.print("NOMINAL");
    // Indicate with arrow which parameter to adjust
    switch(select%(MAXPARAM+2)){
    case VOL:
      break;
    case INP:
      lcd.setCursor(0,0);
      lcd.write(126);
      break;
    case INF:
      lcd.setCursor(7,0);
      lcd.write(126);
      break;
    case FIR:
      lcd.setCursor(0,1);
      lcd.write(126);
      break;
    case IIR:
      lcd.setCursor(7,1);
      lcd.write(126);
      break;
    case LLI:
      lcd.setCursor(0,2);
      lcd.write(126);
      break;
    case DEE:
      lcd.setCursor(0,3);
      lcd.write(126);
      break;
    case BAL:
      lcd.setCursor(7,3);
      lcd.write(126);
      break;
    case LLD:
      lcd.setCursor(7,2);
      lcd.write(126);
      break;
    case SRF:
      lcd.setCursor(12,0);
      lcd.write(126);
      break;
    }  // End of switch
  }
  
  // For the rotary encoder. The control depends whether in display mode or select mode
  if(rotating)
  {
    delay(INTERVAL_BOUNCE);                  // debounce by waiting INTERVAL_BOUNCE time

    switch(select%(MAXPARAM+2)){
    case VOL:  // Volume adjustment
      dimmed=false;                          // Disengage dim
      if (digitalRead(ROTPINA) == digitalRead(ROTPINB))  // CCW -increase attenuation
      {
        if (currAttnu<MAXATTNU)              // Check if not already at maximum attenuation
        {
          currAttnu+=2;                      // Increase 1 dB attenuation (decrease volume 1 db)
          setSabreVolume(currAttnu);         // Write value into registers
          printTwoNumber(13,currAttnu/2);    // Divide by 2 to print in whole dBs
        }
      }
      else                                   // If not CCW, then it is CW
      {
        if (currAttnu>MINATTNU)              // Check if not already at minimum attenuation
        {
          currAttnu-=2;                      // Decrease attenuation 1 dB (increase volume 1 db)
          setSabreVolume(currAttnu);         // Write value into registers
          printTwoNumber(13,currAttnu/2);    // Divide by 2 to print in whole dBs
        }
      }
      break;
    case INP:  // Input selection
      if (digitalRead(ROTPINA) == digitalRead(ROTPINB)) input--;  // CCW
      else input++;
      input%=ICHO;
      setAndPrintInput(input);
      selectMillis=millis();  // Reset being-in-select-mode timer
      break;
    case INF:  // Input format selection
      if (digitalRead(ROTPINA) == digitalRead(ROTPINB)) settings[input][FORMATVAL]--;  // CCW      
      else settings[input][FORMATVAL]++;
      setAndPrintInputFormat(settings[input][FORMATVAL]%FORMATCHO);
      selectMillis=millis();  // Reset being-in-select-mode timer
      break;
    case FIR:  // FIR filter selection
      if (digitalRead(ROTPINA) == digitalRead(ROTPINB)) settings[input][FIRVAL]--;  // CCW
      else settings[input][FIRVAL]++;
      setAndPrintFirFilter(settings[input][FIRVAL]%FIRCHO); 
      selectMillis=millis();  // Update being-in-select-mode timer
      break;
    case IIR:  // IIR filter selection
      if (digitalRead(ROTPINA) == digitalRead(ROTPINB)) settings[input][IIRVAL]--;  // CCW
      else settings[input][IIRVAL]++;
      setAndPrintIirFilter(settings[input][IIRVAL]%IIRCHO);
      selectMillis=millis();  // Update being-in-select-mode timer
      break;
    case DEE:  // De-emphasis selection
      if (digitalRead(ROTPINA) == digitalRead(ROTPINB)) settings[input][DEEMVAL]--;  // CCW
      else settings[input][DEEMVAL]++;
      setAndPrintDeemphasis(settings[input][DEEMVAL]%DEEMCHO);
      selectMillis=millis();  // Reset being-in-select-mode timer
      break;
    case LLI:  // DPLL setting for I2S
      if (digitalRead(ROTPINA) == digitalRead(ROTPINB)) settings[input][I2SDPLLVAL]--;  // CCW
      else settings[input][I2SDPLLVAL]++;
      setAndPrintI2sDPLL(settings[input][I2SDPLLVAL]%I2SDPLLCHO);
      selectMillis=millis();  // Reset being-in-select-mode timer
      break;
    case BAL:  // Balance setting
      if (digitalRead(ROTPINA) == digitalRead(ROTPINB)){
        if (settings[input][BALVAL]%BALCHO>0) settings[input][BALVAL]--;   // CCW
      }
      else{
        if (settings[input][BALVAL]%BALCHO<38) settings[input][BALVAL]++;
      }
      setAndPrintBalance(settings[input][BALVAL]%BALCHO); 
      selectMillis=millis();  // Reset being-in-select-mode timer
      break;
    case LLD:  // DPLL setting for DSD
      if (digitalRead(ROTPINA) == digitalRead(ROTPINB)) settings[input][DSDDPLLVAL]--;  // CCW
      else settings[input][DSDDPLLVAL]++;
      setAndPrintDsdDPLL(settings[input][DSDDPLLVAL]%DSDDPLLCHO);
      selectMillis=millis();  // Reset being-in-select-mode timer
      break;
    case SRF:  //Sampler Rate format -Since this is a toggle, we just call the function
      setAndPrintSRFormat();
      selectMillis=millis();  // Reset being-in-select-mode timer
      break;

    }  // End of (rotary encoder) switch
    rotating=false; // Reset the flag
    
  }  // End of if(rotating)

  // When the being-in-select mode timer expires, we revert to volume/display mode
  if(selectMode&&millis()-selectMillis > INTERVAL_SELECT*1000){
    selectMode=false;  // No longer in select mode
    printSelectBar(A); // "Removes" the select bar by printing blanks
    select=VOL;        // Back to volume mode
    writeSettings();   // Write new settings into EEPROM, including current input
  } 
}  // End of loop()

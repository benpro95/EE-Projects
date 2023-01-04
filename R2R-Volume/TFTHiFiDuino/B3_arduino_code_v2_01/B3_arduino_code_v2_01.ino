/**************************************************************************************************
 * TFT HIFIDUINO Code: My adaptation of the HiFiDUINO code so that is displays on a colour TFT.
 *
 * Project Page: http://www.dimdim.gr/arduino/tft-hifiduino-code/
 *
 * v2.01    01/01/15 : - Bugfixes (Exact SR not displaying, SR redrawing every 2 sec).
 *
 * v2.00    31/12/14 : - Official release of v2 code!
 *                     - New graphics in limited display mode. Code size is now definitely over 256KB.
 *
 * v1.20    22/12/14 : - !! Still very ALPHA !! 
 *                     - Sidecar signalling changed from LOW->HIGH for B3 to support new switching circuit.
 *
 * v1.19    07/12/14 : - !! Still very ALPHA !! now defaults to a limited display. Goes into full display when 
 *                       changes are made to parameters.
 *                     - Outputs debugging data to SerialUSB.
 *
 * v1.18    02/12/14 : - !! Very ALPHA !! now defaults to a limited display. Goes into full display when 
 *                       changes are made to parameters.
 *
 * v1.14    28/11/14 : - Changed IR code to IRremote library. Now supports a lot more remote controls.
 *                     - Beta support for proportional fonts via UTFT_DLB library (add-on to UTFT).
 *
 * v1.08    19/11/14 : - Added code to use MCP23008 to control misc devices.
 *                     - Added option to set 0db as default (power-on) volume.
 * 
 * v1.06    01/11/13 : - Compatible with Buffalo 3 and Buffalo 3 SE. Just comment out the relevant statement.
 *                     - Fixed "OS Filt" & "SR disp".. They were not working correctly.
 *                     - Blue select boxes are gone.. they looked quite bad.
 *                     - Some other minor (mainly aesthetic) fixes..
 *
 * v1.03    30/11/13 : - Buffalo IIISE edition. Just 3 inputs: S/PDIF + 2xUSB.
 *
 * v1.00    03/09/13 : - Finished with aesthetics (new fonts, selection grid, etc. First official release.
 *
 * v0.96    25/03/13 : - Changed all pins so as to be compatible with the new Buffalo Shield.
 *
 * v0.95    25/02/13 : - Added IR codes for select, left, right. Everything now works on MEGA & Due! 
 *                       Display still buggy (no functional problems though).
 * 
 * v0.93    17/02/13 : - Changed the rot encoder code. Should be compatible with Due & Mega but only 
 *                       works on the MEGA for the time being.
 *
 * v0.91g   22/12/12 : - Added code to support rotary encoder on Due & added relevant option for selecting 
 *                       either MEGA or Due board (!! still not working!!).
 *                     - Fixed s/pdif frequency display.
 *
 * v0.9     13/12/12 : - Fixed bug in volume control code. Now works as it is supposed to. :)
 *                     - Remote works very well. To do: TFT backlight control.
 *
 * v.0.8.8  10/12/12 : - Powerup and shutdown routines added (still need to configure IR code for power).
 *                     - Aesthetic changes.
 *
 * v.0.8.7  09/12/12 : - Adapted most of HiFiDuino's enhancements to the code. Display is buggy.
 *
 * v.0.8.6  08/12/12 : - Now saves settings into 24LC256 EEPROM instead of built-in EEPROM (beta, seems 
 *                       to be working OK).
 *
 * v.0.8.5  07/12/12 : - Addition of USB input (9th). To do: set as default input.
 *                     - Now fully tested with I2S sources.
 *
 * v.0.8.4  15/08/12 : - Code cleanup, reordering of functions, etc.
 *
 * v.0.8    28/04/12 : - Now supports 240x400 instead of 240x320 TFT.
 *
 * v.0.7    16/04/12 : - Major revision. Now supports UTFT library for 240x320 TFT.
 *
 * v.0.51   11/03/12 : - Contains a few minor bugs but for the most part works.
 *                     - Includes support of source selection on Buffalo III boards.  
 *                     - The Apple remote codes are changed to another device's that I had handy.
 *                     - Has not been tested with I2S sources.
 *
 * HIFIDUINO's original comments have been retired. They can be found on the documentation (see link on top of file).
 *
 ***************************************************************************************************/


/***************************************************************************************************
 * Code starts here
 ***************************************************************************************************/

// LIBRARIES
#include <Wire.h> // For I2C
#include "Adafruit_MCP23008.h" //for the I2C port expander
#include <UTFT.h>
#include <UTFT_DLB.h>
#include <IRremote2.h>

// CONSTANT DEFINITION

// Declare which fonts we will be using
extern uint8_t SmallFont[];
extern uint8_t BigFont[];
extern uint8_t Ubuntubold[];
extern uint8_t Shruti_Bold_num_48x70[];
extern uint8_t calibri_bold_80[];
extern uint8_t calibri_bold_60[];
extern uint8_t calibri_bold_40[];
extern uint8_t calibri_bold_26[];

UTFT_DLB myGLCD(ITDB32WD,38,39,40,41);

//******* Bitmaps *********/

extern unsigned short usb[0x3840];
extern unsigned short toslink[0x3840];
extern unsigned short bnc[0x3840];
extern unsigned short dsd[0x1BA8];
extern unsigned short pcm[0x1AC5];
extern unsigned short spdif[0x1428];
extern unsigned short nolock[0x3840];
extern unsigned short xlr[0x3840];

/******************* Code Customization Section *********************/


/* First: Choose the clock frequency you have and comment the other */

//#define USE80MHZ  
#define USE100MHZ

/* Second: Choose your configuration
 
 | CONFIGURATION       | #define DUALMONO | #define TPAPHASE |
 |---------------------|------------------|------------------|
 | Dual mono in-phase  | un-comment       | comment          |
 | Dual mono TPA phase | un-comment       | un-comment       |
 | Stereo              | comment          | comment          |
 |---------------------|------------------|------------------|    */

//#define DUALMONO
//#define TPAPHASE

/* Choose the kind of Buffalo that you have. Either BUFFALO3 or B3SE. If you have a 32S or a II choose B3SE.*/
#define BUFFALO3
//#define B3SE

/* Optionally choose whether you will be using the remote power on/off functionality.*/
/* Comment out if you will be using the remote for on/off.*/
//#define ALWAYSON

/* Optionally choose 0db as the default (on power-on) volume level.*/
/* Comment out if you like the -50db setting.*/
//#define FULLVOL

/* Optionally choose the number of inputs.*/
#ifdef B3SE
  #define ICHO 3
#endif B3SE

#ifdef BUFFALO3
  #define ICHO 10
#endif BUFFALO3

/* Optionally change the name of the inputs. They all have to take up the same length on the display. 
   If you decide to change the names, some trial and error with the blanks may be required since the font is proportional. */

#ifdef B3SE
  char no0[] = "S/PDIF";
  char no1[] = "USB 1";
  char no2[] = "USB 2";
#endif B3SE

#ifdef BUFFALO3
  char no0[] = "COAX 1";
  char no1[] = "OPT 1";
  char no2[] = "OPT 2";
  char no3[] = "AES/EBU";
  char no4[] = "COAX 2";
  char no5[] = "COAX 3";
  char no6[] = "COAX 4";
  char no7[] = "COAX 5";
  char no8[] = "USB 1";
  char no9[] = "USB 2";
#endif BUFFALO3

/* Make sure you use the correct chip address for each board
   for stereo Buffalo: use address 0x90
   for dual mono: Use address 0x90 for mono left Buffalo 
   Use address 0x92 for mono right Buffalo           */

/********************************************************************/

#define DEFAULTATTNU 0x64 //-50 dB this is 50x2=100 or 0x64. Sabre32 is 0 to -127dB in .5dB steps

#ifdef FULLVOL
  #define DEFAULTATTNU 0x00 //0 dB this is 0x2=0 or 0x00. Sabre32 is 0 to -127dB in .5dB steps
#endif FULLVOL

#define MAXATTNU 0xC6     //-99dB this is 99X2=198 or 0xC6 -Can go higher but LCD shows 2 digits
#define MINATTNU 0x00     //-0 dB -Maximum volume is -0 dB
#define DIM 0x8C          //-70 dB this is 70x2=140 or 0x8C. Dim volume
#define RAMP 10           // Additional msec delay per 1 db for ramping up to volume after dim

int VOLUPPIN=6;          // RotEnc A terminal for right rotary encoder.
int VOLDOWNPIN=7;        // RotEnc B terminal for right rotary encoder.
#define SELECTPIN 5      // Switch to select function for right rotary encoder.

int VOLUPPIN2=A4;          // RotEnc A terminal for left rotary encoder.
int VOLDOWNPIN2=A3;        // RotEnc B terminal for left rotary encoder.
#define SELECTPIN2 A5      // Switch to select function for left rotary encoder.

#define SETTINGFONT calibri_bold_26    // Font for displaying various settings
#define SRFONT calibri_bold_40      // Font for displaying Sampling Rate

int irdir=0;

// Remote control definitions section
int RECV_PIN = 9;        // Pin for IR receiver (remote control)
IRrecv irrecv(RECV_PIN);
decode_results results;
int prev_result;
#define POWER_CODE 0xFF48B7      // Code for power on/off
#define VOLUP_CODE 0xFF30CF      // Code for Volume up
#define VOLDOWN_CODE 0xFF609F    // Code for Volume down
#define MUTE_CODE  0xFF40BF      // Code for mute
#define SELECT_CODE 0xFF02FD     // Code for Select button
#define LEFT_CODE 0xFF9867       // Code for left arrow
#define RIGHT_CODE 0xFF8877      // Code for right arrow
#define SOURCE1_CODE 0xFF827D    // Code for source 1
#define SOURCE2_CODE 0xFFB24D    // Code for source 2
#define SOURCE3_CODE 0xFFA25D    // Code for source 3
#define SOURCE4_CODE 0xFF42BD    // Code for source 4
#define SOURCE5_CODE 0xFF728D    // Code for source 5
#define SOURCE6_CODE 0xFF629D    // Code for source 6
#define SOURCE7_CODE 0xFFC23D    // Code for source 7
#define SOURCE8_CODE 0xFFF20D    // Code for source 8
#define SOURCE9_CODE 0xFFE21D    // Code for source 9
#define SOURCE10_CODE 0xFF00FF   // Code for source 10


#ifdef B3SE
#define SELECTI2SPIN A7      // Pin for S/PDIF vs I2S selection. High for S/PDIF. Low for I2S.
#define AUX1PIN A2          // Pin #1 for controlling other things
#define AUX2PIN 3          // Pin #2 for controlling other things
#endif B3SE

#ifdef BUFFALO3
#define SELECTI2SPIN A2      // Pin for Sidecar activation (Select I2S vs. S/PDIF)
#define AUX1PIN 3          // Pin #1 for controlling other things
#define AUX2PIN A7          // Pin #2 for controlling other things
#endif BUFFALO3

#define POWERPIN A0        // Pin for main power activation
#define DIMMPIN 8          // Pin for TFT backlight controll. A LOW on this pin turns on the backlight.

#define INTERVAL_SAMPLE 1     // Time interval in SECONDS for refreshing the sample rate
#define INTERVAL_BOUNCE 2     // Time in milliseconds to debounce the rotary encoder
#define INTERVAL_SWITCHBOUNCE 200  // Time in milliseconds to debounce switch
#define INTERVAL_SELECT 4     // Time in sec to exit select mode when no activity

#define VOL 0  // The order of selection when clicking the select switch
#define INP 1  // Input selection
#define INF 2  // Input format selection
#define FIR 3  // FIR filter selection
#define IIR 4  // IIR filter selection
#define DPL 5  // DPLL bandwidth setting
#define QTZ 6  // Quantizer selection
#define NCH 7  // Notch selection
#define PLM 8  // DPLL mode
#define OSF 9  // Oversampling
#define SRF 10 // Sample Rage format: "nominal" or "exact"

// Order of settings in the array for each input
#define FORMATVAL 0
#define FIRVAL 1
#define IIRVAL 2
#define DPLLVAL 3
#define QUANVAL 4
#define NOTCHVAL 5
#define PLMVAL 6
#define OSFVAL 7
#define SRFVAL 8

/* Total number of parameters to keep track for each input. The 9 listed above plus the input
 choice. This makes 10.
 VOL is not tracked per input as it always starts with the default volume
 Thus there are 10 parameters, 9 which are saved per input, and 1 is across all inputs
 */

#define MAXPARAM 10                      

// Number of valid choices for each parameter
#define FORMATCHO 2 // SPDIF, I2S/DSD
#define FIRCHO 2    // Fast, slow
#define IIRCHO 4    // PCM, 50k, 60k, 70k
#define DPLLCHO 8   // Lowest to Best 8 values
#define QUANCHO 6   // 6bT, 7bP, 7bT, 8bP, 8bT, 9bP
#define NOTCHCHO 6  // /4 to /64
#define PLMCHO 4    // Normal x1 (NOR), Multiplier x128 (MUL), Bypass (OFF), INV (Invert phase)
#define OSFCHO 2    // Oversampling filter. On or off.
#define SRFCHO 2    // Sampling frequency display. Normal or exact.
// There is one more: ICHO defined above

#define chip1 0x50    // device address for the 24LC256 EEPROM chip

// VARIABLE DECLARATION

// Register variables: used for registers that have lots of options. They are initialized here
// with valid values, but will eventually be overwritten by the values in EEPROM
byte reg14=0xF9;   // Default value for register 14. We use a variable for reg 14 because it controls
                   // several parameters: IIR, FIR, differential whereas the other registers typically
                   // controls a single parameter.
byte reg25=0;      // 0= allow all settings
byte reg17L=0x1C;  // Auto SPDIF, 8-channel mode, other defaults
                   // reg17L is used for stereo and for left channel if set for MONO
#ifdef DUALMONO
byte reg17R=0x9D;  // Auto SPDIF, MONO RIGHT CHANNEL, other defaults
#endif DUALMONO

byte reg10=0xCE; // jitter ON, dacs unmute, other defaults

unsigned long displayMillis = 0;   // Stores last recorded time for display interval
unsigned long debounceMillis = 0;  // Stores last recorded time for switch debounce interval
unsigned long selectMillis = 0;    // Stores last recorded time for being in select mode
unsigned long sr = 0;              // Stores the incoming sample rate. Used for auto OSF bypass for 80MHz crystal
unsigned long srold = 0;         // Stores the already displayed incoming sample rate. Used to save screen refreshes if the sr does not change.

byte input=0;                 // The current input to the DAC
byte currAttnu=DEFAULTATTNU;  // Variable to hold the current attenuation value

byte select;              // To record current select position (FIL, VOL, DPLL, etc)

boolean selectMode=false; // To indicate whether in select mode or not
boolean SPDIFValid;       // To indicate whether SPDIF valid data
boolean spdifIn;          // To indicate whether in I2S/DSD or SPDIF input format mode
boolean bypassOSF=false;  // false=no bypass; This is the default condition at startup
boolean SRExact=true;     // Display exact sample rate value; false = display nominal value
boolean primed=false;     // To indicate if dpll has been conditioned (once at startup)
boolean dimmed=false;     // To indicate dim (mute) or not
boolean poweron=false;    // Default power-on condition: off
boolean leandisp=true;    // Default display mode: lean
byte pulse=0;             // Used by the "heartbeat" display
byte Status;              // To hold value of Sabre32 status register

int inputtype;            // To hold the type of input type. Set to 1 for DSD, 2 for PCM, 3 for I2S, 4 is for No Lock.

Adafruit_MCP23008 mcp;

// The array holds the parameters for each input
// The current input is recorded after the array
byte settings[ICHO][MAXPARAM];  // Array to hold parameter values

void writeSettings(){
  for(byte i=0;i<ICHO;i++) {
    for (byte j=0;j<MAXPARAM;j++) {
      if(readData(chip1,(i*MAXPARAM)+j)!=settings[i][j]) // Check an see if there are any changes
        writeData(chip1,(i*MAXPARAM)+j,settings[i][j]);  // Write the changes in eeprom
    }
  }
  writeData(chip1,ICHO*MAXPARAM,input); // Write the current input in a variable location after the array
  writeData(chip1,ICHO*MAXPARAM+1,SRExact); // Write the value of SRExact
  //serialUSB.println("Setting written");
   
}

void readSettings(){
  for(byte i=0;i<ICHO;i++) {
    for (byte j=0;j<MAXPARAM;j++) {
      settings[i][j]=readData(chip1, (i*MAXPARAM)+j);
    }
  }
  input=readData(chip1,ICHO*MAXPARAM);  // Read the last saved input setting
  SRExact=readData(chip1,ICHO*MAXPARAM+1);  // Read the last saved setting for SRExact
  //serialUSB.println("Setting read");
   
}

/*
 ROTARY ENCODER
 
 The rotary encoder is connected to digital pin 2 (A) and digital pin 4 (B). It does not matter 
 which terminal is connected to which pin. The third terminal is connected to GND. At each cycle, 
 the rotary encoder will pull the pins LOW or HIGH.
 
 Debounce
 In this version, the code implements debouncing by adding a few millisecond delay that pauses 
 the code from evaluating the state of the pins. Typically, all the switching noise
 is generated in the first few milliseconds after the switch event. The code is optimized by calling 
 the delay only if there is any activity in the rotary encoder.
 
 There is just a slight change in the software if H/W debouncing is implemented, namely the pull up 
 resistors in the pins are to be disabled.
 
 Further, the rotary encoder has an on-off switch, and the debouncing of the switch is also done in s/w
 (again the same h/w debouncing can be implemented, but it is optional). Because pushing the switch manually 
 will generate not only noise during the switching, but the switch can remain pressed for 100 milliseconds 
 or more because one cannot lift the finger that fast.
 In this implementation and with my way of pusshing down the switch, 200 msec is an appropriate value.
 
 */

// Rotary encoder service routine
static boolean rotating=false;
void rotEncoder()
{
  rotating=true; // If motion is detected in the rotary encoder, set the flag to true
}

static boolean rotating2=false;
void rotEncoder2()
{
  rotating2=true; // If motion is detected in the rotary encoder, set the flag to true
}

/*
READING THE SAMPLE RATE
 
 The sample rate can be calculated by reading the DPLL 32-bit register. For SPDIF DPLL value 
 is divided by (2^32/Crystal-Frequency). In Buffalo II (original), the Crystal frequency is
 80,000,000 Hz. In Arduino (and other small microprocessors) it is NOT advisable to do floating point
 math because "it is very slow"; therefore integer math will be used to calculate the sample rate.
 
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
 
 I used a spreadsheet to calculate the multipliers for SPDIF and I2S and for both 80Mhz and 100Mhz
 
 SPDIF 80Mhz:  x80, %4295
 SPDIF 100Mhz: x20, %859
 I2S 80Mhz:     x1, %3436
 I2S 100Mhz:    x4, %10995 (higher multiplier will overflow the 32bit value for 384KHz SR)
 x5, %13744 (More accurate but only works up to 192KHz SR)
 
 For I2S input format the dpll number is divided by (2^32*64/Crystal-Frequency) Note the 64 factor.
 The value of this is 3435.97 which rounds off nicely to 3436 (which is only 0.0008% error). The
 resultant value for the sample rate is the same wheter in spdif or I2S mode.
 */

// Sample rate reading routines

volatile unsigned long DPLLNum; // Variable to hold DPLL value
volatile unsigned long RegVal;  // Variable to hold Register values

byte readDPLL(byte regAddr) {
  Wire.beginTransmission(0x48); // Hard coded the Sabre/Buffalo device  address
  Wire.write(regAddr);          // Queues the address of the register
  Wire.endTransmission();       // Sends the address of the register
  Wire.requestFrom(0x48,1);     // Hard coded to Buffalo, request one byte from address
  if(Wire.available())          // Wire.available indicates if data is available
      return Wire.read();       // Wire.read() reads the data on the wire
  else
    return 0;                   // In no data in the wire, then return 0 to indicate error
}

unsigned long sampleRate() {
  DPLLNum=0;
  // Reading the 4 registers of DPLL one byte at a time and stuffing into a single 32-bit number
  DPLLNum|=readDPLL(31);
  DPLLNum<<=8;
  DPLLNum|=readDPLL(30);
  DPLLNum<<=8;
  DPLLNum|=readDPLL(29);
  DPLLNum<<=8;
  DPLLNum|=readDPLL(28);
  // The following calculation supports either 80 MHz or 100MHz oscillator
  if (SPDIFValid){
#ifdef USE80MHZ
    DPLLNum*=80;      // Calculate SR for SPDIF -80MHz part
    DPLLNum/=4295;    // Calculate SR for SDPIF -80MHz part
#endif
#ifdef USE100MHZ
    DPLLNum*=20;      // Calculate SR for SPDIF -100MHz part 
    DPLLNum/=859;     // Calculate SR for SDPIF -100MHz part
#endif
  }
  else {              // Different calculation for SPDIF and I2S
#ifdef USE80MHZ
    DPLLNum/=3436;    // Calculate SR for I2S -80MHz part
#endif
#ifdef USE100MHZ
    DPLLNum*=4;      // Calculate SR for I2S -100MHz part
    DPLLNum/=10995;  // Calculate SR for I2S -100MHz part
#endif
  }
  if (DPLLNum < 90000) // Adjusting because in integer operation, the residual is truncated
    DPLLNum+=1;
  else
    if (DPLLNum < 190000)
      DPLLNum+=2;
    else
      if (DPLLNum < 350000)
        DPLLNum+=3;
      else
        DPLLNum+=4;

  if(bypassOSF)      // When OSF is bypassed, the magnitude of DPLL is reduced by a factor of 64
    DPLLNum*=64;

  return DPLLNum;
}

 

// Reading the status register. There is a status register providing the following information:
// 1- dsd or pcm mode
// 2- spdif valid or invalid
// 3- spdif mode enabled or disabled. 
// 4- Jitter Eliminator locked/not locked to incoming signal

byte readStatus() {             // Hardcoded to the status register
  Wire.beginTransmission(0x48); // Hard coded the Sabre/Buffalo device  address
  Wire.write(27);               // Hard coded to status register
  Wire.endTransmission();
  Wire.requestFrom(0x48,1);     // Hard coded to Buffalo, request one byte from address
  if(Wire.available())
    return Wire.read();         // Return the value returned by specified register
  else
    return 0;
}

/*
CONTROLLING THE DIGITAL ATTENUATION (VOLUME) -and other registers IN THE DAC
 
 The device address of Sabre DAC Datasheet specifies the address as 0x90 which is an 8-bit value.
 The wire library in Arduino uses 7-bit device addresses and the 8th R/W bit is added automatically
 depending on whether you use the write call [beginTransmission()] or the read call [requestFrom()].
 Therefore, you will use the 7 most significant bits of the 8-bit address.
 In our example, 0x90 becomes 0x48 as follows:
 0x90: 10010000 (we eliminate the rightmost bit to get I2C address)
 0x48: 1001000
 When using dual-mono configuration, the other device can be set to addres 0x92
 0x92: 10010010 (we eliminate the rightmost bit to get I2C address)
 0x49: 1001001
 */

// The default mode is for the address 0x48 to be the left chip
void writeSabreLeftReg(byte regAddr, byte regVal)
{
  Wire.beginTransmission(0x48); // Hard coded to the the Sabre/Buffalo device address for stereo
                                // or mono left. For stereo same as writeSabreReg()
  Wire.write(regAddr);          // Specifying the address of register
  Wire.write(regVal);           // Writing the value into the register
  Wire.endTransmission();
}

// In dual mono, sometimes different values are written to L and R chips
#ifdef DUALMONO
void writeSabreRightReg(byte regAddr, byte regVal)
{
  Wire.beginTransmission(0x49); //Hard coded to the the Sabre/Buffalo device address
  Wire.write(regAddr); // Specifying the address of register
  Wire.write(regVal); // Writing the value into the register
  Wire.endTransmission();
}
#endif DUALMONO

// The following routine writes to both chips in dual mono mode. With some exceptions, one only needs
// to set one of the chips to be the right channel

void writeSabreReg(byte regAddr, byte regVal)
{
  // By default the chip with address 0x48 is the left channel
  writeSabreLeftReg(regAddr, regVal);
  
  #ifdef DUALMONO
  // If dual mono write also to the other chip. 
  writeSabreRightReg(regAddr, regVal);
  // Set the right chip (addr 0x49) to be the right channel
  reg17R=reg17;
  writeSabreRightReg(0x11, bitSet(reg17R,7));
  #endif DUALMONO
}

void setSabreVolume(byte regVal)
{
  writeSabreReg(0, regVal); // set up volume in DAC1
  writeSabreReg(1, regVal); // set up volume in DAC2
  writeSabreReg(2, regVal); // set up volume in DAC3
  writeSabreReg(3, regVal); // set up volume in DAC4
  writeSabreReg(4, regVal); // set up volume in DAC5
  writeSabreReg(5, regVal); // set up volume in DAC6
  writeSabreReg(6, regVal); // set up volume in DAC7
  writeSabreReg(7, regVal); // set up volume in DAC8
}


void dispVol(byte currAttnu)
{
  int x;
  int y;
  
  if (leandisp!=true) 
      {
        x = 230;  // x coordinate of where the volume display area begins
        y = 100;  // y coordinate of where the volume display area begins
        myGLCD.setFont(BigFont);
        myGLCD.setColor(0, 0, 0);
        myGLCD.setBackColor(255, 255, 255);
        myGLCD.print("- ", x, y);
        if (currAttnu/2<10)
        {
          myGLCD.setColor(255, 255, 255);
          myGLCD.fillRect(x+12, y, x+50, y+70);
          myGLCD.setColor(0, 0, 0);
          myGLCD.setBackColor(255, 255, 255);
          myGLCD.setFont(Shruti_Bold_num_48x70);
          myGLCD.printNumI(currAttnu/2, x+59, y);
        }
        else
        {
          myGLCD.setColor(0, 0, 0);
          myGLCD.setBackColor(255, 255, 255);
          myGLCD.setFont(Shruti_Bold_num_48x70);
          myGLCD.printNumI(currAttnu/2, x+11, y);
        }
       myGLCD.setFont(Ubuntubold);
       myGLCD.setColor(0, 0, 0);
       myGLCD.setBackColor(255, 255, 255);
       myGLCD.print("dB", x+110, y+25);
       selectMillis=millis();  // Reset being-in-select-mode timer
      }
   else
      {
        x = 120;  // x coordinate of where the volume display area begins
        y = 10;   // y coordinate of where the volume display area begins
        myGLCD.setColor(0, 0, 0);
        myGLCD.setBackColor(255, 255, 255);
        myGLCD.setFont(calibri_bold_80);
        myGLCD.print("- ", x-10, y);
                
        if (currAttnu/2<10)
        {
          myGLCD.setColor(255, 255, 255);
          myGLCD.fillRect(x+12, y, x+50, y+70);
          myGLCD.setColor(0, 0, 0);
          myGLCD.setFont(Shruti_Bold_num_48x70);
          myGLCD.printNumI(currAttnu/2, x+59, y);
        }
        else
        {
          myGLCD.setFont(Shruti_Bold_num_48x70);
          myGLCD.printNumI(currAttnu/2, x+11, y);
        }
        myGLCD.setFont(calibri_bold_60);
        myGLCD.print("dB", x+110, y+20);
      }
  
//serialUSB.println("Volume displayed");
 
}

void rampUp()
{
  byte i=(DIM-currAttnu); 
  for(byte dimval=DIM;dimval>currAttnu;dimval--){
    setSabreVolume(dimval);
    dispVol(dimval);
    delay((RAMP)*(1+(10/i*i)));
    i--;
  }
}

void setAndPrintInputFormat(byte value, boolean invert){
  // This register also controls mono-8channel operation, thus more code...

  switch(value){
    case 0:
      writeSabreReg(0x08,0xE8);        // Reg 8: Enable SPDIF input format
      bitSet(reg17L,3);                // Reg 17: auto spdif detection ON -Set bit 3
      writeSabreLeftReg(0x11,reg17L);  // Reg 17: write value into register
      #ifdef DUALMONO
      bitSet(reg17R,3);                // Reg 17: auto spdif detection ON -Set bit 3
      writeSabreRightReg(0x11,reg17R); // Reg 17: write value into register
      #endif DUALMONO
      spdifIn=true;                    // Indicates input format is spdif. For label for input format
      if (leandisp!=true) 
      {
        myGLCD.setFont(SETTINGFONT);
        myGLCD.setColor(0, 255, 0);
        myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
        myGLCD.print("Format: ", 2, 2);
        myGLCD.print("S/PDIF   ", myGLCD.getStringWidth("Format: "), 2);
      }
      break;

    case 1:
      bitClear(reg17L,3);              // Reg 17: manual SPDIF -Clear bit 3
      writeSabreLeftReg(0x11,reg17L);  // Reg 17: Auto spdif detection OFF
      #ifdef DUALMONO
      bitClear(reg17R,3);              // Reg 17: manual SPDIF -Clear bit 3
      writeSabreRightReg(0x11,reg17R); // Reg 17: Auto spdif detection OFF
      #endif DUALMONO
      writeSabreReg(0x08,0x68);        // Reg 8: Enable I2S/DSD input format
      spdifIn=false;                   // Set variable to indicate input format is I2S/DSD mode
      if (leandisp!=true) 
      {
        myGLCD.setFont(SETTINGFONT);
        myGLCD.setColor(0, 255, 0);
        myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
        myGLCD.print("Format: ", 2, 2);
        myGLCD.print("I2S/DSD", myGLCD.getStringWidth("Format: "), 2);   
      }
      break;
    }
  }

void setAndPrintFirFilter(byte value, boolean invert){

  switch(value){
  case 0:
    bitSet(reg14,0);           // Set bit 0 of reg 14 for sharp fir
    writeSabreReg(0x0E,reg14);
    if (leandisp!=true) 
    {
      myGLCD.setFont(SETTINGFONT);
      myGLCD.setColor(0, 150, 255);
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      myGLCD.print("Filter: ", 2, myGLCD.getFontHeight()+2);
      myGLCD.print("Sharp FIR", myGLCD.getStringWidth("Filter: "), myGLCD.getFontHeight()+2);
    }
    break;

  case 1:
    bitClear(reg14,0);         // Clear bit 0 of reg 14 for slow fir
    writeSabreReg(0x0E,reg14);
    if (leandisp!=true) 
    {
      myGLCD.setFont(SETTINGFONT);
      myGLCD.setColor(0, 150, 255);
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      myGLCD.print("Filter: ", 2, myGLCD.getFontHeight()+2);
      myGLCD.print("Slow FIR  ", myGLCD.getStringWidth("Filter: "), myGLCD.getFontHeight()+2);
    }
   break;
  }
}

void setAndPrintIirFilter(byte value, boolean invert){

  //serialUSB.println(value);
  
  switch(value){
  case 0:                        // | | | | | |0|0| | IIR Bandwidth: Normal (for PCM)
    bitClear(reg14,1);           // Clear bit 1
    bitClear(reg14,2);           // Clear bit 2
    writeSabreReg(0x0E,reg14);
    if (leandisp!=true) 
    {
      myGLCD.setFont(SETTINGFONT);
      myGLCD.setColor(0, 150, 255);
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      myGLCD.print("IIR: ", 2, 2*myGLCD.getFontHeight()+4);
      myGLCD.print("PCM", myGLCD.getStringWidth("IIR: "), 2*myGLCD.getFontHeight()+4);
    }
    break;

  case 1:                        // | | | | | |0|1| | IIR Bandwidth: 50k (for DSD) (D)
    bitSet(reg14,1);             // Set bit 1
    bitClear(reg14,2);           // Clear bit 2
    writeSabreReg(0x0E,reg14);
    if (leandisp!=true) 
    {
      myGLCD.setFont(SETTINGFONT);
      myGLCD.setColor(0, 150, 255);
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      myGLCD.print("IIR: ", 2, 2*myGLCD.getFontHeight()+4);
      myGLCD.print("50K ", myGLCD.getStringWidth("IIR: "), 2*myGLCD.getFontHeight()+4);
    }
    break;

  case 2:                        // | | | | | |1|0| | IIR Bandwidth: 60k (for DSD)
    bitSet(reg14,2);             // Set bit 2
    bitClear(reg14,1);           // Clear bit 1
    writeSabreReg(0x0E,reg14);
    if (leandisp!=true) 
    {
      myGLCD.setFont(SETTINGFONT);
      myGLCD.setColor(0, 150, 255);
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      myGLCD.print("IIR: ", 2, 2*myGLCD.getFontHeight()+4);
      myGLCD.print("60K ", myGLCD.getStringWidth("IIR: "), 2*myGLCD.getFontHeight()+4);
    }
    break;

  case 3:                        // | | | | | |1|1| | IIR Bandwidth: 70k (for DSD)
    bitSet(reg14,1);             // Set bit 1
    bitSet(reg14,2);             // Set bit 2
    writeSabreReg(0x0E,reg14);
    if (leandisp!=true) 
    {
      myGLCD.setFont(SETTINGFONT);
      myGLCD.setColor(0, 150, 255);
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      myGLCD.print("IIR: ", 2, 2*myGLCD.getFontHeight()+4);
      myGLCD.print("70K ", myGLCD.getStringWidth("IIR: "), 2*myGLCD.getFontHeight()+4);
    }
    break;
  }
}


void setAndPrintDPLL(byte value, boolean invert){

    if (leandisp!=true) 
    {
      myGLCD.setFont(SETTINGFONT);
      myGLCD.setColor(0, 150, 255);
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      myGLCD.print("DPLL B/W: ", 2, 3*myGLCD.getFontHeight()+6);
        
  switch(value){
  case 0:
    bitSet(reg25,1);            // Reg 25: set bit 1 for "use best dpll"
    writeSabreReg(0x19,reg25);  // Write value into reg 25 for best dpll
    if (leandisp!=true) 
    {
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      myGLCD.print("Auto      ", myGLCD.getStringWidth("DPLL B/W: "), 3*myGLCD.getFontHeight()+6);
    }
    break;
  case 1:
    bitClear(reg25,1);          // Reg 25: Clear bit 1 for "use all settings"
    writeSabreReg(0x19,reg25);  // Write value into reg 25 for all settings
    writeSabreReg(0x0B,0x85);   // Reg 11: Set corresponding DPLL bandwidth
    if (leandisp!=true) 
    {
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      myGLCD.print("Lowest    ", myGLCD.getStringWidth("DPLL B/W: "), 3*myGLCD.getFontHeight()+6);
    }
    break;
  case 2:
    bitClear(reg25,1);          // Reg 25: Clear bit 1 for "use all settings"
    writeSabreReg(0x19,reg25);  // Write value into reg 25 for all settings
    writeSabreReg(0x0B,0x89);   // Reg 11: Set corresponding DPLL bandwidth
    if (leandisp!=true) 
    {
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      myGLCD.print("Low      ", myGLCD.getStringWidth("DPLL B/W: "), 3*myGLCD.getFontHeight()+6);
    }
    break;
  case 3:
    bitClear(reg25,1);          // Reg 25: Clear bit 1 for "use all settings"
    writeSabreReg(0x19,reg25);  // Write value into reg 25 for all settings
    writeSabreReg(0x0B,0x8D);   // Reg 11: Set corresponding DPLL bandwidth
    if (leandisp!=true) 
    {
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      myGLCD.print("Low-Med  ", myGLCD.getStringWidth("DPLL B/W: "), 3*myGLCD.getFontHeight()+6);
    }
    break;
  case 4:
    bitClear(reg25,1);          // Reg 25: Clear bit 1 for "use all settings"
    writeSabreReg(0x19,reg25);  // Write value into reg 25 for all settings
    writeSabreReg(0x0B,0x91);   // Reg 11: Set corresponding DPLL bandwidth
    if (leandisp!=true) 
    {
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      myGLCD.print("Medium   ", myGLCD.getStringWidth("DPLL B/W: "), 3*myGLCD.getFontHeight()+6);
    }
    break;
  case 5:
    bitClear(reg25,1);          // Reg 25: Clear bit 1 for "use all settings"
    writeSabreReg(0x19,reg25);  // Write value into reg 25 for all settings
    writeSabreReg(0x0B,0x95);   // Reg 11: Set corresponding DPLL bandwidth
    if (leandisp!=true) 
    {
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      myGLCD.print("Med-High", myGLCD.getStringWidth("DPLL B/W: "), 3*myGLCD.getFontHeight()+6);
    }
    break;
  case 6:
    bitClear(reg25,1);          // Reg 25: Clear bit 1 for "use all settings"
    writeSabreReg(0x19,reg25);  // Write value into reg 25 for all settings
    writeSabreReg(0x0B,0x99);   // Reg 11: Set corresponding DPLL bandwidth
    if (leandisp!=true) 
    {
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      myGLCD.print("High        ", myGLCD.getStringWidth("DPLL B/W: "), 3*myGLCD.getFontHeight()+6);
    }
    break;
  case 7:
    bitClear(reg25,1);          // Reg 25: Clear bit 1 for "use all settings"
    writeSabreReg(0x19,reg25);  // Write value into reg 25 for all settings
    writeSabreReg(0x0B,0x9D);   // Reg 11: Set corresponding DPLL bandwidth
    if (leandisp!=true) 
    {
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      myGLCD.print("Highest    ", myGLCD.getStringWidth("DPLL B/W: "), 3*myGLCD.getFontHeight()+6);
    }
    break;
  }
  }
}


void setAndPrintQuantizer(byte value, boolean invert){

    if (leandisp!=true) 
    {
      myGLCD.setFont(SETTINGFONT);
      myGLCD.setColor(0, 150, 255);
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      myGLCD.print("Qtz: ", 2, 4*myGLCD.getFontHeight()+8);
    }
    switch(value){
    case 0:                        // 6-bit true diff
      bitSet(reg14,3);             // True differential
      writeSabreReg(0x0E,reg14);
      writeSabreReg(0x0F,0x00);    // 6-bit quantizer
      if (leandisp!=true) 
      {
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
        myGLCD.print("6bT", myGLCD.getStringWidth("Qtz: "), 4*myGLCD.getFontHeight()+8);
      }
      break;

    case 1:                        // 7-bit pseudo diff
      bitClear(reg14,3);           // Pseudo diff
      writeSabreReg(0x0E,reg14);
      writeSabreReg(0x0F,0x55);    // 7-bit quantizer
      if (leandisp!=true) 
      {
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
        myGLCD.print("7bP", myGLCD.getStringWidth("Qtz: "), 4*myGLCD.getFontHeight()+8);
      } 
      break;

    case 2:                        // 7-it true diff
      bitSet(reg14,3);             // True differential
      writeSabreReg(0x0E,reg14);
      writeSabreReg(0x0F,0x55);    // 7-bit quantizer
      if (leandisp!=true) 
      {
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
        myGLCD.print("7bT", myGLCD.getStringWidth("Qtz: "), 4*myGLCD.getFontHeight()+8);
      }
      break;

    case 3:                        // 8-bit pseudo diff
      bitClear(reg14,3);           // Pseudo diff
      writeSabreReg(0x0E,reg14);
      writeSabreReg(0x0F,0xAA);    // 8-bit quantizer
      if (leandisp!=true) 
      {
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
        myGLCD.print("8bP", myGLCD.getStringWidth("Qtz: "), 4*myGLCD.getFontHeight()+8);
      }
      break;  

    case 4:                        // 8-bit true diff
      bitSet(reg14,3);             // True differential
      writeSabreReg(0x0E,reg14);
      writeSabreReg(0x0F,0xAA);    // 8-bit quantizer
      if (leandisp!=true) 
      {
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
        myGLCD.print("8bT", myGLCD.getStringWidth("Qtz: "), 4*myGLCD.getFontHeight()+8);
      }
      break;

    case 5:                        // 9-bit pseudo diff
      bitClear(reg14,3);           // Pseudo diff
      writeSabreReg(0x0E,reg14);
      writeSabreReg(0x0F,0xFF);    // 9-bit quantizer
      if (leandisp!=true) 
      {
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
        myGLCD.print("9bP", myGLCD.getStringWidth("Qtz: "), 4*myGLCD.getFontHeight()+8);
      }
      break; 
  }
}

void setAndPrintNotch(byte value, boolean invert){

    if (leandisp!=true) 
    {
      myGLCD.setFont(SETTINGFONT);
      myGLCD.setColor(0, 150, 255);
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      myGLCD.print("Notch: ", 2, 5*myGLCD.getFontHeight()+10);
    }

  switch(value){
  case 0:
    writeSabreReg(0x0C,0x20);    // No notch delay
    if (leandisp!=true) 
    {
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      myGLCD.print("NON", myGLCD.getStringWidth("Notch: "), 5*myGLCD.getFontHeight()+10);
    }
    break;
  case 1:
    writeSabreReg(0x0C,0x21);    // notch delay=mclk/4
    if (leandisp!=true) 
    {
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      myGLCD.print("/04  ", myGLCD.getStringWidth("Notch: "), 5*myGLCD.getFontHeight()+10);
    }
    break;
  case 2:
    writeSabreReg(0x0C,0x23);    // notch delay=mclk/8
    if (leandisp!=true) 
    {
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      myGLCD.print("/08  ", myGLCD.getStringWidth("Notch: "), 5*myGLCD.getFontHeight()+10);
    }
    break;
  case 3:
    writeSabreReg(0x0C,0x27);    // notch delay=mclk/16
    if (leandisp!=true) 
    {
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      myGLCD.print("/16  ", myGLCD.getStringWidth("Notch: "), 5*myGLCD.getFontHeight()+10);
    }
    break;  
  case 4:
    writeSabreReg(0x0C,0x2F);    // notch delay=mclk/32
    if (leandisp!=true) 
    {
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      myGLCD.print("/32  ", myGLCD.getStringWidth("Notch: "), 5*myGLCD.getFontHeight()+10);
    }
    break;
  case 5:
    writeSabreReg(0x0C,0x3F);    // notch delay=mclk/64
    if (leandisp!=true) 
    {
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      myGLCD.print("/64  ", myGLCD.getStringWidth("Notch: "), 5*myGLCD.getFontHeight()+10);
    }
    break; 
  }
}


void setAndPrintDPLLMode(byte value, boolean invert){ // Set the DPLL Mode

    if (leandisp!=true) 
    {
      myGLCD.setFont(SETTINGFONT);
      myGLCD.setColor(0, 150, 255);
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      myGLCD.print("DPLL Mode: ", 2, 6*myGLCD.getFontHeight()+12);
    }
    
  switch(value){
  case 0:
    bitSet(reg10,2);           // Set bit 2 of reg 10: jitter reduction ON
    writeSabreReg(0x0A,reg10);
    bitSet(reg25,0);           // Set bit 0 of reg 25 for x128 DPLL bandwidth
    writeSabreReg(0x19,reg25);
    bitClear(reg17L,1);        // Reg 17: Clear bit 1 to NOT invert DPLL phase
    writeSabreLeftReg(0x11,reg17L);
    #ifdef DUALMONO
      bitClear(reg17R,1);        // Reg 17: Clear bit 1 to NOT invert DPLL phase -dual mono 
      writeSabreRightReg(0x11,reg17R);
    #endif DUALMONO
    if (leandisp!=true) 
    {
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      myGLCD.print("x128  ", myGLCD.getStringWidth("DPLL Mode: "), 6*myGLCD.getFontHeight()+12);
    }
    break;
  case 1:
    bitSet(reg10,2);           // Set bit 2 of reg 10: jitter reduction ON
    writeSabreReg(0x0A,reg10);
    bitClear(reg25,0);         // Clear bit 0 of reg 25 for x1 DPLL bandwidth
    writeSabreReg(0x19,reg25);
    bitClear(reg17L,1);        // Reg 17: Clear bit 1 to NOT invert DPLL phase
    writeSabreLeftReg(0x11,reg17L);
    #ifdef DUALMONO
      bitClear(reg17R,1);        // Reg 17: Clear bit 1 to NOT invert DPLL phase -dual mono 
      writeSabreRightReg(0x11,reg17R);
    #endif DUALMONO
    if (leandisp!=true) 
    {
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      myGLCD.print("x1    ", myGLCD.getStringWidth("DPLL Mode: "), 6*myGLCD.getFontHeight()+12);
    }
    break;
  case 2:                      
    bitSet(reg10,2);           // Set bit 2 of reg 10: jitter reduction ON
    writeSabreReg(0x0A,reg10);
    bitClear(reg25,0);         // Clear bit 0 of reg 25 for x1 DPLL bandwidth
    writeSabreReg(0x19,reg25);
    bitSet(reg17L,1);          // Reg 17: Set bit 1 to invert DPLL phase
    writeSabreLeftReg(0x11,reg17L);
    #ifdef DUALMONO
      bitSet(reg17R,1);          // Reg 17: Set bit 1 to invert DPLL phase -dual mono 
      writeSabreRightReg(0x11,reg17R);
    #endif DUALMONO
    if (leandisp!=true) 
    {
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      myGLCD.print("x1 INV", myGLCD.getStringWidth("DPLL Mode: "), 6*myGLCD.getFontHeight()+12);
    }
    break;
  case 3:
    bitClear(reg10,3);         // Clear bit 3 of reg 10: jitter reduction bypass
    writeSabreReg(0x0A,reg10);
    if (leandisp!=true) 
    {
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      myGLCD.print("None  ", myGLCD.getStringWidth("DPLL Mode: "), 6*myGLCD.getFontHeight()+12);
    }
    break;
  }
}

void setAndPrintBypassOSF(byte value, boolean invert){         // Set and print Bypass OSF
    
    myGLCD.setFont(SETTINGFONT);
    myGLCD.setColor(0, 150, 255);
  
  switch(value){
  case 0:
    bypassOSF=false;
    bitClear(reg17L,6);              // Reg 17: clear bypass oversampling bit in register
    writeSabreLeftReg(0x11,reg17L);  // Reg 17: bypass OSF off
    #ifdef DUALMONO
      bitClear(reg17R,6);              // Reg 17: clear bypass oversampling bit in register -dual mono
      writeSabreRightReg(0x11,reg17R); // Reg 17: bypass OSF off
    #endif DUALMONO
    if (leandisp!=true) 
    {
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      myGLCD.print("OS filt: ", 2, 7*myGLCD.getFontHeight()+14);
      myGLCD.print("On ", myGLCD.getStringWidth("OS filt: "), 7*myGLCD.getFontHeight()+14);      // Indicate oversampling is ON
    }
    break;
  case 1:
    bypassOSF=true;                  // This is to bypass oversampling
    bitSet(reg17L,6);                // Reg 17: set bypass oversampling bit in register
    bitSet (reg17L,5);               // Reg 17: set Jitter lock bit, normal operation
    writeSabreLeftReg(0x11,reg17L);  // Reg 17: bypass OSF on, force relock
    delay(50);
    bitClear(reg17L,5);              // Reg 17: clear relock jitter for normal operation
    writeSabreLeftReg(0x11,reg17L);  // Reg 17: Jitter eliminator Normal operation 
    #ifdef DUALMONO
      bitSet(reg17R,6);                // Reg 17: set bypass oversampling bit in register -dual mono
      bitSet (reg17R,5);               // Reg 17: set Jitter lock bit, normal operation -dual mono
      writeSabreRightReg(0x11,reg17R); // Reg 17: bypass OSF on, force relock
      delay(50);
      bitClear(reg17L,5);              // Reg 17: clear relock jitter for normal operation
      writeSabreLeftReg(0x11,reg17L);  // Reg 17: Jitter eliminator Normal operation
    #endif DUALMONO
    if (leandisp!=true) 
    {
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      myGLCD.print("OS filt: ", 2, 7*myGLCD.getFontHeight()+14);
      myGLCD.print("Off", myGLCD.getStringWidth("OS filt: "), 7*myGLCD.getFontHeight()+14);      // Indicate oversampling is OFF
    }
    break;
  }
}

void setAndPrintSRFormat(byte value, boolean invert){          // This is a toggle function for selecting SR display format

  myGLCD.setFont(SETTINGFONT);
  myGLCD.setColor(0, 150, 255);

  switch(value){
  case 0:
    SRExact=false;                   // Set to Nominal
    if (leandisp!=true) 
    {
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      myGLCD.print("SR Disp: ", 2, 8*myGLCD.getFontHeight()+16);
      myGLCD.print("Nominal", myGLCD.getStringWidth("SR Disp: "), 8*myGLCD.getFontHeight()+16);    // Indicate NOMINAL mode
    }
    break;

  case 1:
    SRExact=true;                    // Set to display exact sample rate
    if (leandisp!=true) 
    {
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      myGLCD.print("SR Disp:", 2, 8*myGLCD.getFontHeight()+16);
      myGLCD.print("Exact     ", myGLCD.getStringWidth("SR Disp: "), 8*myGLCD.getFontHeight()+16);    // Indicate EXACT mode
    }
    break;
  }
}


void setAndPrintInput(byte value, boolean invert){
  setAndPrintInputFormat(settings[input][FORMATVAL]%FORMATCHO, false);  // Setup input format value
  setAndPrintFirFilter(settings[input][FIRVAL]%FIRCHO, false);          // Setup FIR filter value
  setAndPrintIirFilter(settings[input][IIRVAL]%IIRCHO, false);          // Setup IIR filter value
  setAndPrintDPLL(settings[input][DPLLVAL]%DPLLCHO, false);             // Setup the DPLL value
  setAndPrintQuantizer(settings[input][QUANVAL]%QUANCHO, false);        // Setup quantizer value
  setAndPrintNotch(settings[input][NOTCHVAL]%NOTCHCHO, false);          // Setup notch delay value
  setAndPrintDPLLMode(settings[input][PLMVAL]%PLMCHO, false);           // Setup dpll mode value
  setAndPrintBypassOSF(settings[input][OSFVAL]%OSFCHO, false);          // Setup oversampling bypass value
  setAndPrintSRFormat(settings[input][SRFVAL]%SRFCHO, false);           // Setup Sampling Rate display format
  dispVol(currAttnu);

  //serialUSB.println("Input parameters set");

  if (leandisp!=true) 
  {
    myGLCD.setFont(Ubuntubold);
    myGLCD.setColor(255, 255, 255);
    myGLCD.fillRect(230, 9, 395, 80);
    myGLCD.setColor(0, 0, 0);
    myGLCD.setBackColor(255, 255, 255);
    if (invert==true)
    {
      myGLCD.setColor(0, 0, 0);
      myGLCD.fillRect(230, 9, 395, 80);
      myGLCD.setColor(255, 255, 255);
      myGLCD.setBackColor(0, 0, 0);
    }  
    myGLCD.print("Input:", 230, 10);
  }  

  myGLCD.setFont(calibri_bold_40);
  
  #ifdef B3SE
  switch (value){
  case 0:
    writeSabreReg(0x12,0x01);          // Set input to SPDIF (#1)
    digitalWrite(SELECTI2SPIN, LOW);   // Keep IP_S high.
    if (leandisp!=true) 
    {
      myGLCD.print(no0, 230, 42);
    } else
    {
      myGLCD.drawBitmap(30, 90, 120, 120, bnc);
      myGLCD.setColor(255, 255, 255);
      myGLCD.fillRect(1, 207, 195, 239);
      myGLCD.setColor(0, 0, 0);
      myGLCD.print(no0, 90-(myGLCD.getStringWidth(no0)/2), 202);
    }
    break;
  case 1:
    writeSabreReg(0x12,0x01);          // Set input to USB 1 (#2)
    digitalWrite(SELECTI2SPIN, HIGH);  // Pull IP_S low.
    digitalWrite(AUX1PIN, LOW);        // Select Input 1 on OTTO-II.
    if (leandisp!=true) 
    {
      myGLCD.print(no1, 230, 42);
    } else
    {
      myGLCD.drawBitmap(30, 90, 120, 120, usb);
      myGLCD.setColor(255, 255, 255);
      myGLCD.fillRect(1, 207, 195, 239);
      myGLCD.setColor(0, 0, 0);
      myGLCD.print(no1, 90-(myGLCD.getStringWidth(no1)/2), 202);
    }

    break;
  case 2:
    writeSabreReg(0x12,0x01);          // Set input to USB 2 (#3)
    digitalWrite(SELECTI2SPIN, HIGH);  // Pull IP_S low.
    digitalWrite(AUX1PIN, HIGH);       // Select Input 2 on OTTO-II.
    if (leandisp!=true) 
    {
      myGLCD.print(no2, 230, 42);
    } else
    {
      myGLCD.drawBitmap(30, 90, 120, 120, usb);
      myGLCD.setColor(255, 255, 255);
      myGLCD.fillRect(1, 207, 195, 239);
      myGLCD.setColor(0, 0, 0);
      myGLCD.print(no2, 90-(myGLCD.getStringWidth(no2)/2), 202);
    }
    break;
  }
  #endif B3SE
  
  #ifdef BUFFALO3
  switch (value){
  case 0:
    writeSabreReg(0x12,0x01);         // Set SPDIF to input #1
    digitalWrite(SELECTI2SPIN, LOW); // Power off the sidecar relay.
    mcp.digitalWrite(0, LOW);         // Alternative method of powering off the sidecar relay.
    if (leandisp!=true) 
    {
      myGLCD.print(no0, 230, 42);
    } else
    {
      myGLCD.drawBitmap(30, 90, 120, 120, bnc);
      myGLCD.setColor(255, 255, 255);
      myGLCD.fillRect(1, 207, 195, 239);
      myGLCD.setColor(0, 0, 0);
      myGLCD.print(no0, 90-(myGLCD.getStringWidth(no0)/2), 202);
    }
    break;
  case 1:
    writeSabreReg(0x12,0x02);         // Set SPDIF to input #2
    digitalWrite(SELECTI2SPIN, LOW); // Power off the sidecar relay.
    mcp.digitalWrite(0, LOW);         // Alternative method of powering off the sidecar relay.
    if (leandisp!=true) 
    {
      myGLCD.print(no1, 230, 42);
    } else
    {
      myGLCD.drawBitmap(30, 90, 120, 120, toslink);
      myGLCD.setColor(255, 255, 255);
      myGLCD.fillRect(1, 207, 195, 239);
      myGLCD.setColor(0, 0, 0);
      myGLCD.print(no1, 90-(myGLCD.getStringWidth(no1)/2), 202);
    }
    break;
  case 2:
    writeSabreReg(0x12,0x04);         // Set SPDIF to input #3
    digitalWrite(SELECTI2SPIN, LOW); // Power off the sidecar relay.
    mcp.digitalWrite(0, LOW);         // Alternative method of powering off the sidecar relay.
    if (leandisp!=true) 
    {
      myGLCD.print(no2, 230, 42);
    } else
    {
      myGLCD.drawBitmap(30, 90, 120, 120, toslink);
      myGLCD.setColor(255, 255, 255);
      myGLCD.fillRect(1, 207, 195, 239);
      myGLCD.setColor(0, 0, 0);
      myGLCD.print(no2, 90-(myGLCD.getStringWidth(no2)/2), 202);
    }
    break;
  case 3:
    writeSabreReg(0x12,0x08);         // Set SPDIF to input #4
    digitalWrite(SELECTI2SPIN, LOW); // Power off the sidecar relay.
    mcp.digitalWrite(0, LOW);         // Alternative method of powering off the sidecar relay.
    if (leandisp!=true) 
    {
      myGLCD.print(no3, 230, 42);
    } else
    {
      myGLCD.drawBitmap(30, 90, 120, 120, xlr);
      myGLCD.setColor(255, 255, 255);
      myGLCD.fillRect(1, 207, 195, 239);
      myGLCD.setColor(0, 0, 0);
      myGLCD.print(no3, 90-(myGLCD.getStringWidth(no3)/2), 202);
    }
    break;
  case 4:
    writeSabreReg(0x12,0x10);         // Set SPDIF to input #5
    digitalWrite(SELECTI2SPIN, LOW); // Power off the sidecar relay.
    mcp.digitalWrite(0, LOW);         // Alternative method of powering off the sidecar relay.
    if (leandisp!=true) 
    {
      myGLCD.print(no4, 230, 42);
    } else
    {
      myGLCD.drawBitmap(30, 90, 120, 120, bnc);
      myGLCD.setColor(255, 255, 255);
      myGLCD.fillRect(1, 207, 195, 239);
      myGLCD.setColor(0, 0, 0);
      myGLCD.print(no4, 90-(myGLCD.getStringWidth(no4)/2), 202);
    }
    break;
  case 5:
    writeSabreReg(0x12,0x20);         // Set SPDIF to input #6
    digitalWrite(SELECTI2SPIN, LOW); // Power off the sidecar relay.
    mcp.digitalWrite(0, LOW);         // Alternative method of powering off the sidecar relay.
    if (leandisp!=true) 
    {
      myGLCD.print(no5, 230, 42);
    } else
    {
      myGLCD.drawBitmap(30, 90, 120, 120, bnc);
      myGLCD.setColor(255, 255, 255);
      myGLCD.fillRect(1, 207, 195, 239);
      myGLCD.setColor(0, 0, 0);
      myGLCD.print(no5, 90-(myGLCD.getStringWidth(no5)/2), 202);
    }
    break;
  case 6:
    writeSabreReg(0x12,0x40);         // Set SPDIF to input #7
    digitalWrite(SELECTI2SPIN, LOW); // Power off the sidecar relay.
    mcp.digitalWrite(0, LOW);         // Alternative method of powering off the sidecar relay.
    if (leandisp!=true) 
    {
      myGLCD.print(no6, 230, 42);
    } else
    {
      myGLCD.drawBitmap(30, 90, 120, 120, bnc);
      myGLCD.setColor(255, 255, 255);
      myGLCD.fillRect(1, 207, 195, 239);
      myGLCD.setColor(0, 0, 0);
      myGLCD.print(no6, 90-(myGLCD.getStringWidth(no6)/2), 202);
    }
    break;
  case 7:
    writeSabreReg(0x12,0x80);         // Set SPDIF to input #8
    digitalWrite(SELECTI2SPIN, LOW); // Power off the sidecar relay.
    mcp.digitalWrite(0, LOW);         // Alternative method of powering off the sidecar relay.
    if (leandisp!=true) 
    {
      myGLCD.print(no7, 230, 42);
    } else
    {
      myGLCD.drawBitmap(30, 90, 120, 120, bnc);
      myGLCD.setColor(255, 255, 255);
      myGLCD.fillRect(1, 207, 195, 239);
      myGLCD.setColor(0, 0, 0);
      myGLCD.print(no7, 90-(myGLCD.getStringWidth(no7)/2), 202);
    }
    break;
  case 8:
    writeSabreReg(0x12,0x01);         // Set input to USB 1 (#9)
    digitalWrite(SELECTI2SPIN, HIGH);  // Power on the sidecar relay.
    mcp.digitalWrite(0, HIGH);        // Alternative method of powering on the sidecar relay.
    if (leandisp!=true) 
    {
      myGLCD.print(no8, 230, 42);
    } else
    {
      myGLCD.drawBitmap(30, 90, 120, 120, usb);
      myGLCD.setColor(255, 255, 255);
      myGLCD.fillRect(1, 207, 195, 239);
      myGLCD.setColor(0, 0, 0);
      myGLCD.print(no8, 90-(myGLCD.getStringWidth(no8)/2), 202);
    }
    break;
  case 9:
    writeSabreReg(0x12,0x01);         // Set input to USB 2 (#9)
    digitalWrite(SELECTI2SPIN, HIGH);  // Power on the sidecar relay.
    mcp.digitalWrite(0, HIGH);        // Alternative method of powering on the sidecar relay.
    if (leandisp!=true) 
    {
      myGLCD.print(no9, 230, 42);
    } else
    {
      myGLCD.drawBitmap(30, 90, 120, 120, usb);
      myGLCD.setColor(255, 255, 255);
      myGLCD.fillRect(1, 207, 195, 239);
      myGLCD.setColor(0, 0, 0);
      myGLCD.print(no9, 90-(myGLCD.getStringWidth(no9)/2), 202);
    }
    break;
  }
  #endif BUFFALO3
}


// The following 2 routines set up the registers in the DAC at startup

void startDac1(){               // Set registers not requiring display info

// Print the welcome message and other labels to the LCD

  myGLCD.clrScr();
  myGLCD.setColor(255, 255, 255);
  myGLCD.fillRoundRect(0, 0, 399, 239);
  myGLCD.setFont(calibri_bold_40);
  myGLCD.setColor(0, 0, 255);
  myGLCD.setBackColor(255, 255, 255);
  myGLCD.print("T F T", 200-(myGLCD.getStringWidth("T F T")/2), 10);
  myGLCD.print("H I F I D U I N O", 200-(myGLCD.getStringWidth("H i F i D u i n o")/2), 55);
  myGLCD.print("v.2.01 STEREO", 200-(myGLCD.getStringWidth("v.2.01 STEREO")/2), 200);

#ifdef DUALMONO
  myGLCD.setFont(calibri_bold_40);
  myGLCD.setColor(0, 0, 255);
  myGLCD.print("DUALMONO", 200-(myGLCD.getStringWidth("DUALMONO")/2), 100);

#endif DUALMONO

#ifdef TPAPHASE
  myGLCD.setFont(calibri_bold_40);
  myGLCD.setColor(0, 0, 255);
  myGLCD.print("DUAL-TPA", 200-(myGLCD.getStringWidth("DUAL-TPA")/2), 100);

#endif TPAPHASE

  myGLCD.setBackColor(0, 0, 0);
  delay(1500);

  myGLCD.clrScr();

  bitSet(reg10,0);              // Set bit zero for reg 10: Mute DACs
  writeSabreReg(0x0A,reg10);    // Mute DACs. Earliest we can mute the DACs to avoid full vol
  setSabreVolume(currAttnu);    // Reg 0 to Reg 7 Set volume registers with startup volume level
  writeSabreReg(0x0D,0x00);     // DAC in-phase
  writeSabreReg(0x13,0x00);     // DACB anti-phase
  writeSabreReg(0x25,0x00);     // Use built in filter for stage 1 and stage 2
  writeSabreReg(0x0E,reg14);    // Reg 14: BuffII input map, trueDiff, normal IIR and Fast rolloff

#ifdef DUALMONO                // DAC registers default to stereo. Set to MONO L/R for dual MONO
  bitSet(reg17L,0);              // Set for MONO left channel. Right ch variable is already set for MONO
  writeSabreReg(0x11,reg17);     // Sets both chips to MONO
#endif DUALMONO

#ifdef TPAPHASE  
  /* The outputs on each side of each MONO board will be in opposite phase. In order to do this
   the phase of the odd dacs are out of phase with the even dacs. Further, buffalo is configured
   such that (In dual mono mode) the channel on the DAC which is opposite of the selected channel  
   carries the same analog signal but in anti-phase (odd dacs are the left channel;
   even dacs are the right channel)
   See http://hifiduino.wordpress.com/sabre32/ for further explaination
   */
  writeSabreLeftReg(0x0D,0x22);  // MONO LEFT DACx: odd dacs=in-phase; even dacs=anti-phase
  // writeSabreLeftReg(0x13,0x00);  // MONO LEFT DACBx: all dacs anti-phase with respect to DACx
  writeSabreRightReg(0x0D,0x11); // MONO RIGHT DACx: odd dacs=anti-phase; even dacs=in-phase
  // writeSabreRightReg(0x13,0x00); // MONO RIGHT DACBx: all dacs anti-phase with respect to DACx
#endif TPAPHASE
//serialUSB.println("StartDAC1 complete");
 
}

void startDac2(){
  myGLCD.setColor(255, 255, 255);
  myGLCD.fillRoundRect(0, 0, 399, 239);
  readSettings();
  setAndPrintInput(input, false);
  bitClear(reg10,0);                 // Clear bit zero of reg 10: unmute DACs
  writeSabreReg(0x0A,reg10);         // Unmute DACs
  //serialUSB.println("StartDAC2 complete");
   
}


/************************ MAIN PROGRAM ************************************************************/

void setup() {

  //serialUSB.begin(9600);          // for debugging

  irrecv.enableIRIn();            // Start the receiver
  
  // Set up MCP23008 pins
  mcp.begin();                    // use default address 0
  mcp.pinMode(0, OUTPUT);
  mcp.pinMode(1, OUTPUT);
  mcp.pinMode(2, OUTPUT);
  mcp.pinMode(3, OUTPUT);
  mcp.pinMode(4, OUTPUT);
  mcp.pinMode(5, OUTPUT);
  mcp.pinMode(6, OUTPUT);
  mcp.pinMode(7, OUTPUT);
  
  myGLCD.InitLCD();
  myGLCD.clrScr();
  myGLCD.setBackColor(0, 0, 0);

  // Set up the pin modes
  pinMode(VOLUPPIN, INPUT);       // Button switch or Encoder pin for volume up
  digitalWrite(VOLUPPIN, HIGH);   // If H/W debouncing is implemented, set to LOW

  pinMode(VOLDOWNPIN, INPUT);     // Button switch or Encoder pin for volume down
  digitalWrite(VOLDOWNPIN, HIGH); // If H/W debouncing is implemented, set to LOW

  pinMode(SELECTPIN, INPUT);      // Button switch or pin for encoder built-in switch 
  digitalWrite(SELECTPIN, HIGH);  // Enable pull-up resistor 

  pinMode(VOLUPPIN2, INPUT);       // Button switch or Encoder2 pin for volume up
  digitalWrite(VOLUPPIN2, HIGH);   // If H/W debouncing is implemented, set to LOW

  pinMode(VOLDOWNPIN2, INPUT);     // Button switch or Encoder2 pin for volume down
  digitalWrite(VOLDOWNPIN2, HIGH); // If H/W debouncing is implemented, set to LOW

  pinMode(SELECTPIN2, INPUT);      // Button switch or pin for Encoder2 built-in switch 
  digitalWrite(SELECTPIN2, HIGH);  // Enable pull-up resistor 


  pinMode(RECV_PIN, INPUT);      // Pin for IR sensor
  digitalWrite(RECV_PIN, HIGH);  // Enable pull-up resistor

  pinMode(SELECTI2SPIN, OUTPUT);  // Sidecar or IP_S control pin. 
  digitalWrite(SELECTI2SPIN, LOW);//  

  pinMode(AUX1PIN, OUTPUT);       //  
  digitalWrite(AUX1PIN, LOW);     //  

  pinMode(AUX2PIN, OUTPUT);       //  
  digitalWrite(AUX2PIN, LOW);     //  

  pinMode(POWERPIN, OUTPUT);      // Main power control pin. High output powers on the DAC. 
  digitalWrite(POWERPIN, LOW);    // By default, keep power off. 

  pinMode(DIMMPIN, OUTPUT);       // TFT backlight control pin. Low output powers on the backlight. 
  digitalWrite(DIMMPIN, HIGH);    // By default, keep backlight off. 

  #ifdef ALWAYSON
  digitalWrite(POWERPIN, HIGH);
  digitalWrite(DIMMPIN, LOW);
  poweron=true;
  Wire.begin();                   // Join the I2C bus as a master
  startDac1();
  delay(300);
  startDac2();
  delay(200);
  #endif ALWAYSON

  //serialUSB.println("Setup complete!");
   

  }  // End setup()


// Begin 24LC256 EEPROM section

void writeData(int device, unsigned int add, byte data) // writes a byte of data 'data' to the chip at I2C address 'device', in memory location 'add'
{
  Wire.beginTransmission(device);
  Wire.write((int)(add >> 8));   // left-part of pointer address
  Wire.write((int)(add & 0xFF)); // and the right
  Wire.write(data);
  Wire.endTransmission();
  delay(6);
  //serialUSB.println("Data written");
   
}

byte readData(int device, unsigned int add) // reads a byte of data from memory location 'add' in chip at I2C address 'device'
{
  byte result;  // returned value
  Wire.beginTransmission(device); //  these three lines set the pointer position in the EEPROM
  Wire.write((int)(add >> 8));   // left-part of pointer address
  Wire.write((int)(add & 0xFF)); // and the right
  Wire.endTransmission();
  Wire.requestFrom(device,1); // now get the byte of data...
  result = Wire.read();
  return result; // and return it as a result of the function readData
}

// End 24LC256 section

// -------------------------------------loop section------------------------------------------

void loop() {

  // Print the sample rate, input and lock (once every "INTERVAL_SAMPLE" time)
  if((millis() - displayMillis > INTERVAL_SAMPLE*1000)&&!selectMode&&poweron) {
    displayMillis = millis(); // Saving last time we display sample rate
    Status=readStatus();  // Read status register
    if(Status&B00000100) SPDIFValid=true; // Determine if valid spdif data
    else SPDIFValid=false;                // SR calculation depends on I2S or SPDIf

    // Update the sample rate display if there is a lock (otherwise, SR=0)
    myGLCD.setFont(SRFONT);
    myGLCD.setColor(0, 0, 0);
    if(Status&B00000001)
    {        // There is a lock in a signal
      sr = sampleRate();
      if(Status&B00001000)      // Determines if DSD
      {
        sr*=64;                 // It is DSD, the sample rate is 64x
        if (leandisp=true)
        {
          if (inputtype!=1) 
          {
          myGLCD.setColor(255, 255, 255);
          myGLCD.fillRoundRect(215, 100, 340, 180);
          myGLCD.setColor(0, 0, 0);
          myGLCD.drawBitmap(215, 110, 120, 59, dsd);
          srold = 0;
          }
          inputtype = 1;        // Input type is DSD.
        }
        else
        {
          myGLCD.print("DSD    ", 220, 215);
          inputtype = 1;        // Input type is DSD.
          srold = 0;
        }
        if(SRExact==true && (sr>(srold+50)||sr<(srold-50)))
        {
          myGLCD.setColor(255, 255, 255);
          myGLCD.fillRect(190, 181, 399, 239);
          myGLCD.setColor(0, 0, 0);
          myGLCD.setFont(Ubuntubold);
          myGLCD.printNumI(sr, 275-(myGLCD.getStringWidth("0000000")/2), 185);    // Print DSD sample rate in exact format
          myGLCD.setFont(SRFONT);
        }
        else                                 // Print DSD sample rate in nominal format
        if(sr>(srold+50)||sr<(srold-50))
        {
        myGLCD.setColor(255, 255, 255);
        myGLCD.fillRect(190, 181, 399, 239);
        myGLCD.setColor(0, 0, 0);
        if(sr>6300000)
          myGLCD.print("Inv. DSD", 275-(myGLCD.getStringWidth("Inv. DSD")/2), 185);
        else
          if(sr>6143000)
            myGLCD.print("6.1 MHz", 275-(myGLCD.getStringWidth("6.1 MHz")/2), 185);
          else
            if(sr>5644000)
              myGLCD.print("5.6 MHz", 275-(myGLCD.getStringWidth("5.6 MHz")/2), 185);
            else
              if(sr>3071000)
                myGLCD.print("3.0 MHz", 275-(myGLCD.getStringWidth("3.0 MHz")/2), 185);
              else
                if(sr>2822000)
                  myGLCD.print("2.8 MHz", 275-(myGLCD.getStringWidth("2.8 MHz")/2), 185);
                else
                  myGLCD.print("Unknown", 275-(myGLCD.getStringWidth("Unknown")/2), 185);
        }
      }
      else 
      {                       // If not DSD then it is I2S or SPDIF
        if(SPDIFValid)        // If valid spdif data, then it is spdif
        {
        if (leandisp=true)
        {
          if (inputtype!=3) 
          {
          myGLCD.setColor(255, 255, 255);
          myGLCD.fillRoundRect(215, 100, 340, 180);
          myGLCD.setColor(0, 0, 0);
          myGLCD.drawBitmap(215, 120, 120, 43, spdif);
          srold = 0;
          }
          inputtype = 3;       // Input type is S/PDIF.
        }
        else
        {
          myGLCD.print("S/PDIF ", 220, 215);
          inputtype = 3;       // Input type is S/PDIF.
          srold = 0;
        }
        if(SRExact==true && (sr>(srold+50)||sr<(srold-50)))        // Print PCM sample rate in exact format
          {
            myGLCD.setColor(255, 255, 255);
            myGLCD.fillRect(190, 181, 399, 239);
            myGLCD.setColor(0, 0, 0);
            myGLCD.setFont(Ubuntubold);
            myGLCD.printNumI(sr, 275-(myGLCD.getStringWidth("000000")/2), 185);
            myGLCD.setFont(SRFONT);
          }
          else                     // Print PCM sample rate in nominal format
          if (sr>(srold+50)||sr<(srold-50))
          {
            myGLCD.setColor(255, 255, 255);
            myGLCD.fillRect(190, 181, 399, 239);
            myGLCD.setColor(0, 0, 0);
            if(sr>192900)
              myGLCD.print("Inval. SR", 275-(myGLCD.getStringWidth("Inval. SR")/2), 185);
            else
              if(sr>191900)
                myGLCD.print("192 KHz", 275-(myGLCD.getStringWidth("192 KHz")/2), 185);
              else
                if(sr>176300)
                  myGLCD.print("176 KHz", 275-(myGLCD.getStringWidth("176 KHz")/2), 185);
                else
                  if(sr>95900)
                    myGLCD.print("96 KHz", 275-(myGLCD.getStringWidth("96 KHz")/2), 185);
                  else
                    if(sr>88100)
                      myGLCD.print("88 KHz", 275-(myGLCD.getStringWidth("88 KHz")/2), 185);
                    else
                      if(sr>47900)
                        myGLCD.print("48 KHz", 275-(myGLCD.getStringWidth("48 KHz")/2), 185);
                      else
                        myGLCD.print("44.1 KHz", 275-(myGLCD.getStringWidth("44.1 KHz")/2), 185);
             }
          }

        else {
        if (leandisp=true)
        {
          if (inputtype!=2) {
          myGLCD.setColor(255, 255, 255);
          myGLCD.fillRoundRect(215, 100, 340, 180);
          myGLCD.setColor(0, 0, 0);
          myGLCD.drawBitmap(230, 100, 89, 77, pcm);
          srold = 0;
          }
          inputtype = 2;                                          // Input type is PCM.
        }
        else
        {
          myGLCD.print("PCM    ", 220, 215);                     // Otherwise it is PCM
          inputtype = 2;                                         // Input type is PCM.
          srold = 0;
        }
          if(SRExact==true && (sr>(srold+50)||sr<(srold-50)))    // Print PCM sample rate in exact format
          {        
            myGLCD.setColor(255, 255, 255);
            myGLCD.fillRect(190, 181, 399, 239);
            myGLCD.setColor(0, 0, 0);
            myGLCD.setFont(Ubuntubold);
            myGLCD.printNumI(sr, 275-(myGLCD.getStringWidth("000000")/2), 185);
            myGLCD.setFont(SRFONT);
          }
          else                                                  // Print PCM sample rate in nominal format
          if (sr>(srold+50)||sr<(srold-50))
          {
            myGLCD.setColor(255, 255, 255);
            myGLCD.fillRect(190, 181, 399, 239);
            myGLCD.setColor(0, 0, 0);
            if(sr>385000)
                myGLCD.print("Inv. PCM", 275-(myGLCD.getStringWidth("Inv. PCM")/2), 185);
            else
              if(sr>383900)
                myGLCD.print("384 KHz", 275-(myGLCD.getStringWidth("384 KHz")/2), 185);
              else
                if(sr>352700)
                  myGLCD.print("352 KHz", 275-(myGLCD.getStringWidth("352 KHz")/2), 185);
                else
                  if(sr>191900)
                    myGLCD.print("192 KHz", 275-(myGLCD.getStringWidth("192 KHz")/2), 185);
                  else
                    if(sr>176300)
                      myGLCD.print("176 KHz", 275-(myGLCD.getStringWidth("176 KHz")/2), 185);
                    else
                      if(sr>95900)
                        myGLCD.print("96 KHz", 275-(myGLCD.getStringWidth("96 KHz")/2), 185);
                      else
                        if(sr>88100)
                          myGLCD.print("88 KHz", 275-(myGLCD.getStringWidth("88 KHz")/2), 185);
                        else
                          if(sr>47900)
                            myGLCD.print("48 KHz", 275-(myGLCD.getStringWidth("48 KHz")/2), 185);
                          else
                            if(sr>44000)
                              myGLCD.print("44.1 KHz", 275-(myGLCD.getStringWidth("44.1 KHz")/2), 185);
                            else
                              myGLCD.print("32 KHz", 275-(myGLCD.getStringWidth("32 KHz")/2), 185);
          }
        }
      }
    }
    else {                                                 // There is no lock we print input selection
      if (leandisp=true)
      {
          if (inputtype!=4) {
          myGLCD.setColor(255, 255, 255);
          myGLCD.fillRoundRect(200, 100, 399, 239);
          myGLCD.setColor(0, 0, 0);
          myGLCD.drawBitmap(215, 100, 120, 120, nolock);
          }
          inputtype = 4;                                  // Input type is No Lock.
          srold = 0;
      }
      else
      {
      myGLCD.setFont(SRFONT);
      myGLCD.setColor(255, 0, 0);
      myGLCD.print("No Lock     ", 200, 185);
      myGLCD.print("            ", 220, 215);
      inputtype = 4;                                      // Input type is No Lock.
      srold = 0;
      }
    }

     // Heatbeat to show that the software is still running...

    myGLCD.setFont(SmallFont);
    myGLCD.setColor(255, 0, 0);
    if(pulse++%2){
      myGLCD.print("d", 390, 0);
    }
    else {
      myGLCD.print("p", 390, 0);
    }
  srold = sr;
  }


// New IR code!!

// Process the IR input, if any
  if (irrecv.decode(&results)) {
    if (results.value == POWER_CODE) {
        //serialUSB.write("Power\n");
        if (poweron==false)                              // Check if already powered on
      {
        digitalWrite(POWERPIN, HIGH);
        digitalWrite(DIMMPIN, LOW);
        poweron=true;
        Wire.begin();                                   // Join the I2C bus as a master
        startDac1();
        delay(300);
        startDac2();
        delay(200);
      }
      else {
        digitalWrite(POWERPIN, LOW);
        digitalWrite(DIMMPIN, HIGH);
        digitalWrite(SELECTI2SPIN, LOW);
        poweron=false;
        myGLCD.clrScr();
      }
    } 
    else if (results.value == VOLUP_CODE) {
      //serialUSB.write("Volume Up\n");
      if (currAttnu>MINATTNU && poweron)               // Check if not already at minimum attenuation
      {
        if(dimmed) {
          rampUp();                                    // Disengage dim
          dimmed=false;
        }

        currAttnu-=2;                                 // Decrease attenuation 1 dB (increase volume 1 db)
        setSabreVolume(currAttnu);                    // Write value into registers
        dispVol(currAttnu);
        prev_result = VOLUP_CODE;
      }
     } 
    else if (results.value == VOLDOWN_CODE) {
      //serialUSB.write("Volume Down\n");
         if (currAttnu<MAXATTNU && poweron)           // Check if not already at maximum attenuation
      {
        if(dimmed) {
          rampUp();                        // Disengage dim
          dimmed=false;
        }

        currAttnu+=2;                      // Increase 1 dB attenuation (decrease volume 1 db)
        setSabreVolume(currAttnu);         // Write value into registers
        dispVol(currAttnu);
        prev_result = VOLDOWN_CODE;
      }
    } 
    else if (results.value == MUTE_CODE) {
      //serialUSB.write("Mute\n");
        if(dimmed && poweron){             // Back to previous volume level 1 db at a time
        rampUp();                          // Disengage dim
        dimmed=false;
      }
      else {
        if(DIM>=currAttnu && poweron) {    // only DIM if current attenuation is lower
          setSabreVolume(DIM);             // Dim volume
          dispVol(DIM);
          dimmed=true;
        }
      }
      prev_result = 0;
    } 
    else if (results.value == SELECT_CODE) {
      //serialUSB.write("Select\n");
      if(poweron) 
      {
        if(selectMode==false)
      {
        myGLCD.setColor(255, 255, 255);
        myGLCD.fillRoundRect(0, 0, 399, 239);
      }  
      selectMode=true;          // Now we are in select mode
      
      if (leandisp=true)
      {
        leandisp=false;
      //setAndPrintInput(input, false);
      }
      debounceMillis=millis();  // Start debounce timer
      selectMillis=millis();    // Start being-in-select-mode timer
      select++;                 // Move to next select option
      myGLCD.setColor(255, 255, 0);

      switch(select%(MAXPARAM+1)){
      case VOL:
        break;
      case INP:
        setAndPrintSRFormat(settings[input][SRFVAL]%SRFCHO, false);
        setAndPrintInput(input, true);
        break;
      case INF:
        setAndPrintInput(input, false);
        setAndPrintInputFormat(settings[input][FORMATVAL]%FORMATCHO, true);
        break;
      case FIR:
        setAndPrintInputFormat(settings[input][FORMATVAL]%FORMATCHO, false);
        setAndPrintFirFilter(settings[input][FIRVAL]%FIRCHO, true);
        break;
      case IIR:
        setAndPrintFirFilter(settings[input][FIRVAL]%FIRCHO, false);
        setAndPrintIirFilter(settings[input][FIRVAL]%FIRCHO, true);
        break;
      case DPL:
        setAndPrintIirFilter(settings[input][FIRVAL]%FIRCHO, false);
        setAndPrintDPLL(settings[input][DPLLVAL]%DPLLCHO, true);
        break;
      case QTZ:
        setAndPrintDPLL(settings[input][DPLLVAL]%DPLLCHO, false);
        setAndPrintQuantizer(settings[input][QUANVAL]%QUANCHO, true);
        break;
      case NCH:
        setAndPrintQuantizer(settings[input][QUANVAL]%QUANCHO, false);
        setAndPrintNotch(settings[input][NOTCHVAL]%NOTCHCHO, true);
        break;
      case PLM:
        setAndPrintNotch(settings[input][NOTCHVAL]%NOTCHCHO, false);
        setAndPrintDPLLMode(settings[input][PLMVAL]%PLMCHO, true);
        break;
      case OSF:
        setAndPrintDPLLMode(settings[input][PLMVAL]%PLMCHO, false);
        setAndPrintBypassOSF(settings[input][OSFVAL]%OSFCHO, true);
        break;
      case SRF:
        setAndPrintBypassOSF(settings[input][OSFVAL]%OSFCHO, false);
        setAndPrintSRFormat(settings[input][SRFVAL]%SRFCHO, true);
        break;
      }  // End of switch
      
      }
      prev_result = 0;
    } 
    else if (results.value == LEFT_CODE) {
      //serialUSB.write("Left\n");
      if(poweron) {
      irdir=1;    
      }
      prev_result = 0;
    } 
    else if (results.value == RIGHT_CODE) {
      //serialUSB.write("Right\n");
      if(poweron) {
      irdir=2;  
      }
      prev_result = 0;
    } 
    else if (results.value == SOURCE1_CODE) {
      //serialUSB.write("Source 1\n");
      if(poweron) {
        input=0;
        inputtype = 0;
        input%=ICHO;
        setAndPrintInput(input, false);
      }
      prev_result = 0;
    } 
    else if (results.value == SOURCE2_CODE) {
      //serialUSB.write("Source 2\n");
      if(poweron) {
        input=1;
        inputtype = 0;
        input%=ICHO;
        setAndPrintInput(input, false);
      }
      prev_result = 0;
    } 
    else if (results.value == SOURCE3_CODE) {
      //serialUSB.write("Source 3\n");
      if(poweron) {
        input=2;
        inputtype = 0;
        input%=ICHO;
        setAndPrintInput(input, false);
      }
      prev_result = 0;
    } 
    else if (results.value == SOURCE4_CODE) {
      //serialUSB.write("Source 4\n");
      if(poweron) {
        input=3;
        inputtype = 0;
        input%=ICHO;
        setAndPrintInput(input, false);
      }
      prev_result = 0;
    } 
    else if (results.value == SOURCE5_CODE) {
      //serialUSB.write("Source 5\n");
      if(poweron) {
        input=4;
        inputtype = 0;
        input%=ICHO;
        setAndPrintInput(input, false);
      }
      prev_result = 0;
    } 
    else if (results.value == SOURCE6_CODE) {
      //serialUSB.write("Source 6\n");
      if(poweron) {
        input=5;
        inputtype = 0;
        input%=ICHO;
        setAndPrintInput(input, false);
      }
      prev_result = 0;
    } 
    else if (results.value == SOURCE7_CODE) {
      //serialUSB.write("Source 7\n");
      if(poweron) {
        input=6;
        inputtype = 0;
        input%=ICHO;
        setAndPrintInput(input, false);
      }
      prev_result = 0;
    } 
    else if (results.value == SOURCE8_CODE) {
      //serialUSB.write("Source 8\n");
      if(poweron) {
        input=7;
        inputtype = 0;
        input%=ICHO;
        setAndPrintInput(input, false);
      }
      prev_result = 0;
    } 
    else if (results.value == SOURCE9_CODE) {
      //serialUSB.write("Source 9\n");
      if(poweron) {
        input=8;
        inputtype = 0;
        input%=ICHO;
        setAndPrintInput(input, false);
      }
      prev_result = 0;
    } 
    else if (results.value == SOURCE10_CODE) {
      //serialUSB.write("Source 10\n");
      if(poweron) {
        input=9;
        inputtype = 0;
        input%=ICHO;
        setAndPrintInput(input, false);
      }
      prev_result = 0;
    } 
    else if (results.value == 0xFFFFFFFF) {
    //serialUSB.write("Repeat\n");
    //serialUSB.println(prev_result);
    switch(prev_result){
      case VOLUP_CODE:
        //serialUSB.write("Volume Up\n");
        if (currAttnu>MINATTNU && poweron)                // Check if not already at minimum attenuation
        {
        if(dimmed) {
          rampUp();                          // Disengage dim
          dimmed=false;
        }
        currAttnu-=2;                      // Decrease attenuation 1 dB (increase volume 1 db)
        setSabreVolume(currAttnu);         // Write value into registers
        dispVol(currAttnu);
        prev_result == 0;
      }
      break;
     
    case VOLDOWN_CODE:
      //serialUSB.write("Volume Down\n");
         if (currAttnu<MAXATTNU && poweron)              // Check if not already at maximum attenuation
        {
        if(dimmed) {
          rampUp();                        // Disengage dim
          dimmed=false;
        }
        currAttnu+=2;                      // Increase 1 dB attenuation (decrease volume 1 db)
        setSabreVolume(currAttnu);         // Write value into registers

        dispVol(currAttnu);
        prev_result == 0;
        }
        break;
      }
    }
  
    else {
      //serialUSB.print("unexpected value: ");
      //serialUSB.println(results.value, HEX);
      prev_result = 0;
    }
    irrecv.resume(); // Resume decoding (necessary!)
  }

  /*
  To debounce the switch, we detect the first instance of the "click" and then ignore
   the switch: first during the switch "bouncing" time and then while the switch remains
   pressed because one cannot lift the finger so fast.
   We want to register a single "click" per push of the switch
   */

  if((digitalRead(SELECTPIN)==LOW)&&((millis()-debounceMillis)>INTERVAL_SWITCHBOUNCE)) {

    if (poweron==false)                // Check if already powered on
    {
      digitalWrite(POWERPIN, HIGH);
      digitalWrite(DIMMPIN, LOW);
      poweron=true;
      startDac1();
      startDac2();        
    }
    else {
      selectMode=true;          // Now we are in select mode
      if (leandisp=true)
      {
        myGLCD.clrScr();                      
        myGLCD.setColor(255, 255, 255);
        myGLCD.fillRoundRect(0, 0, 399, 239);
        leandisp=false;
        setAndPrintInput(input, false);
      }
      debounceMillis=millis();  // Start debounce timer
      selectMillis=millis();    // Start being-in-select-mode timer

      select++;  // Move to next select option
      myGLCD.setColor(255, 255, 0);

      switch(select%(MAXPARAM+1)){
      case VOL:
        break;
      case INP:
        setAndPrintSRFormat(settings[input][SRFVAL]%SRFCHO, false);
        setAndPrintInput(input, true);
        break;
      case INF:
        setAndPrintInput(input, false);
        setAndPrintInputFormat(settings[input][FORMATVAL]%FORMATCHO, true);
        break;
      case FIR:
        setAndPrintInputFormat(settings[input][FORMATVAL]%FORMATCHO, false);
        setAndPrintFirFilter(settings[input][FIRVAL]%FIRCHO, true);
        break;
      case IIR:
        setAndPrintFirFilter(settings[input][FIRVAL]%FIRCHO, false);
        setAndPrintIirFilter(settings[input][FIRVAL]%FIRCHO, true);
        break;
      case DPL:
        setAndPrintIirFilter(settings[input][FIRVAL]%FIRCHO, false);
        setAndPrintDPLL(settings[input][DPLLVAL]%DPLLCHO, true);
        break;
      case QTZ:
        setAndPrintDPLL(settings[input][DPLLVAL]%DPLLCHO, false);
        setAndPrintQuantizer(settings[input][QUANVAL]%QUANCHO, true);
        break;
      case NCH:
        setAndPrintQuantizer(settings[input][QUANVAL]%QUANCHO, false);
        setAndPrintNotch(settings[input][NOTCHVAL]%NOTCHCHO, true);
        break;
      case PLM:
        setAndPrintNotch(settings[input][NOTCHVAL]%NOTCHCHO, false);
        setAndPrintDPLLMode(settings[input][PLMVAL]%PLMCHO, true);
        break;
      case OSF:
        setAndPrintDPLLMode(settings[input][PLMVAL]%PLMCHO, false);
        setAndPrintBypassOSF(settings[input][OSFVAL]%OSFCHO, true);
        break;
      case SRF:
        setAndPrintBypassOSF(settings[input][OSFVAL]%OSFCHO, false);
        setAndPrintSRFormat(settings[input][SRFVAL]%SRFCHO, true);
        break;
      }  // End of switch
    }
  }

  // For the rotary encoder and the remote control. The control depends whether in display mode or select mode
  
  int dir=RotEncoder();
  
  if(dir==1 || dir==2 || irdir==1 || irdir==2)
  {
    delay(INTERVAL_BOUNCE);  // debounce by waiting INTERVAL_BOUNCE time

    switch(select%(MAXPARAM+1)){
    case VOL:  // Volume adjustment
      dimmed=false;                          // Disengage dim
      if (dir==1 || irdir==1)  // CCW
      {
        if (currAttnu<MAXATTNU && poweron)              // Check if not already at maximum attenuation
        {
          currAttnu+=2;                      // Increase 1 dB attenuation (decrease volume 1 db)
          setSabreVolume(currAttnu);         // Write value into registers
          dispVol(currAttnu);
          irdir=0;
        }
      }
      else                                   // If not CCW, then it is CW
      {
        if (currAttnu>MINATTNU && poweron)              // Check if not already at minimum attenuation
        {
          currAttnu-=2;                      // Decrease attenuation 1 dB (increase volume 1 db)
          setSabreVolume(currAttnu);         // Write value into registers
          dispVol(currAttnu);
          irdir=0;
        }
      }
      break;
    case INP:                      // Input selection
      if (dir==1 || irdir==1) 
      {input--;                    // CCW
      irdir=0;}
      else input++;
      input%=ICHO;
      setAndPrintSRFormat(settings[input][SRFVAL]%SRFCHO, false);
      setAndPrintInput(input, true);
      selectMillis=millis();       // Reset being-in-select-mode timer
      irdir=0;
      break;
    case INF:                      // Input format selection
      if (dir==1 || irdir==1) {
      settings[input][FORMATVAL]--;// CCW      
      irdir=0; }
      else settings[input][FORMATVAL]++;
      setAndPrintInput(input, false);
      setAndPrintInputFormat(settings[input][FORMATVAL]%FORMATCHO, true);
      selectMillis=millis();      // Reset being-in-select-mode timer
      irdir=0;
      break;
    case FIR:                     // FIR filter selection
      if (dir==1 || irdir==1) {
      settings[input][FIRVAL]--;  // CCW
      irdir=0; }
      else settings[input][FIRVAL]++;
      setAndPrintInputFormat(settings[input][FORMATVAL]%FORMATCHO, false);
      setAndPrintFirFilter(settings[input][FIRVAL]%FIRCHO, true); 
      selectMillis=millis();      // Update being-in-select-mode timer
      irdir=0;
      break;
    case IIR:                     // IIR filter selection
      if (dir==1 || irdir==1) {
      settings[input][IIRVAL]--;  // CCW
      irdir=0;  }      
      else settings[input][IIRVAL]++;
      setAndPrintFirFilter(settings[input][FIRVAL]%FIRCHO, false);
      setAndPrintIirFilter(settings[input][IIRVAL]%IIRCHO, true);
      selectMillis=millis();     // Update being-in-select-mode timer
      irdir=0;
      break;
    case DPL:                    // DPLL setting: separate selection for SPDIF and I2S
      if (dir==1 || irdir==1) {      
      settings[input][DPLLVAL]--;  // CCW
      irdir=0;  }
      else settings[input][DPLLVAL]++;
      setAndPrintIirFilter(settings[input][FIRVAL]%FIRCHO, false);
      setAndPrintDPLL(settings[input][DPLLVAL]%DPLLCHO, true);
      selectMillis=millis();      // Reset being-in-select-mode timer
      irdir=0;
      break;
    case QTZ:                      // Quantizer selection
      if (dir==1 || irdir==1) {
      settings[input][QUANVAL]--;  // CCW
      irdir=0; }
      else settings[input][QUANVAL]++;
      setAndPrintDPLL(settings[input][DPLLVAL]%DPLLCHO, false);
      setAndPrintQuantizer(settings[input][QUANVAL]%QUANCHO, true);
      selectMillis=millis();      // Reset being-in-select-mode timer
      irdir=0;
      break;
    case NCH:                      // Notch selection
      if (dir==1 || irdir==1) {
      settings[input][NOTCHVAL]--; // CCW
      irdir=0;  }
      else settings[input][NOTCHVAL]++;
      setAndPrintQuantizer(settings[input][QUANVAL]%QUANCHO, false);
      setAndPrintNotch(settings[input][NOTCHVAL]%NOTCHCHO, true);
      selectMillis=millis();  // Reset being-in-select-mode timer
      irdir=0;
      break;
    case PLM:  // DPLL mode selection
      if (dir==1 || irdir==1) {
      settings[input][PLMVAL]--;  // CCW
      irdir=0;  }
      else settings[input][PLMVAL]++;
      setAndPrintNotch(settings[input][NOTCHVAL]%NOTCHCHO, false);
      setAndPrintDPLLMode(settings[input][PLMVAL]%PLMCHO, true);
      selectMillis=millis();  // Reset being-in-select-mode timer
      irdir=0;
      break;
    case OSF:  //OSF Bypass -Since this is a toggle, we just call the function
      if (dir==1 || irdir==1) {
      settings[input][OSFVAL]--;  // CCW
      irdir=0;  }
      else settings[input][OSFVAL]++;
      setAndPrintDPLLMode(settings[input][PLMVAL]%PLMCHO, false);
      setAndPrintBypassOSF(settings[input][OSFVAL]%OSFCHO, true);
      selectMillis=millis();  // Reset being-in-select-mode timer
      irdir=0;
      break;
    case SRF:  //Sampler Rate format -Since this is a toggle, we just call the function
      if (dir==1 || irdir==1) {
      settings[input][SRFVAL]--;  // CCW
      irdir=0;  }
      else settings[input][SRFVAL]++;
      setAndPrintBypassOSF(settings[input][OSFVAL]%OSFCHO, false);
      setAndPrintSRFormat(settings[input][SRFVAL]%SRFCHO, true);
      selectMillis=millis();  // Reset being-in-select-mode timer
      irdir=0;
      break;

    }  // End of (rotary encoder) switch

  }  // End of if(rotating)

  // When the being-in-select mode timer expires, we revert to volume/display mode
  if(selectMode&&millis()-selectMillis > INTERVAL_SELECT*1000){
    selectMode=false;                        // No longer in select mode
    leandisp=true;                           // Go to lean display mode
    inputtype = 0;                           // Input type is undefined.
    myGLCD.setColor(255, 255, 255);          
    myGLCD.fillRoundRect(0, 0, 399, 239);    // Draw white background.
    setAndPrintInput(input, false);          // Draw the screen
    dispVol(currAttnu);                      // Display current volume level
    select=VOL;                              // Back to volume mode
    writeSettings();                         // Write new settings into EEPROM, including current input
  } 
} // End of loop()


// Begin new rotary encoder code (no longer using interrupts)

int state0[]={0,0,0,0};
int RotEncoder() {
  int encc = (digitalRead(VOLUPPIN) << 1) + digitalRead(VOLDOWNPIN);
  int out=4;

  int state1[]={1,0,2,3};
  int state2[]={0,2,3,1};
  int state3[]={2,3,1,0};
  int state4[]={3,1,0,2};
  int state5[]={0,1,3,2};
  int state6[]={1,3,2,0};
  int state7[]={3,2,0,1};
  int state8[]={2,0,1,3};

  if (state0[3] != encc) {
    for (int i=0;i<4;i++) {
      if (i<3) state0[i]=state0[i+1];
      else state0[i]=encc;
    }
    
    if ((state1[0] == state0[0] && state1[1] == state0[1] && state1[2] == state0[2] && state1[3] == state0[3])||
        (state2[0] == state0[0] && state2[1] == state0[1] && state2[2] == state0[2] && state2[3] == state0[3])||
        (state3[0] == state0[0] && state3[1] == state0[1] && state3[2] == state0[2] && state3[3] == state0[3])||
        (state4[0] == state0[0] && state4[1] == state0[1] && state4[2] == state0[2] && state4[3] == state0[3])) 
    out = 1;
    else if ((state5[0] == state0[0] && state5[1] == state0[1] && state5[2] == state0[2] && state5[3] == state0[3])||
             (state6[0] == state0[0] && state6[1] == state0[1] && state6[2] == state0[2] && state6[3] == state0[3])||
             (state7[0] == state0[0] && state7[1] == state0[1] && state7[2] == state0[2] && state7[3] == state0[3])||
             (state8[0] == state0[0] && state8[1] == state0[1] && state8[2] == state0[2] && state8[3] == state0[3])) 
    out = 2;
    else
    out = 3;
  }
  return out;
}

int RotEncoder2() {
  int encc = (digitalRead(VOLUPPIN2) << 1) + digitalRead(VOLDOWNPIN2);
  int out=4;

  int state1[]={1,0,2,3};
  int state2[]={0,2,3,1};
  int state3[]={2,3,1,0};
  int state4[]={3,1,0,2};
  int state5[]={0,1,3,2};
  int state6[]={1,3,2,0};
  int state7[]={3,2,0,1};
  int state8[]={2,0,1,3};

  if (state0[3] != encc) {
    for (int i=0;i<4;i++) {
      if (i<3) state0[i]=state0[i+1];
      else state0[i]=encc;
    }
    
    if ((state1[0] == state0[0] && state1[1] == state0[1] && state1[2] == state0[2] && state1[3] == state0[3])||
        (state2[0] == state0[0] && state2[1] == state0[1] && state2[2] == state0[2] && state2[3] == state0[3])||
        (state3[0] == state0[0] && state3[1] == state0[1] && state3[2] == state0[2] && state3[3] == state0[3])||
        (state4[0] == state0[0] && state4[1] == state0[1] && state4[2] == state0[2] && state4[3] == state0[3])) 
    out = 1;
    else if ((state5[0] == state0[0] && state5[1] == state0[1] && state5[2] == state0[2] && state5[3] == state0[3])||
             (state6[0] == state0[0] && state6[1] == state0[1] && state6[2] == state0[2] && state6[3] == state0[3])||
             (state7[0] == state0[0] && state7[1] == state0[1] && state7[2] == state0[2] && state7[3] == state0[3])||
             (state8[0] == state0[0] && state8[1] == state0[1] && state8[2] == state0[2] && state8[3] == state0[3])) 
    out = 2;
    else
    out = 3;
  }
  return out;
}









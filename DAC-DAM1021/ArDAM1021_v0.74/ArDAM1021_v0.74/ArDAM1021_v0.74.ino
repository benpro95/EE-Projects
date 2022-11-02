/*********************************************************************************************************
 * ArDAM1021: Inspired by my adaptation of the HiFiDUINO code so that is displays on a colour TFT.
 *
 * Project Page: http://www.dimdim.gr/arduino/ardam1021-project/
 *
 * v0.74    19/08/15 : - Changed the rotary encoder code. Now it works exactly as it should.
 *                     - Various serious bugs fixed (vol control etc).
 *                     - New status: Beta
 *
 * v0.63    19/08/15 : - Added support for white text on black background. Added the necessary graphics.
 *
 * v0.58    18/08/15 : - Added code to support selection of the 4 available filters (via remote or rot/enc).
 *                     - Changed the number of inputs to the built-in 3 (plus Auto).
 *                     - Some code cleanup. Still very Alpha..
 *                     
 * v0.50    31/07/15 : - Code cleanup. Still in Alpha condition.. Preview release on dimdim.gr.
 * 
 * v0.30    11/02/15 : - Added code to select inputs. Last choice is the Auto Detection function.
 *
 * v0.25    08/02/15 : - Added code to read from Serial and display sampling rate.
 *
 * v0.21    05/02/15 : - Tested with actual DAC. Confirmed proper operation of volume control.
 *
 * v0.20    25/01/15 : - Serial out through Serial3 implemented. Outputs volume control codes. 
 *                       Not tested with actual DAC.
 *
 * v0.10    25/01/15 : - Initial codebase (based on TFT HiFiDuino). Not much actual functionality included.
 *
 ***********************************************************************************************************/


/***********************************************************************************************************
 * Code starts here
 ***********************************************************************************************************/

// LIBRARIES
#include <Wire.h>                 // For I2C
#include "Adafruit_MCP23008.h"    // For the I2C port expander
#include <UTFT.h>                 // For the UTFT library
#include <UTFT_DLB.h>             // UTFT library addon for proportional fonts
#include <IRremote2.h>            // DUE-compatible IRremote library
#include <RotaryEncoder.h>        // Library for the encoders

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
extern unsigned short automatic[0x3840];

extern unsigned short usb_b[0x3840];
extern unsigned short toslink_b[0x3840];
extern unsigned short bnc_b[0x3840];
extern unsigned short dsd_b[0x1BA8];
extern unsigned short pcm_b[0x1AC5];
extern unsigned short spdif_b[0x1428];
extern unsigned short nolock_b[0x3840];
extern unsigned short xlr_b[0x3840];
extern unsigned short automatic_b[0x3840];


/******************* Code Customization Section *********************/


/* Optionally choose whether you will be using the remote power on/off functionality.*/
/* Comment out if you will be using the remote for on/off.*/
#define ALWAYSON

/* Optionally choose 0db as the default (on power-on) volume level.*/
/* Comment out if you like the -50db setting.*/
//#define FULLVOL

/* Choose the number of inputs. This should be "the actual number of inputs + 1" since the last input is always the AUTO input.*/
#define ICHO 4

/* Optionally change the name of the inputs. They all have to take up the same length on the display. 
   If you decide to change the names, some trial and error with the blanks may be required since the font is proportional. */

  char no0[] = "COAX  ";
  char no1[] = "OPT  ";
  char no2[] = "USB  ";
  char no3[] = "AUTO";

// Set whether you want black characters on white background (default) or white characters on black background (#define WHITE).
#define WHITE

/********************************************************************/

#define DEFAULTATTNU 20   //-20 dB

#ifdef FULLVOL
  #define DEFAULTATTNU 0  //-0 dB
#endif FULLVOL

#define MAXATTNU 99       //-99dB
#define MINATTNU 0        //-0 dB
#define DIM 60            //-60 dB Dim volume
#define RAMP 10           // Additional msec delay per 1 db for ramping up to volume after dim

int VOLUPPIN=6;           // RotEnc A terminal for right rotary encoder.
int VOLDOWNPIN=7;         // RotEnc B terminal for right rotary encoder.
#define SELECTPIN 5       // Switch to select function for right rotary encoder.

int VOLUPPIN2=A4;         // RotEnc A terminal for left rotary encoder.
int VOLDOWNPIN2=A3;       // RotEnc B terminal for left rotary encoder.
#define SELECTPIN2 A5     // Switch to select function for left rotary encoder.

#define SETTINGFONT calibri_bold_26    // Font for displaying various settings
#define SRFONT calibri_bold_40         // Font for displaying Sampling Rate

int irdir=0;

// Remote control definitions section
int RECV_PIN = 9;                // Pin for IR receiver (remote control)
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
#define FILT1_CODE 0xFFB04F      // Code for Filter 1 (my remote: RETURN)
#define FILT2_CODE 0xFFB847      // Code for Filter 2 (my remote: L/R)
#define FILT3_CODE 0xFFA857      // Code for Filter 3 (my remote: STEP)
#define FILT4_CODE 0xFF58A7      // Code for Filter 4 (my remote: SLOW)


#define AUX1PIN 3            // Pin #1 for controlling other things - only applicable to original TFT HiFiDuino Shield, not to Universal Isolator Shield
#define AUX2PIN A7           // Pin #2 for controlling other things - only applicable to original TFT HiFiDuino Shield, not to Universal Isolator Shield

#define POWERPIN A0          // Pin for main power activation
#define DIMMPIN 8            // Pin for TFT backlight controll. A LOW on this pin turns on the backlight.

#define INTERVAL_SAMPLE 1     // Time interval in SECONDS for refreshing the sample rate
#define INTERVAL_BOUNCE 2     // Time in milliseconds to debounce the rotary encoder
#define INTERVAL_SWITCHBOUNCE 200  // Time in milliseconds to debounce switch
#define INTERVAL_SELECT 4     // Time in sec to exit select mode when no activity

#define VOL 0                 // The order of selection when clicking the select switch
#define INP 1                 // Input selection
#define FILT 2                // Filter selection

// Order of settings in the array for each input
#define FILTVAL 0

/* Total number of parameters to keep track for each input. The one listed above plus the input
 choice. This makes 2.
 VOL is not tracked per input as it always starts with the default volume
 Thus there are 2 parameters, 1 is saved per input, and 1 is across all inputs
 */

#define MAXPARAM 2                      

// Number of valid choices for each parameter
#define FILTERSEL 4   // Filter selection. Four possible filters.

#define chip1 0x50    // device address for the 24LC256 EEPROM chip

// VARIABLE DECLARATION

//char serialdata[80];               // Variable for storing the characters received through Serial3
//int lf = 10;                       // The character for line feed

String serialin;                   // Variable for storing the characters received through Serial3

RotaryEncoder encoder(VOLUPPIN, VOLDOWNPIN);       // Setup the first Rotary Encoder
RotaryEncoder encoder2(VOLUPPIN2, VOLDOWNPIN2);    // Setup the second Rotary Encoder
static int pos = 0;

unsigned long displayMillis = 0;   // Stores last recorded time for display interval
unsigned long debounceMillis = 0;  // Stores last recorded time for switch debounce interval
unsigned long selectMillis = 0;    // Stores last recorded time for being in select mode

unsigned long sr = 0;              // Stores the incoming sample rate.
unsigned long srold = 0;           // Stores the already displayed incoming sample rate. Used to save screen refreshes if the sr does not change.

byte input=0;                      // The current input to the DAC
byte filter=0;                     // The selected filter to the DAC
int volume=DEFAULTATTNU;           // Variable to hold the current attenuation value

byte select;                       // To record current select position (INP, VOL, etc)

boolean selectMode=false;          // To indicate whether in select mode or not
boolean spdifIn;                   // To indicate whether in I2S/DSD or SPDIF input format mode
//boolean SRExact=true;              // Display exact sample rate value; false = display nominal value
boolean dimmed=false;              // To indicate dim (mute) or not
boolean poweron=false;             // Default power-on condition: off
boolean leandisp=true;             // Default display mode: lean
byte pulse=0;                      // Used by the "heartbeat" display

int inputtype;                     // To hold the type of input. Set to 1 for DSD, 2 for PCM, 3 for I2S, 4 is for No Lock.
int inputtypeOld;                  // To hold the old type of input.

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
  //writeData(chip1,ICHO*MAXPARAM+1,SRExact); // Write the value of SRExact
  SerialUSB.println("Settings written");
   
}

void readSettings(){
  for(byte i=0;i<ICHO;i++) {
    for (byte j=0;j<MAXPARAM;j++) {
      settings[i][j]=readData(chip1, (i*MAXPARAM)+j);
    }
  }
  input=readData(chip1,ICHO*MAXPARAM);  // Read the last saved input setting
  //SRExact=readData(chip1,ICHO*MAXPARAM+1);  // Read the last saved setting for SRExact
  SerialUSB.println("Settings read");
   
}

/*
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
*/

// Volume setting routine. It just writes the current volume level to the Serial3 port.
void setVolume(int volume)
{
  Serial3.print("V-");          // output the new volume level to the DAC's serial port
  Serial3.println(volume);      // output the new volume level to the DAC's serial port
  SerialUSB.print("V-");        // for debugging
  SerialUSB.println(volume);    // for debugging
}


// Volume display routine.
void dispVol(int volume)
{
  int x;
  int y;
  //SerialUSB.println(volume);
  if (leandisp!=true) // Use this display layout when in the Settings menu.
      {
        x = 230;  // x coordinate of where the volume display area begins
        y = 100;  // y coordinate of where the volume display area begins
        myGLCD.setFont(BigFont);
        #ifdef WHITE
          myGLCD.setColor(255, 255, 255);
          myGLCD.setBackColor(0, 0, 0);
        #else
          myGLCD.setColor(0, 0, 0);
          myGLCD.setBackColor(255, 255, 255);
        #endif WHITE
        myGLCD.print("- ", x, y);
        if (volume<10)
        {
          #ifdef WHITE
            myGLCD.setColor(0, 0, 0);
            myGLCD.fillRect(x+12, y, x+50, y+70);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setBackColor(0, 0, 0);
          #else
            myGLCD.setColor(255, 255, 255);
            myGLCD.fillRect(x+12, y, x+50, y+70);
            myGLCD.setColor(0, 0, 0);
            myGLCD.setBackColor(255, 255, 255);
          #endif WHITE
          myGLCD.setFont(Shruti_Bold_num_48x70);
          myGLCD.printNumI(volume, x+59, y);
        }
        else
        {
        #ifdef WHITE
          myGLCD.setColor(255, 255, 255);
          myGLCD.setBackColor(0, 0, 0);
        #else
          myGLCD.setColor(0, 0, 0);
          myGLCD.setBackColor(255, 255, 255);
        #endif WHITE
          myGLCD.setFont(Shruti_Bold_num_48x70);
          myGLCD.printNumI(volume, x+11, y);
        }
       myGLCD.setFont(Ubuntubold);
        #ifdef WHITE
          myGLCD.setColor(255, 255, 255);
          myGLCD.setBackColor(0, 0, 0);
        #else
          myGLCD.setColor(0, 0, 0);
          myGLCD.setBackColor(255, 255, 255);
        #endif WHITE
       myGLCD.print("dB", x+110, y+25);
       selectMillis=millis();  // Reset being-in-select-mode timer
      }
   else      // Use this display layout when in the main menu.
      {
        x = 120;  // x coordinate of where the volume display area begins
        y = 10;   // y coordinate of where the volume display area begins
        #ifdef WHITE
          myGLCD.setColor(255, 255, 255);
          myGLCD.setBackColor(0, 0, 0);
        #else
          myGLCD.setColor(0, 0, 0);
          myGLCD.setBackColor(255, 255, 255);
        #endif WHITE
        myGLCD.setFont(calibri_bold_80);
        myGLCD.print("- ", x-10, y);
                
        if (volume<10)
        {
          #ifdef WHITE
            myGLCD.setColor(0, 0, 0);
            myGLCD.fillRect(x+12, y, x+50, y+70);
            myGLCD.setColor(255, 255, 255);
            myGLCD.setBackColor(0, 0, 0);
          #else
            myGLCD.setColor(255, 255, 255);
            myGLCD.fillRect(x+12, y, x+50, y+70);
            myGLCD.setColor(0, 0, 0);
            myGLCD.setBackColor(255, 255, 255);
          #endif WHITE
          myGLCD.setFont(Shruti_Bold_num_48x70);
          myGLCD.printNumI(volume, x+59, y);
        }
        else
        {
          myGLCD.setFont(Shruti_Bold_num_48x70);
          myGLCD.printNumI(volume, x+11, y);
        }
        myGLCD.setFont(calibri_bold_60);
        myGLCD.print("dB", x+110, y+20);
      }

SerialUSB.println("Volume displayed");
}

void rampUp()
{
  int i=(DIM-volume); 
  for(int dimval=DIM;dimval>volume;dimval--){
    setVolume(dimval);
    dispVol(dimval);
    delay((RAMP)*(1+(10/i*i)));
    i--;
  }
}


void setAndPrintFilter(byte value, boolean invert, boolean set){       // This is a function for selecting the filter

  myGLCD.setFont(SETTINGFONT);
  myGLCD.setColor(0, 150, 255);

  switch(value){
  case 0:
    if (set==true)
    {
      Serial3.println("F4");                                  // Select the first available filter
    }
    if (leandisp!=true) 
    {
      #ifdef WHITE
      myGLCD.setBackColor(0, 0, 0);
          if (invert==true)
          {
            myGLCD.setBackColor(255, 255, 255);
          }  
      #else
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      #endif WHITE
      myGLCD.print("Filter Selected: ", 2, 2);
      myGLCD.print("Linear      ", 2, myGLCD.getFontHeight()+2);
    }
    break;

  case 1:
    if (set==true)
    {
      Serial3.println("F5");                                  // Select the second available filter    
    }
    if (leandisp!=true) 
    {
      #ifdef WHITE
      myGLCD.setBackColor(0, 0, 0);
          if (invert==true)
          {
            myGLCD.setBackColor(255, 255, 255);
          }  
      #else
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      #endif WHITE 
      myGLCD.print("Filter Selected: ", 2, 2);
      myGLCD.print("Mixed      ", 2, myGLCD.getFontHeight()+2);
    }
    break;

  case 2:
    if (set==true)
    {
      Serial3.println("F6");                                  // Select the third available filter
    }
    if (leandisp!=true) 
    {
      #ifdef WHITE
      myGLCD.setBackColor(0, 0, 0);
          if (invert==true)
          {
            myGLCD.setBackColor(255, 255, 255);
          }  
      #else
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      #endif WHITE 
      myGLCD.print("Filter Selected: ", 2, 2);
      myGLCD.print("Minimum", 2, myGLCD.getFontHeight()+2);
    }
    break;

  case 3:
    if (set==true)
    {
      Serial3.println("F7");                                  // Select the fourth available filter
    }
    if (leandisp!=true) 
    {
      #ifdef WHITE
      myGLCD.setBackColor(0, 0, 0);
          if (invert==true)
          {
            myGLCD.setBackColor(255, 255, 255);
          }  
      #else
      myGLCD.setBackColor(255, 255, 255);
          if (invert==true)
          {
            myGLCD.setBackColor(0, 0, 0);
          }  
      #endif WHITE 
      myGLCD.print("Filter Selected: ", 2, 2);
      myGLCD.print("Soft          ", 2, myGLCD.getFontHeight()+2);
    }
    break;
  }
}

void setAndPrintInput(int value, boolean invert, boolean set){
  setAndPrintFilter(settings[input][FILTVAL]%FILTERSEL, false, false);  // Setup Filter display format
  dispVol(volume);                                                      // Display current volume

  SerialUSB.println("Input parameters set");

  if (leandisp!=true) 
  {
    myGLCD.setFont(Ubuntubold);
    #ifdef WHITE
    myGLCD.setColor(0, 0, 0);
    myGLCD.fillRect(230, 9, 395, 80);
    myGLCD.setColor(255, 255, 255);
    myGLCD.setBackColor(0, 0, 0);
    if (invert==true)
    {
      myGLCD.setColor(255, 255, 255);
      myGLCD.fillRect(230, 9, 395, 80);
      myGLCD.setColor(0, 0, 0);
      myGLCD.setBackColor(255, 255, 255);
    }  
    #else
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
    #endif WHITE
    myGLCD.print("Input:", 230, 10);
  }  

  myGLCD.setFont(calibri_bold_40);
  
  switch (value){
  case 0:
    if (set==true)
    {
    Serial3.println("I1");            // output the new input to the DAC's serial port
    SerialUSB.println("I1");          // for debugging
    }
    if (leandisp!=true) 
    {
      myGLCD.print(no0, 230, 42);
    } else
    {
      #ifdef WHITE
      myGLCD.drawBitmap(30, 90, 120, 120, bnc_b);
      myGLCD.setColor(0, 0, 0);
      myGLCD.fillRect(1, 207, 195, 239);
      myGLCD.setColor(255, 255, 255);
      #else
      myGLCD.drawBitmap(30, 90, 120, 120, bnc);
      myGLCD.setColor(255, 255, 255);
      myGLCD.fillRect(1, 207, 195, 239);
      myGLCD.setColor(0, 0, 0);
      #endif WHITE
      
      myGLCD.print(no0, 90-(myGLCD.getStringWidth(no0)/2), 202);
    }
    break;
  case 1:
    if (set==true)
    {
    Serial3.println("I2");            // output the new input to the DAC's serial port
    SerialUSB.println("I2");          // for debugging
    }
    if (leandisp!=true) 
    {
      myGLCD.print(no1, 230, 42);
    } else
    {
      #ifdef WHITE
      myGLCD.drawBitmap(30, 90, 120, 120, toslink_b);
      myGLCD.setColor(0, 0, 0);
      myGLCD.fillRect(1, 207, 195, 239);
      myGLCD.setColor(255, 255, 255);
      #else
      myGLCD.drawBitmap(30, 90, 120, 120, toslink);
      myGLCD.setColor(255, 255, 255);
      myGLCD.fillRect(1, 207, 195, 239);
      myGLCD.setColor(0, 0, 0);
      #endif WHITE

      myGLCD.print(no1, 90-(myGLCD.getStringWidth(no1)/2), 202);
    }
    break;
  case 2:
    if (set==true)
    {
    Serial3.println("I0");            // output the new input to the DAC's serial port
    SerialUSB.println("I0");          // for debugging
    }
    if (leandisp!=true) 
    {
      myGLCD.print(no2, 230, 42);
    } else
    {
      #ifdef WHITE
      myGLCD.drawBitmap(30, 90, 120, 120, usb_b);
      myGLCD.setColor(0, 0, 0);
      myGLCD.fillRect(1, 207, 195, 239);
      myGLCD.setColor(255, 255, 255);
      #else
      myGLCD.drawBitmap(30, 90, 120, 120, usb);
      myGLCD.setColor(255, 255, 255);
      myGLCD.fillRect(1, 207, 195, 239);
      myGLCD.setColor(0, 0, 0);
      #endif WHITE

      myGLCD.print(no2, 90-(myGLCD.getStringWidth(no2)/2), 202);
    }
    break;
  case 3:
    if (set==true)
    {
    Serial3.println("I3");            // output the new input to the DAC's serial port
    SerialUSB.println("I3");          // for debugging
    }
    if (leandisp!=true) 
    {
      myGLCD.print(no3, 230, 42);
    } else
    {
      #ifdef WHITE
      myGLCD.drawBitmap(30, 90, 120, 120, automatic_b);
      myGLCD.setColor(0, 0, 0);
      myGLCD.fillRect(1, 207, 195, 239);
      myGLCD.setColor(255, 255, 255);
      #else
      myGLCD.drawBitmap(30, 90, 120, 120, automatic);
      myGLCD.setColor(255, 255, 255);
      myGLCD.fillRect(1, 207, 195, 239);
      myGLCD.setColor(0, 0, 0);
      #endif WHITE

      myGLCD.print(no3, 90-(myGLCD.getStringWidth(no3)/2), 202);
    }
    break;
  }
}

void dispSR(int samplerate)                // Display the detected Sampling Rate and signal type.
  {
    if (leandisp=true)
        {
          if (inputtype==2 && inputtype!=inputtypeOld) {
          #ifdef WHITE
          myGLCD.setColor(0, 0, 0);
          myGLCD.fillRoundRect(215, 100, 340, 180);
          myGLCD.setColor(255, 255, 255);
          myGLCD.drawBitmap(230, 100, 89, 77, pcm_b);
          #else
          myGLCD.setColor(255, 255, 255);
          myGLCD.fillRoundRect(215, 100, 340, 180);
          myGLCD.setColor(0, 0, 0);
          myGLCD.drawBitmap(230, 100, 89, 77, pcm);
          #endif WHITE
          srold = 0;
          inputtypeOld = inputtype;
          }
          inputtype = 2;                                          // Input type is PCM.
        }
        else
        {
          myGLCD.print("PCM    ", 220, 215);
          inputtype = 2;
          srold = 0;
        }
            myGLCD.setFont(SRFONT);
            #ifdef WHITE
            myGLCD.setColor(0, 0, 0);
            myGLCD.fillRect(190, 181, 399, 239);
            myGLCD.setColor(255, 255, 255);
            #else
            myGLCD.setColor(255, 255, 255);
            myGLCD.fillRect(190, 181, 399, 239);
            myGLCD.setColor(0, 0, 0);
            #endif WHITE
            
              if(sr == 384)
                myGLCD.print("384 KHz", 275-(myGLCD.getStringWidth("384 KHz")/2), 185);
              else
                if(sr == 352)
                  myGLCD.print("352 KHz", 275-(myGLCD.getStringWidth("352 KHz")/2), 185);
                else
                  if(sr == 192)
                    myGLCD.print("192 KHz", 275-(myGLCD.getStringWidth("192 KHz")/2), 185);
                  else
                    if(sr == 176)
                      myGLCD.print("176 KHz", 275-(myGLCD.getStringWidth("176 KHz")/2), 185);
                    else
                      if(sr == 96)
                        myGLCD.print("96 KHz", 275-(myGLCD.getStringWidth("96 KHz")/2), 185);
                      else
                        if(sr == 88)
                          myGLCD.print("88 KHz", 275-(myGLCD.getStringWidth("88 KHz")/2), 185);
                        else
                          if(sr == 48)
                            myGLCD.print("48 KHz", 275-(myGLCD.getStringWidth("48 KHz")/2), 185);
                          else
                            if(sr == 44)
                              myGLCD.print("44.1 KHz", 275-(myGLCD.getStringWidth("44.1 KHz")/2), 185);
                            else
                              if(sr == 32)
                              myGLCD.print("32 KHz", 275-(myGLCD.getStringWidth("32 KHz")/2), 185);
  }


// Begin 24LC256 EEPROM section

void writeData(int device, unsigned int add, byte data) // writes a byte of data 'data' to the chip at I2C address 'device', in memory location 'add'
{
  Wire.beginTransmission(device);
  Wire.write((int)(add >> 8));   // left-part of pointer address
  Wire.write((int)(add & 0xFF)); // and the right
  Wire.write(data);
  Wire.endTransmission();
  delay(6);
  SerialUSB.println("Data written");
}

byte readData(int device, unsigned int add) // reads a byte of data from memory location 'add' in chip at I2C address 'device'
{
  byte result;                    // returned value
  Wire.beginTransmission(device); //  these three lines set the pointer position in the EEPROM
  Wire.write((int)(add >> 8));    // left-part of pointer address
  Wire.write((int)(add & 0xFF));  // and the right
  Wire.endTransmission();
  Wire.requestFrom(device,1);     // now get the byte of data...
  result = Wire.read();
  return result;                  // and return it as a result of the function readData
}
// End 24LC256 section
    
// The following routine sets up the DAC at startup

void startDac(){
// Print the welcome message and other labels to the LCD
  myGLCD.clrScr();
  myGLCD.setColor(255, 255, 255);
  myGLCD.fillRoundRect(0, 0, 399, 239);
  myGLCD.setFont(calibri_bold_40);
  myGLCD.setColor(0, 0, 255);
  myGLCD.setBackColor(255, 255, 255);
  myGLCD.print("A r D A M 1 0 2 1", 200-(myGLCD.getStringWidth("A r D A M 1 0 2 1")/2), 10);
  myGLCD.print("Soekris R-2R DAC", 200-(myGLCD.getStringWidth("Soekris R-2R DAC")/2), 55);
  myGLCD.print("v.0.74", 200-(myGLCD.getStringWidth("v.0.74")/2), 200);

  myGLCD.setBackColor(0, 0, 0);
  delay(2500);
  myGLCD.clrScr();
  setVolume(volume);
  #ifdef WHITE
  myGLCD.setColor(0, 0, 0);
  #else
  myGLCD.setColor(255, 255, 255);
  myGLCD.fillRoundRect(0, 0, 399, 239);
  #endif WHITE
  
  readSettings();
  input=3;                                    // Use Auto as default input
  setAndPrintInput(input, false, true);
  SerialUSB.println("StartDAC complete");
}

/************************ MAIN PROGRAM ************************************************************/

void setup() {

  SerialUSB.begin(115200);        // for debugging
  Serial3.begin(115200);

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
  startDac();
  delay(300);
  //startDac2();
  //delay(200);
  #endif ALWAYSON

  SerialUSB.println("Setup complete!");
   
  }  // End setup()

// -------------------------------------loop section------------------------------------------

void loop() {
 
  // Print the sample rate, input and lock (once every "INTERVAL_SAMPLE" time)
  if((millis() - displayMillis > INTERVAL_SAMPLE*1000)&&!selectMode&&poweron) {
    displayMillis = millis(); // Saving last time we display sample rate
    
   if(Serial3.available())
    {        
      serialin = Serial3.readStringUntil('\n');
         
      SerialUSB.print("I received: ");        // for debugging
      SerialUSB.print(serialin);              // for debugging
      SerialUSB.print("\n");                  // for debugging
        
      if(serialin == "L384\r")
        sr = 384;
        else
          if(serialin == "L352\r")
            sr = 352;
            else
              if(serialin == "L192\r")
                sr = 192;
                else
                  if(serialin == "L176\r")
                    sr = 176;
                    else
                      if(serialin == "L096\r")
                        sr = 96;
                        else
                          if(serialin == "L088\r")
                            sr = 88;
                            else
                              if(serialin == "L048\r")
                                sr = 48;
                                else
                                  if(serialin == "L044\r")
                                    sr = 44;
                                    else
                                      if(serialin == "L32\r")
                                      sr = 32;
  
       SerialUSB.print("Sampling rate is detected to be: ");    // for debugging
       SerialUSB.print(sr);                                     // for debugging
       SerialUSB.print("KHz\n");                                // for debugging
       
       serialin = "";
    }
       dispSR(sr);
       srold = sr;
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

// Beginning of IR code
// Process the IR input, if any
  if (irrecv.decode(&results)) {
    if (results.value == POWER_CODE) {
        SerialUSB.write("Power\n");
        if (poweron==false)                              // Check if already powered on
      {
        digitalWrite(POWERPIN, HIGH);
        digitalWrite(DIMMPIN, LOW);
        poweron=true;
        Wire.begin();                                   // Join the I2C bus as a master
        startDac();
        delay(300);
      }
      else {
        digitalWrite(POWERPIN, LOW);
        digitalWrite(DIMMPIN, HIGH);
        poweron=false;
        myGLCD.clrScr();
      }
    } 
    else if (results.value == VOLUP_CODE) {
      SerialUSB.write("Volume Up\n");
      if (volume>MINATTNU && poweron)               // Check if not already at minimum attenuation
      {
        if(dimmed) {
          rampUp();                                    // Disengage dim
          dimmed=false;
        }

        volume-=1;                                 // Decrease attenuation 1 dB (increase volume 1 db)
        setVolume(volume);                         // Write value into registers
        dispVol(volume);
        prev_result = VOLUP_CODE;
      }
     } 
    else if (results.value == VOLDOWN_CODE) {
      SerialUSB.write("Volume Down\n");
         if (volume<MAXATTNU && poweron)           // Check if not already at maximum attenuation
      {
        if(dimmed) {
          rampUp();                        // Disengage dim
          dimmed=false;
        }

        volume+=1;                      // Increase 1 dB attenuation (decrease volume 1 db)
        setVolume(volume);         // Write value into registers
        dispVol(volume);
        prev_result = VOLDOWN_CODE;
      }
    } 
    else if (results.value == MUTE_CODE) {
      SerialUSB.write("Mute\n");
        if(dimmed && poweron){             // Back to previous volume level 1 db at a time
        rampUp();                          // Disengage dim
        dimmed=false;
      }
      else {
        if(DIM>=volume && poweron) {    // only DIM if current attenuation is lower
          setVolume(DIM);             // Dim volume
          dispVol(DIM);
          dimmed=true;
        }
      }
      prev_result = 0;
    } 
    else if (results.value == SELECT_CODE) {
      SerialUSB.write("Select\n");
      if(poweron) 
      {
        if(selectMode==false)
      {
        #ifdef WHITE
        myGLCD.setColor(0, 0, 0);
        myGLCD.clrScr();
        //myGLCD.fillRoundRect(0, 0, 399, 239);       // This can probably go..
        #else
        myGLCD.setColor(255, 255, 255);
        myGLCD.fillRoundRect(0, 0, 399, 239);
        #endif WHITE
      }  
      selectMode=true;          // Now we are in select mode
      
      if (leandisp=true)
      {
        leandisp=false;
      }
      debounceMillis=millis();  // Start debounce timer
      selectMillis=millis();    // Start being-in-select-mode timer
      select++;                 // Move to next select option
      myGLCD.setColor(255, 255, 0);

      switch(select%(MAXPARAM+1)){
      case VOL:
        break;
      case INP:
        setAndPrintFilter(settings[input][FILTVAL]%FILTERSEL, false, false);
        setAndPrintInput(input, true, false);
        break;
      case FILT:
        setAndPrintInput(input, false, false);
        setAndPrintFilter(settings[input][FILTVAL]%FILTERSEL, true, true);
        break;
      }  // End of switch
      
      }
      prev_result = 0;
    } 
    else if (results.value == LEFT_CODE) {
      SerialUSB.write("Left\n");
      if(poweron) {
      irdir=1;    
      }
      prev_result = 0;
    } 
    else if (results.value == RIGHT_CODE) {
      SerialUSB.write("Right\n");
      if(poweron) {
      irdir=2;  
      }
      prev_result = 0;
    } 
    else if (results.value == SOURCE1_CODE && input!=0) {
      SerialUSB.write("Source 1\n");
      if(poweron) {
        input=0;
        inputtype = 2;
        input%=ICHO;
        setAndPrintInput(input, false, true);
      }
      prev_result = 0;
    } 
    else if (results.value == SOURCE2_CODE && input!=1) {
      SerialUSB.write("Source 2\n");
      if(poweron) {
        input=1;
        inputtype = 2;
        input%=ICHO;
        setAndPrintInput(input, false, true);
      }
      prev_result = 0;
    } 
    else if (results.value == SOURCE3_CODE && input!=2) {
      SerialUSB.write("Source 3\n");
      if(poweron) {
        input=2;
        inputtype = 2;
        input%=ICHO;
        setAndPrintInput(input, false, true);
      }
      prev_result = 0;
    } 
    else if (results.value == SOURCE4_CODE && input!=3) {
      SerialUSB.write("Source 4\n");
      if(poweron) {
        input=3;
        inputtype = 2;
        input%=ICHO;
        setAndPrintInput(input, false, true);
      }
      prev_result = 0;
    } 
    else if (results.value == SOURCE5_CODE && input!=4) {
      SerialUSB.write("Source 5\n");
      if(poweron) {
        input=4;
        inputtype = 2;
        input%=ICHO;
        setAndPrintInput(input, false, true);
      }
      prev_result = 0;
    } 
    else if (results.value == SOURCE6_CODE && input!=5) {
      SerialUSB.write("Source 6\n");
      if(poweron) {
        input=5;
        inputtype = 2;
        input%=ICHO;
        setAndPrintInput(input, false, true);
      }
      prev_result = 0;
    } 
    else if (results.value == SOURCE7_CODE && input!=6) {
      SerialUSB.write("Source 7\n");
      if(poweron) {
        input=6;
        inputtype = 2;
        input%=ICHO;
        setAndPrintInput(input, false, true);
      }
      prev_result = 0;
    } 
    else if (results.value == SOURCE8_CODE && input!=7) {
      SerialUSB.write("Source 8\n");
      if(poweron) {
        input=7;
        inputtype = 2;
        input%=ICHO;
        setAndPrintInput(input, false, true);
      }
      prev_result = 0;
    } 
    else if (results.value == SOURCE9_CODE && input!=8) {
      SerialUSB.write("Source 9\n");
      if(poweron) {
        input=8;
        inputtype = 2;
        input%=ICHO;
        setAndPrintInput(input, false, true);
      }
      prev_result = 0;
    } 
    else if (results.value == SOURCE10_CODE && input!=9) {
      SerialUSB.write("Source 10\n");
      if(poweron) {
        input=9;
        inputtype = 2;
        input%=ICHO;
        setAndPrintInput(input, false, true);
      }
      prev_result = 0;
    } 

    else if (results.value == FILT1_CODE && filter!=0) {
      SerialUSB.write("Filter 1\n");
      if(poweron) {
        filter=0;
        //input%=ICHO;
        setAndPrintFilter(filter, false, true);
      }
      prev_result = 0;
    } 

    else if (results.value == FILT2_CODE && filter!=1) {
      SerialUSB.write("Filter 2\n");
      if(poweron) {
        filter=1;
        //input%=ICHO;
        setAndPrintFilter(filter, false, true);
      }
      prev_result = 0;
    }
    
    else if (results.value == FILT3_CODE && filter!=2) {
      SerialUSB.write("Filter 3\n");
      if(poweron) {
        filter=2;
        //input%=ICHO;
        setAndPrintFilter(filter, false, true);
      }
      prev_result = 0;
    }

    else if (results.value == FILT4_CODE && filter!=3) {
      SerialUSB.write("Filter 4\n");
      if(poweron) {
        filter=3;
        //input%=ICHO;
        setAndPrintFilter(filter, false, true);
      }
      prev_result = 0;
    }
    
    else if (results.value == 0xFFFFFFFF) {
    SerialUSB.write("Repeat\n");
    SerialUSB.println(prev_result);
    switch(prev_result){
      case VOLUP_CODE:
        SerialUSB.write("Volume Up\n");
        if (volume>MINATTNU && poweron)                // Check if not already at minimum attenuation
        {
        if(dimmed) {
          rampUp();                     // Disengage dim
          dimmed=false;
        }
        volume-=1;                      // Decrease attenuation 1 dB (increase volume 1 db)
        setVolume(volume);              // Send volume command to the DAC
        dispVol(volume);                // Display the volume
        prev_result == 0;
      }
      break;
     
    case VOLDOWN_CODE:
      SerialUSB.write("Volume Down\n");
         if (volume<MAXATTNU && poweron)              // Check if not already at maximum attenuation
        {
        if(dimmed) {
          rampUp();                     // Disengage dim
          dimmed=false;
        }
        volume+=1;                      // Increase 1 dB attenuation (decrease volume 1 db)
        setVolume(volume);              // Send volume command to the DAC
        dispVol(volume);                // Display the volume
        prev_result == 0;
        }
        break;
      }
    }
  
    else {
      SerialUSB.print("unexpected value: ");
      SerialUSB.println(results.value, HEX);
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
      startDac();
      //startDac2();        
    }
    else {
      selectMode=true;          // Now we are in select mode
      if (leandisp=true)
      {
        #ifdef WHITE
        myGLCD.clrScr();
        myGLCD.setColor(0, 0, 0);
        //myGLCD.fillRoundRect(0, 0, 399, 239);
        #else
        myGLCD.setColor(255, 255, 255);
        myGLCD.fillRoundRect(0, 0, 399, 239);
        #endif WHITE
        leandisp=false;
        setAndPrintInput(input, false, false);
      }
      debounceMillis=millis();  // Start debounce timer
      selectMillis=millis();    // Start being-in-select-mode timer

      select++;  // Move to next select option
      myGLCD.setColor(255, 255, 0);

      switch(select%(MAXPARAM+1)){
      case VOL:
        break;
      case INP:
        setAndPrintFilter(settings[input][FILTVAL]%FILTERSEL, false, false);
        setAndPrintInput(input, true, true);
        break;
      case FILT:
        setAndPrintInput(input, false, false);
        setAndPrintFilter(settings[input][FILTVAL]%FILTERSEL, true, true);
        break;
      }  // End of switch
    }
  }

  // For the rotary encoder and the remote control. The control depends whether in display mode or select mode
  
  int dir = 0;
  //static int pos = 0;
  encoder.tick();
  int newPos = encoder.getPosition();
  if (pos != newPos) 
  {
    if (pos < newPos)
    {
      dir = 1;
      pos = newPos;
    }
    else if (pos > newPos)
         {
           dir = 2;
           pos = newPos;
         }
  }
    
  if(dir==1 || dir==2 || irdir==1 || irdir==2)
  {
    delay(INTERVAL_BOUNCE);  // debounce by waiting INTERVAL_BOUNCE time

    switch(select%(MAXPARAM+1)){
    case VOL:  // Volume adjustment
      dimmed=false;                          // Disengage dim
      if (dir==1 || irdir==1)  // CCW
      {
        if (volume<MAXATTNU && poweron)      // Check if not already at maximum attenuation
        {
          volume+=1;                      // Increase 1 dB attenuation (decrease volume 1 db)
          setVolume(volume);              // Send volume command to the DAC
          dispVol(volume);                // Display the volume
          irdir=0;
        }
      }
      else                                   // If not CCW, then it is CW
      {
        if (volume>MINATTNU && poweron)              // Check if not already at minimum attenuation
        {
          volume-=1;                      // Decrease attenuation 1 dB (increase volume 1 db)
          setVolume(volume);              // Send volume command to the DAC
          dispVol(volume);                // Display the volume
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
      setAndPrintFilter(settings[input][FILTVAL]%FILTERSEL, false, false);
      setAndPrintInput(input, true, true);
      selectMillis=millis();       // Reset being-in-select-mode timer
      irdir=0;
      break;
    case FILT:                     // Filter selection
      if (dir==1 || irdir==1) {
      settings[input][FILTVAL]--;  // CCW
      irdir=0;  }
      else settings[input][FILTVAL]++;
      setAndPrintInput(input, false, false);
      setAndPrintFilter(settings[input][FILTVAL]%FILTERSEL, true, true);
      selectMillis=millis();       // Reset being-in-select-mode timer
      irdir=0;
      break;

    }  // End of (rotary encoder) switch

  }  // End of if(rotating)

  // When the being-in-select mode timer expires, we revert to volume/display mode
  if(selectMode&&millis()-selectMillis > INTERVAL_SELECT*1000){
    selectMode=false;                        // No longer in select mode
    leandisp=true;                           // Go to lean display mode
    inputtypeOld = 0;                        // Input type is undefined.
    
    #ifdef WHITE
    myGLCD.clrScr();
    myGLCD.setColor(0, 0, 0);          
    //myGLCD.fillRoundRect(0, 0, 399, 239);    // Draw black background.
    #else
    myGLCD.setColor(255, 255, 255);          
    myGLCD.fillRoundRect(0, 0, 399, 239);    // Draw white background.
    #endif WHITE
    
    setAndPrintInput(input, false, false);   // Draw the screen
    dispVol(volume);                         // Display current volume level
    select=VOL;                              // Back to volume mode
    writeSettings();                         // Write new settings into EEPROM, including current input
  } 
} // End of loop()


// Begin new rotary encoder code (no longer using interrupts)
  int state0[]={0,0,0,0};
  int RotEncoder() 
  {
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


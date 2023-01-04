/*******************************************************************************************************************
 * La Skala Attenuator: A ladder-type volume attenuator with Arduino control.
 *
 * Project Page: http://www.dimdim.gr/diyaudio/la-skala-attenuator/ 
 *
 * v0.52    30/05/2016 : - Bug fixes. Everything seems to be working OK.
 *                       - First public release.
 *                       - Commented out TFT code to make code fit in an UNO.
 *                     
 * v0.44    11/05/2016 : - Added code to take into account the number of relays that switch states, so as to
 *                         reduce power supply loading due to many relays activating at the same time.
 *                       - Attempt to eliminate small glitches occuring at certain steps by implementing different
 *                         function for increasing & decreasing attenuation.
 * 
 * v0.30    15/02/2016 : - First attempt at coding an appropriate algorithm. It works!
 *                       - Outputs status info to serial port and TFT (ILI9341 with SPI).
 *******************************************************************************************************************/

#include <UTFT.h>                                     // Library for the TFT display.
#include <Wire.h>                                     // Library for the I2C communication.
#include "Adafruit_MCP23008.h"                        // Library for the I/O expander.
#include <RotaryEncoder.h>                            // Library for the encoder.

float volume = 10;                                    // Default attenuation.
float volume_old = 1000;                              // Store old volume.
int energ_relays;                                     // Store the number of energized relays.
int energ_relays_old;                                 // Store the number of previously energized relays.
int relay_delay;

extern uint8_t BigFont[];

// UTFT myGLCD(ILI9341_S5P,MOSI,SCK,3,23,22);            // Initiate the display.

Adafruit_MCP23008 mcp;

#define mcp0 64                                       // First relay will attenuate by 64db.
#define mcp1 32                                       // Second relay will attenuate by 32db.
#define mcp2 16                                       // Third relay will attenuate by 16db.
#define mcp3 8                                        // Fourth relay will attenuate by 8db.
#define mcp4 4                                        // Fifth relay will attenuate by 4db.
#define mcp5 2                                        // Sixth relay will attenuate by 2db.
#define mcp6 1                                        // Seventh relay will attenuate by 1db.
#define mcp7 0.5                                      // Eighth relay will attenuate by 0.5db.

boolean relay0 = 0;
boolean relay1 = 0;
boolean relay2 = 0;
boolean relay3 = 0;
boolean relay4 = 0;
boolean relay5 = 0;
boolean relay6 = 0;
boolean relay7 = 0;

int VOLUPPIN=A3;           // RotEnc A terminal for right rotary encoder.
int VOLDOWNPIN=A2;         // RotEnc B terminal for right rotary encoder.

RotaryEncoder encoder(VOLDOWNPIN, VOLUPPIN);       // Setup the first Rotary Encoder
  
void setup() {  

  //myGLCD.InitLCD();
  //myGLCD.setFont(BigFont);

  mcp.begin();                                // use default address 0
  mcp.pinMode(0, OUTPUT);
  mcp.pinMode(1, OUTPUT);
  mcp.pinMode(2, OUTPUT);
  mcp.pinMode(3, OUTPUT);
  mcp.pinMode(4, OUTPUT);
  mcp.pinMode(5, OUTPUT);
  mcp.pinMode(6, OUTPUT);
  mcp.pinMode(7, OUTPUT);

  setVol(volume);                 // Set the attenuation at its default value. Must be as close to the beginning of the code as possible to avoid audio spikes at start-up.
  
  pinMode(VOLUPPIN, INPUT);       // Button switch or Encoder pin for volume up
  digitalWrite(VOLUPPIN, HIGH);   // If H/W debouncing is implemented, set to LOW
  pinMode(VOLDOWNPIN, INPUT);     // Button switch or Encoder pin for volume down
  digitalWrite(VOLDOWNPIN, HIGH); // If H/W debouncing is implemented, set to LOW

  Serial.begin(115200); 

  //myGLCD.clrScr();
  //myGLCD.setColor(255, 255, 0);
  //myGLCD.setBackColor(0, 0, 0);
  //myGLCD.print("DimDim's Ladder", CENTER, 30);
  //myGLCD.print("Volume Control", CENTER, 60);
  //delay(1000);
}

void loop() {

  static int pos = 0;                        // Read the rotary encoder and increase or decrease attenuation.
  encoder.tick();
  int newPos = encoder.getPosition();
  if (pos != newPos) 
  {
    if (pos < newPos)
    {
      if (volume > 0)
        {
          volume=volume-0.5;
        }
      pos = newPos;
    }
    else if (pos > newPos)
         {
           if (volume < 127)
            {
              volume=volume+0.5;
            }
           pos = newPos;
         }
        
    //myGLCD.setColor(255, 255, 255);
    //myGLCD.setBackColor(0, 0, 0);
    //volume = -1*volume;
    //myGLCD.setFont(BigFont);
    //myGLCD.printNumF(volume, 1, CENTER, 100);
    //volume = -1*volume;
    setVol(volume);
  }

}

void setVol(float volume_temp)              // Set the volume by controlling the 8 relays.
  {
  if (volume_temp != volume_old) {
  Serial.print("Setting volume to: ");
  Serial.println(volume_temp);
  //myGLCD.setColor(255, 255, 255);
  //myGLCD.setBackColor(0, 0, 0);
  //volume = -1*volume;
  //myGLCD.setFont(BigFont);
  //myGLCD.printNumF(volume, 1, CENTER, 100);
  //volume = -1*volume;
  energ_relays = 0;
  float vol_temp_2 = volume_temp;
  
  if (volume_temp >= mcp0)
    {
      relay0 = 1;
      energ_relays++;
      volume_temp = volume_temp - mcp0;
      Serial.println(volume_temp);
    } else relay0 = 0;
  
  if (volume_temp >= mcp1)
    {
      relay1 = 1;
      energ_relays++;
      volume_temp = volume_temp - mcp1;
      Serial.println(volume_temp);
    } else relay1 = 0;
    
  if (volume_temp >= mcp2)
    {
      relay2 = 1;
      energ_relays++;      
      volume_temp = volume_temp - mcp2;
      Serial.println(volume_temp);
    } else relay2 = 0;

  if (volume_temp >= mcp3)
    {
      relay3 = 1;
      energ_relays++;      
      volume_temp = volume_temp - mcp3;
      Serial.println(volume_temp);
    } else relay3 = 0;

  if (volume_temp >= mcp4)
    {
      relay4 = 1;
      energ_relays++;      
      volume_temp = volume_temp - mcp4;
      Serial.println(volume_temp);      
    } else relay4 = 0;

  if (volume_temp >= mcp5)
    {
      relay5 = 1;
      energ_relays++;      
      volume_temp = volume_temp - mcp5;
      Serial.println(volume_temp);      
    } else relay5 = 0;
 
  if (volume_temp >= mcp6)
    {
      relay6 = 1;
      energ_relays++;      
      volume_temp = volume_temp - mcp6;
      Serial.println(volume_temp);      
    } else relay6 = 0;
 
  if (volume_temp >= mcp7)
    {
      relay7 = 1;
      energ_relays++;      
      volume_temp = volume_temp - mcp7;
      Serial.println(volume_temp);      
    } else relay7 = 0;

  Serial.print("Difference in switched relays: ");                  // Determine how many relays will be switching states. Useful to predict the current load imposed on the power supply.
  Serial.println(abs(energ_relays_old - energ_relays));
    
    if (abs(energ_relays_old - energ_relays) <= 3) 
      {
        relay_delay = 0;
      }
    if (abs(energ_relays_old - energ_relays) == 4)                  // When a large number of relays is expected to switch states, introduce a delay between activations to ease the burden of the power supply (and decrease switching noise).
      {
        relay_delay = 5;
      }
    if (abs(energ_relays_old - energ_relays) == 5) 
      {
        relay_delay = 10;
      }
    if (abs(energ_relays_old - energ_relays) == 6) 
      {
        relay_delay = 20;
      }
    if (abs(energ_relays_old - energ_relays) >= 7) 
      {
        relay_delay = 50;
      }
    
    energ_relays_old = energ_relays;
    
    if (vol_temp_2 > volume_old)                                    // If we are increasing the attenuation
      {
      Serial.println("Increasing the attenuation");
        if (relay0 == 0)
          {
            mcp.digitalWrite(0, LOW);
          }
          else mcp.digitalWrite(0, HIGH);

        delay(relay_delay);
        
        if (relay1 == 0)
          {
            mcp.digitalWrite(1, LOW);
          }
          else mcp.digitalWrite(1, HIGH);

        delay(relay_delay);
        
        if (relay2 == 0)
          {
            mcp.digitalWrite(2, LOW);
          }
          else mcp.digitalWrite(2, HIGH);

        delay(relay_delay);
        
        if (relay3 == 0)
          {
            mcp.digitalWrite(3, LOW);
          }
          else mcp.digitalWrite(3, HIGH);

        delay(relay_delay);
        
        if (relay4 == 0)
          {
            mcp.digitalWrite(4, LOW);
          }
          else mcp.digitalWrite(4, HIGH);

        delay(relay_delay);
        
        if (relay5 == 0)
          {
            mcp.digitalWrite(5, LOW);
          }
          else mcp.digitalWrite(5, HIGH);

        delay(relay_delay);
        
        if (relay6 == 0)
          {
            mcp.digitalWrite(6, LOW);
          }
          else mcp.digitalWrite(6, HIGH);

        delay(relay_delay);
        
        if (relay7 == 0)
          {
            mcp.digitalWrite(7, LOW);
          }
          else mcp.digitalWrite(7, HIGH);
      volume_old = vol_temp_2;
      }


    if (vol_temp_2 < volume_old)                                  // If we are decreasing the attenuation
      {
      Serial.println("Decreasing the attenuation");
        if (relay7 == 0)
          {
            mcp.digitalWrite(7, LOW);
          }
          else mcp.digitalWrite(7, HIGH);

        delay(relay_delay);
        
        if (relay6 == 0)
          {
            mcp.digitalWrite(6, LOW);
          }
          else mcp.digitalWrite(6, HIGH);

        delay(relay_delay);
        
        if (relay5 == 0)
          {
            mcp.digitalWrite(5, LOW);
          }
          else mcp.digitalWrite(5, HIGH);

        delay(relay_delay);
        
        if (relay4 == 0)
          {
            mcp.digitalWrite(4, LOW);
          }
          else mcp.digitalWrite(4, HIGH);

        delay(relay_delay);
        
        if (relay3 == 0)
          {
            mcp.digitalWrite(3, LOW);
          }
          else mcp.digitalWrite(3, HIGH);

        delay(relay_delay);
        
        if (relay2 == 0)
          {
            mcp.digitalWrite(2, LOW);
          }
          else mcp.digitalWrite(2, HIGH);

        delay(relay_delay);
        
        if (relay1 == 0)
          {
            mcp.digitalWrite(1, LOW);
          }
          else mcp.digitalWrite(1, HIGH);

        delay(relay_delay);
        
        if (relay0 == 0)
          {
            mcp.digitalWrite(0, LOW);
          }
          else mcp.digitalWrite(0, HIGH);
      volume_old = vol_temp_2;
      }    
  }
  }

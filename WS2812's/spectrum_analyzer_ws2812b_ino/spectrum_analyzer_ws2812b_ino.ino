/*
  Copyright (c) 2019 Shajeeb TM

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

  WS2812B Led Matrix vesion by Janux
*/

#include <arduinoFFT.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>

#define SAMPLES 16        //Must be a power of 2
#define  xres 8          // Total number of  columns in the display, must be <= SAMPLES/2
#define  yres 6           // Total number of  rows in the display
#define ledPIN 6          // pint to control Led Matrix
#define NUM_LEDS (xres * yres)
#define BRIGHTNESS 35
#define buttonPin 5       // the number of the pushbutton pin to change displaycolor

byte yvalue;
byte displaycolumn, displayvalue;
int peaks[xres];
byte state = HIGH;           // the current reading from the input pin
byte previousState = LOW;    // the previous reading from the input pin
byte displaycolor = 0;

//Arrays for samplig
double vReal[SAMPLES];
double vImag[SAMPLES];
byte data_avgs[xres];
arduinoFFT FFT = arduinoFFT(); // FFT object

unsigned long lastDebounceTime = 0;   // the last time the output pin was toggled
unsigned long debounceDelay = 100;    // the debounce time; increase if the output flickers

// Parameter 1 = number of leds in matrix
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel pixel = Adafruit_NeoPixel(NUM_LEDS, ledPIN, NEO_GRB + NEO_KHZ800);

// EQ filter to attenuates bass and improves treble
// Useful on PC sound card which usually has many bass and poor high frequency
bool EQ_ON = false; // set to false to disable eq
byte eq[32] = {50, 55, 60, 70, 75, 80, 85, 95,
               100, 100, 100, 100, 100, 100, 100, 100,
               100, 100, 100, 100, 100, 100, 100, 100,
               115, 125, 140, 160, 185, 200, 225, 255
              };

//Define color for single led, used in setColumn function, 0 for custom color
//Color are calculated by Wheel function, see below
byte colors[][8] = {
  {170, 160, 150, 140, 130, 120, 1, 1},
  {1, 5, 10, 15, 20, 25, 90, 90},
  {90, 85, 80, 75, 70, 65, 1, 1},
  {90, 90, 90, 30, 30, 30, 1, 1},
  {170, 160, 150, 140, 130, 120, 0, 0},
  {170, 160, 150, 140, 130, 120, 1, 1},
  {170, 160, 150, 140, 130, 120, 1, 1}
};

void setup() {
  
  pixel.begin();  
  pixel.setBrightness(BRIGHTNESS);

  // Begin FFT operations
  ADCSRA = 0b11100101;      // set ADC to free running mode and set pre-scalar to 32 (0xe5)
  ADMUX = 0b00000000;       // use pin A0 and external voltage reference
}

void loop() {
    
  // ++ Sampling
  for (int i = 0; i < SAMPLES; i++) {
    while (!(ADCSRA & 0x10));       // wait for ADC to complete current conversion ie ADIF bit set
    ADCSRA = 0b11110101 ;           // clear ADIF bit so that ADC can do next operation (0xf5)
    int value = ADC - 512 ;         // Read from ADC and subtract DC offset caused value    
    vReal[i] = value / 8;           // Copy to bins after compressing
    vImag[i] = 0;
  }

  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
  
  // -- FFT
  // ++ re-arrange FFT result to match with no. of columns on display (xres)
  int step = (SAMPLES / 2) / xres;
  int c = 0;
  for (int i = 0; i < (SAMPLES / 2); i += step) {
    data_avgs[c] = 0;
    for (int k = 0 ; k < step ; k++) {
      data_avgs[c] = data_avgs[c] + vReal[i + k];
    }
    data_avgs[c] = data_avgs[c] / step;
    c++;
  }

  // ++ send to display according measured value
  for (int i = 0; i <xres; i++) {
    if (EQ_ON)
      data_avgs[i] = data_avgs[i] * (float)(eq[i]) / 100; //apply eq filter
    data_avgs[i] = constrain(data_avgs[i], 0, 80);        // set max & min values for buckets to 0-80
    data_avgs[i] = map(data_avgs[i], 0, 80, 0, yres);     // remap averaged values to yres 0-8
    yvalue = data_avgs[i];
    peaks[i] = peaks[i] - 1;                              // decay by one light
    if (yvalue > peaks[i]) peaks[i] = yvalue;             //save peak if > previuos peak
    yvalue = peaks[i];
    displaycolumn = i;
    displayvalue = yvalue;
    setColumn(displaycolumn, displayvalue);               // draw buckets
  }
  pixel.show();                                           // show buckets
  displaycolorChange();                                   // check if button pressed to change color mode
}

//-----------------------------------------------------------------
// Light leds of x column according to y value
void setColumn(byte x, byte y) {
  byte led, i;

  for (i = 0; i < yres; i++) {
    led = GetLedFromMatrix(x, i); //retrieve current led by x,y coordinates
    if (peaks[x] > i) {
      switch (displaycolor) {
        case 4:
          //put zero 0 on array value to customize peaks color
          if (colors[displaycolor][i] > 0) {
            //normal color defined on color array
            pixel.setPixelColor(led, Wheel(colors[displaycolor][i]));
          }
          else {
            //custom color for peaks only with 0 on array value
            pixel.setPixelColor(led, 255, 255, 255); //Led number, R, G, B values
          }
          break;

        case 5:
          //change color by column
          pixel.setPixelColor(led, Wheel(x * 16));
          break;

        case 6:
          //change color by row          
          pixel.setPixelColor(led, Wheel(i * 36));
          break;

        default:
          //display color set -> displaycolor from 0 to 3
          //color are defined on color array
          pixel.setPixelColor(led, Wheel(colors[displaycolor][i]));
      }//END SWITCH
    }
    else {
      pixel.setPixelColor(led, 0);
    }
  }
}

//======================================================================
// Calculate a led number by x,y coordinates
// valid for WS2812B with serpentine layout placed in horizzontal
// and zero led at bottom right (input connector on the right side)
// input value: x=0-31, y=0-7, return a led number from 0 to 255
//========================================================================
byte GetLedFromMatrix(byte x, byte y) {
  x = xres - x - 1;
  if (x & 0x01) {
    //Odd columns increase backwards
    return ((x + 1) * yres - y - 1);
  }
  else {
    //Even columns increase normally
    return ((x + 1) * yres - yres + y);
  }
}
//========================================================================

void displaycolorChange() {
  int reading = digitalRead(buttonPin);
  if (reading == HIGH && previousState == LOW && millis() - lastDebounceTime > debounceDelay) // works only when pressed
  {
    displaycolor++;
    if (displaycolor > 6) displaycolor = 0;
    lastDebounceTime = millis();
  }
  previousState = reading;
}

/* Utility from Adafruit Neopixel demo sketch
   Input a value 0 to 255 to get a color value.
   The colours are a transition R - G - B - back to R.*/
unsigned long Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return pixel.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return pixel.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixel.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

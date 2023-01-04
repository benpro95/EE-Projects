/* DC PANEL METER USING ARDUINO BY A. SAMIUDDHIN */

#include <LiquidCrystal.h>  // LCD Library
LiquidCrystal lcd(7, 6, 5, 4, 3, 2); // Initialize LCD 
const int Vin = A0; // Voltage input pin
const int Ain = A1; // Current input pin
int mVperA = 185; // Output swings at the rate of 185mV per Ampere
int Isensoroffset = 2500; // Offset Voltage of the Current sensor IC ACS712
int Vinput = 0;
int Iinput = 0;
float Voltage = 0;
float Amps = 0;
float Aread = 0;
float Vsense = 0;

void setup()
{
  pinMode(12, OUTPUT); //Buzzer as Output
  lcd.begin(16,2);  // 16 Columns and 2 Rows
  delayMicroseconds(10);
  lcd.setCursor(0,0);
  lcd.print(" DC PANEL METER ");
  lcd.setCursor(0,1);
  lcd.print("By A. SAMIUDDHIN");
  delay(2000);
  Serial.begin(9600); // Initialise serial port with baud rate of 9600bps
  lcd.clear();
}

void loop()
{
  Vinput = analogRead(Vin); // Read analog pin A0
  Iinput = analogRead(Ain); // Read analog pin A1
  Vsense = (Vinput/1024.0)*5000; // Voltage Value in (0-5000) mV
  Voltage = (Vsense*6.66)/1000; // Voltage Value in (0-30) Volts
  Aread = (Iinput/1024.0)*5000; // Conversion of Current into Voltage
  Amps = ((Aread - Isensoroffset)/mVperA); // Current value in Amperes
    if((Vsense >= 4500)||(Amps >= 4.9)||(Amps <= -4.9))
      {
        digitalWrite(12, HIGH); // Turn ON the buzzer
        lcd.setCursor(0,0);
        lcd.print("->PANEL METER<- ");
        lcd.setCursor(0,1);
        lcd.print("   OVER RANGE   ");
        Serial.print("\n");
        Serial.print("OVER-RANGE ALERT\r\n");
      }
    else
      {
        digitalWrite(12, LOW); // Turn OFF the buzzer
        lcd.setCursor(0,0);
        lcd.print(" V=     ");
        lcd.setCursor(4,0);
        lcd.print(Voltage, 2); // Display the Voltage with 2 digits after the decimal point
        lcd.print("  VOLTS ");
        Serial.print("\n");
        Serial.print("V= ");
        Serial.println(Voltage, 2);
        lcd.setCursor(0,1);
        lcd.print(" A=     ");  
        lcd.setCursor(4,1);
        lcd.print(Amps, 2); // Display the Current with 2 digits after the decimal point
        lcd.print("   AMPS ");
        Serial.print("\n");
        Serial.print("A= ");
        Serial.println(Amps, 2);
        delay(300);
      }
}

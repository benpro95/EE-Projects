

/*
 * Ben Provenzano III
 * -----------------
 * October 24th 2016
 * Serial Home Automation / DAM1021 Input Controller / 433Mhz Transmitter / IR Transmitter
 * v3
 ** Patent Pending **
 *
 */
 
#include<string.h>
#include <IRremote.h>

#define dac1 2
#define dac2 9
#define ext1 4
#define ext2 5
#define ext3 6
#define tx 7

char temp;
char str[10];
char i=0;
IRsend irsend;

void setup() 
{
  Serial.begin(9600);
  delay(500);
  pinMode(dac1,OUTPUT);   
  pinMode(dac2,OUTPUT);    
  pinMode(ext1,OUTPUT);    
  pinMode(ext2,OUTPUT);    
  pinMode(ext3,OUTPUT);    
  pinMode(tx, OUTPUT); 
  delay(250);
  rxtx();
}

void loop() 
{
   if(temp==1)
   {

    // DAC Input Controller
    
    if((strncmp(str,"usb",3))==0)
    {
     digitalWrite(dac1, LOW);
     delay(10);
     digitalWrite(dac2, LOW);     
     delay(10);
     rxtx();
    }

    if((strncmp(str,"optical",7))==0)
    {
     digitalWrite(dac2, LOW);
     delay(10);
     digitalWrite(dac1, HIGH);
     delay(10);
     rxtx();
    }

    if((strncmp(str,"coax",4))==0)
    {
     digitalWrite(dac1, LOW);
     delay(10);
     digitalWrite(dac2, HIGH);
     delay(10);
     rxtx();
    }


    // External 12v Power Controller

    if((strncmp(str,"ext1on",6))==0)
    {
     digitalWrite(ext1, HIGH);
     delay(10);
     rxtx();
    }
    
    if((strncmp(str,"ext1off",7))==0)
    {
     digitalWrite(ext1, LOW);
     delay(10);
     rxtx();
    }
    
    if((strncmp(str,"ext2on",6))==0)
    {
     digitalWrite(ext2, HIGH);
     delay(10);
     rxtx();
    }
    
    if((strncmp(str,"ext2off",7))==0)
    {
     digitalWrite(ext2, LOW);
     delay(10);
     rxtx();
    }
    
    if((strncmp(str,"ext3on",6))==0)
    {
     digitalWrite(ext3, HIGH);
     delay(10);
     rxtx();
    }
    
    if((strncmp(str,"ext3off",7))==0)
    {
     digitalWrite(ext3, LOW);
     delay(10);
     rxtx();
    }


    // IR Controller "Sony Bravia TV"

    if((strncmp(str,"pwrtv",5))==0)
    {
     for (int x = 0; x < 3; x++) {
     irsend.sendSony(0xa90, 12);
     delay(10);
     rxtx();
    }
    }

    if((strncmp(str,"jumptv",6))==0)
    {
     for (int x = 0; x < 3; x++) {
     irsend.sendSony(0xdd0, 12);
     delay(10);
     rxtx();
    }
    }

    if((strncmp(str,"inputtv",7))==0)
    {
     for (int x = 0; x < 3; x++) {
     irsend.sendSony(0xa50, 12);
     delay(10);
     rxtx();
    }
    }

    if((strncmp(str,"hometv",5))==0)
    {
     for (int x = 0; x < 3; x++) {
     irsend.sendSony(0x70, 12);
     delay(10);
     rxtx();
    }
    }

    if((strncmp(str,"dwntv",5))==0)
    {
     for (int x = 0; x < 3; x++) {
     irsend.sendSony(0xaf0, 12);
     delay(10);
     rxtx();
    }
    }

    if((strncmp(str,"enttv",5))==0)
    {
     for (int x = 0; x < 3; x++) {
     irsend.sendSony(0xa70, 12);
     delay(10);
     rxtx();
    }
    }


    // IR Controller "Pre-amplifier"

    if((strncmp(str,"dwnf",4))==0)
    {
     irsend.sendNEC(0x20DFE01F, 32);
     delay(10);
     rxtx();
    }

    if((strncmp(str,"upf",3))==0)
    {
     irsend.sendNEC(0x20DF609F, 32);
     delay(10);
     rxtx();
    }

    if((strncmp(str,"pwrhifi",7))==0)
    {
     irsend.sendNEC(0x20DF10EF, 32);
     delay(10);
     rxtx();
    }

    if((strncmp(str,"mute",4))==0)
    {
     irsend.sendNEC(0x20DF906F, 32);
     delay(10);
     rxtx();
    }

    if((strncmp(str,"upc",3))==0)
    {
     irsend.sendNEC(0x20DF02FD, 32);
     delay(10);
     rxtx();
    }

    if((strncmp(str,"dwnc",4))==0)
    {
     irsend.sendNEC(0x20DF827D, 32);
     delay(10);
     rxtx();
    }

    if((strncmp(str,"menu",4))==0)
    {
     irsend.sendNEC(0x20DFC23D, 32);
     delay(10);
     rxtx();
    }

    if((strncmp(str,"backlight",9))==0)
    {
     irsend.sendNEC(0x20DFB24D, 32);
     delay(10);
     rxtx();
    }

    if((strncmp(str,"display",7))==0)
    {
     irsend.sendNEC(0x20DF55AA, 32);
     delay(10);
     rxtx();
    }

    if((strncmp(str,"dac",3))==0)
    {
     irsend.sendNEC(0x20DF8877, 32);
     delay(10);
     rxtx();
    }

    if((strncmp(str,"aux",3))==0)
    {
     irsend.sendNEC(0x20DF48B7, 32);
     delay(10);
     rxtx();
    }

    if((strncmp(str,"phono",5))==0)
    {
     irsend.sendNEC(0x20DFC837, 32);
     delay(10);
     rxtx();
    }
   
    else 
    {
     delay(50);
     rxtx();
     }
   }
}
void serialEvent()
{
   while(Serial.available())
    {
     char Inchar=Serial.read();
     str[i]=Inchar;
     i++;
     delay(10);
     if(Inchar == 0x0A)
     {
      temp=1;
      Inchar=0;
     }
    }
}
void rxtx()
{
   i=0;
   temp=0;
}

/*
  Ascending and descending LED blink
  by Ben Provenzano III  
*/


unsigned long init_time = 2000;
unsigned long tran_time = 0;
unsigned long del_time = 0;
int init_flag = 0;
int del_inc = 0;
int step_id = 0;


void setup() {
//Serial.begin(9600);
pinMode(LED_BUILTIN, OUTPUT);
}


void loop() {


if (init_flag == 0)
{
  del_time = init_time;
  tran_time = init_time * 0.5;
  init_flag = 1;
}

if (del_time <= tran_time)
{
  del_inc = 10;
}
else
{
  del_inc = 30;
}


if (del_time <= 10)
{
  step_id = 1;
}

if (del_time >= init_time)
{
  step_id = 0;
}


if (step_id == 1)
{
  del_time = del_time + del_inc;
}

if (step_id == 0)
{
  del_time = del_time - del_inc;
}


if (del_time <= 10)
{
  del_time = 10;
}


digitalWrite(LED_BUILTIN, HIGH);
delay(del_time);             
digitalWrite(LED_BUILTIN, LOW);  
delay(del_time);          
//Serial.println(del_time);

}

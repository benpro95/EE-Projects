/**********************************************/
//project: 5V realy module and LM35 sensor use 
//with light bulb connected. When the temperature rise up to
//30 degrees C or 86 degrees F the light will be on
//email: info.acoptex@gmail.com
//web: http://acoptex.com
/**********************************************/
int tempsensor = 0;
const int relayPin = 8;
const int heatPin = A0;
/**********************************************/
void setup() {
  Serial.begin(9600);
  pinMode(relayPin, OUTPUT);
}

void loop() {
  tempsensor = analogRead(heatPin);
  float value = ( tempsensor / 1023.0) * 5000;
  float celsius = value / 10;
  float fahrenheit = (celsius * 9) / 5 + 32;

  Serial.print("Temperature = ");
  Serial.print(celsius);
  Serial.print("*C ");
  Serial.print("Temperature = ");
  Serial.print(fahrenheit);
  Serial.print("*F ");
  Serial.println();

  if (celsius >= 30) { // temperature at which the light bulb will be on - you can change it to fahrenheit
    digitalWrite(relayPin, LOW);
  }
  else {
    digitalWrite(relayPin, HIGH);
  }
  if (celsius >= 35) { // temperature at which the light bulb will be on - you can change it to fahrenheit
    digitalWrite(relayPin, LOW);
  }
  else {
    digitalWrite(relayPin, HIGH);
  }
  delay(1000);// one second delay
}

/****************
*Toby LI@Aus Electronics Direct *
*info@auselectronicsdirect.com.au
*2017.9.14 *
*Copyright*
****************/
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
LiquidCrystal_I2C lcd(0x27,16,2); // set the LCD address to 0x27 for a 16 chars and 2
int inputPin = A0; // define ultrasonic signal receiver pin ECHO to D4 
int outputPin= A1; // define ultrasonic signal transmitter pin TRIG to D5 
void setup()
{
  Serial.begin(9600);
  lcd.init(); // initialize the lcd 
  lcd.backlight();
  pinMode(inputPin, INPUT); pinMode(outputPin,OUTPUT);
}
void loop() {
  int old_distance; 
  digitalWrite(outputPin, LOW); 
  delayMicroseconds(2);
  digitalWrite(outputPin, HIGH); // Pulse for 10μ s to trigger ultrasonic detection
  delayMicroseconds(10); 
  digitalWrite(outputPin, LOW); 
  int distance = pulseIn(inputPin, HIGH); // Read receiver pulse time 
  distance= distance/58; //Transform pulse time to distance 
  if(old_distance!=distance)
  {
    old_distance=distance;
    lcd.clear(); lcd.print("H = ");
    lcd.setCursor(4, 0);
    lcd.print(distance);
  }
  delay(200);
}

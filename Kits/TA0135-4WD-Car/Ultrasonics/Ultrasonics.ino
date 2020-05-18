/****************
*Toby LI@Aus Electronics Direct *
*info@auselectronicsdirect.com.au
*2017.9.14 *
*Copyright*
*
* gm310509
* --------
* 2020-05-18  Added conditional compilation to use the final project's wiring configuration.
****************/
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
LiquidCrystal_I2C lcd(0x27,16,2); // set the LCD address to 0x27 for a 16 chars and 2

// Uncomment this line to enable a configuration of the test program
// that is based upon the final project's wiring configuration.
//#define FINAL_PROJECT_CONFIG

#ifdef FINAL_PROJECT_CONFIG
int inputPin = 13; // define ultrasonic signal receiver pin ECHO to 13 as per the final project's wiring configuration
int outputPin= 12; // define ultrasonic signal transmitter pin TRIG to 12 as per the final project's wiring configuration
#else
int inputPin = A0; // define ultrasonic signal receiver pin ECHO to D4 
int outputPin= A1; // define ultrasonic signal transmitter pin TRIG to D5 
#endif
void setup()
{
  Serial.begin(9600);
  lcd.init(); // initialize the lcd 
  lcd.backlight();
  pinMode(inputPin, INPUT); pinMode(outputPin,OUTPUT);
  lcd.clear();
  lcd.print("Ultrasonics");
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
    lcd.setCursor(0,1);
    lcd.print("H = ");
    lcd.print(distance);
    lcd.print("    ");
  }
  delay(200);
}

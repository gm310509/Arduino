/****************
*Toby LI@Aus Electronics Direct *
*info@auselectronicsdirect.com.au
*2017.9.14 *
*Copyright*
****************/
#include <LiquidCrystal_I2C.h> //Include the I2C protocol LCD1602 Lib
#include <Wire.h> // Include I2C Lib
LiquidCrystal_I2C lcd(0x27,16,2); // Define LCD1602

void setup() 
{
  lcd.init(); // Initialize 
  lcd.backlight(); //Turn the Backlight on
  lcd.print("Arduino"); //Display Arduino
  lcd.setCursor(0,1);
  lcd.print("LCD Test");
} 
void loop() { }

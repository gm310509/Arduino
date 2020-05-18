/****************
*Toby LI@Aus Electronics Direct *
*info@auselectronicsdirect.com.au
*2017.9.14 *
*Copyright*
/****************/
#include <LiquidCrystal_I2C.h> //Includes I2C-LCD1602 lib
#include <Wire.h> //Includes I2C Lib 
unsigned char val;//Define val 
LiquidCrystal_I2C lcd(0x27,16,2); //Define LCD1602
void LCD1602_init(void) //Init
{
  lcd.init();
  delay(10); 
  lcd.backlight();
  lcd.clear(); 
} 
void setup()
{
  LCD1602_init();
  Serial.begin(9600); //Init Serial port rate set to 9600
  lcd.setCursor(0, 0); 
  lcd.print("Bluetooth Test ");
} 
void loop()
{
  if(Serial.available()) //Check whether received any data from Serial port which now is Bluetooth HC06 Module
  {
    val=Serial.read(); //read value to val; 
    lcd.setCursor(0, 1);
    lcd.write(val); //Display on the LCD, Only one char is shown
  }
}

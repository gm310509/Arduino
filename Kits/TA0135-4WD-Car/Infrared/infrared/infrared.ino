/****************
*Toby LI@Aus Electronics Direct *
*info@auselectronicsdirect.com.au
*2017.9.14 *
*Copyright*
****************/
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <IRremote.h>
LiquidCrystal_I2C lcd(0x27,16,2);
int RECV_PIN = 2;//12
/***************
 Up :      FF629D ;
 Back:     FFA857 ;
 Left:     FF22DD ;
 Right:    FFC23D ;
 Stop:     FF02FD ;
 1 CNY70:  FF6897 ;
 SelfRun:  FF9867 ;
**************/
IRrecv irrecv(RECV_PIN);
decode_results results; 
void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver 
  lcd.init(); // initialize the lcd 
  lcd.backlight(); 
  lcd.clear();
  lcd.setCursor(0, 0); 
  lcd.print("code:");
} 
void loop()
{
  if (irrecv.decode(&results)) 
  {
    lcd.setCursor(6, 0); 
    lcd.print("");
    lcd.setCursor(6, 0);
    lcd.print(results.value, HEX);
    Serial.println(results.value,HEX);
    irrecv.resume(); // Receive the next value
    
  }
}

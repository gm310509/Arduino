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
#include <IRremote.h>
LiquidCrystal_I2C lcd(0x27,16,2);

// Uncomment this line to enable a configuration of the test program
// that is based upon the final project's wiring configuration.

// NB: For this test, this is technically not needed as the configuration
// of the test and the final version are the same, but I've included it for
// consistency with the other test programs.
//#define FINAL_PROJECT_CONFIG

#ifdef FINAL_PROJECT_CONFIG
int RECV_PIN = 2;
#else
int RECV_PIN = 2;//12
#endif
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
  while (! Serial) {
    delay(1);
  }
  irrecv.enableIRIn(); // Start the receiver 
  lcd.init(); // initialize the lcd 
  lcd.backlight(); 
  lcd.clear();
  lcd.setCursor(0, 0); 
  lcd.print(F("code:"));
  Serial.println("IR Tester ready");
  Serial.print("Connected on: "); Serial.println(RECV_PIN);
} 
void loop()
{
  if (irrecv.decode(&results)) 
  {
    if (results.value != 0xffffffff) {  // Don't bother outputing the "repeat" code to the LCD.
      lcd.setCursor(6, 0); 
      lcd.print("");
      lcd.setCursor(6, 0);
      lcd.print(results.value, HEX);
    }
    Serial.println(results.value,HEX);
    irrecv.resume(); // Receive the next value
  }
}

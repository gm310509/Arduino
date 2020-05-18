/****************
*Toby LI@Aus Electronics Direct *
*info@auselectronicsdirect.com.au
*2017.9.14 *
*Copyright*
****************/
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
LiquidCrystal_I2C lcd(0x27,16,2);
/**************************Pin Definition*************************************/
int pinLB=2; // Define the Left Backward wheel output pin 2 
int pinLF=4; // Define the Left Forward wheel output pin 4 
int pinRB=7; // Define the Right Backward wheel output pin 7 
int pinRF=8; // Define the Right Forward wheel output pin 8 
/**************************PWM Speed Adjustment*******************************/
int Lpwm_pin = 10; //Pin 10 with PWM function Connect with EBA
int Rpwm_pin = 5;  //Pin 5  with PWM function Connect with EBB
/**************************Status flag         *******************************/
int Car_state = 0;
int Car_speed = 200;
/**************************Sub function definition****************************/
void advance() // Forward
{
  digitalWrite(pinRB,HIGH); // 使馬達（右後）動作
  digitalWrite(pinRF,LOW); 
  digitalWrite(pinLB,HIGH); //使馬達（左後）動作 
  digitalWrite(pinLF,LOW);
  Car_state = 1; 
  show_state();
}
void turnR() //Turn right 2 wheel control
{
  digitalWrite(pinRB,LOW); //left 2 wheel backward
  digitalWrite(pinRF,HIGH);
  digitalWrite(pinLB,HIGH); 
  digitalWrite(pinLF,LOW); //right 2 wheel forward
  Car_state = 4; 
  show_state();
}
void turnL() //Left turn 2 wheel control
{
  digitalWrite(pinRB,HIGH);
  digitalWrite(pinRF,LOW); //right 2 wheel forward
  digitalWrite(pinLB,LOW);
  digitalWrite(pinLF,HIGH);//left 2 wheel backward 
  Car_state = 3;
  show_state(); 
} 
void stopp() //Stop
{
  digitalWrite(pinRB,HIGH);
  digitalWrite(pinRF,HIGH);
  digitalWrite(pinLB,HIGH);
  digitalWrite(pinLF,HIGH);
  Car_state = 5; 
  show_state();
}
void back() //Backward
{
  digitalWrite(pinRB,LOW); //right 2 wheel backward
  digitalWrite(pinRF,HIGH);
  digitalWrite(pinLB,LOW); //left 2 wheel forward
  digitalWrite(pinLF,HIGH);
  Car_state = 2; 
  show_state() ;
}
void M_Control_IO_config(void) // Set Pin Mode 
{
  pinMode(pinLB,OUTPUT); // Pin 2
  pinMode(pinLF,OUTPUT); // Pin 4
  pinMode(pinRB,OUTPUT); // Pin 7
  pinMode(pinRF,OUTPUT); // Pin 8
  pinMode(Lpwm_pin,OUTPUT); // Pin 10 (PWM)
  pinMode(Rpwm_pin,OUTPUT); // Pin 5 (PWM)
} 
void Set_Speed(int Left,int Right)
{
  analogWrite(Lpwm_pin,Left);
  analogWrite(Rpwm_pin,Right); // analogRead values go from 0 to 1023, analogWrite values from 0 to 255
  show_speed(Left);
}
void show_state(void)
{ 
  lcd.setCursor(0, 1);
  lcd.print("State:"); 
  switch(Car_state)
  {
    case 1: lcd.print(" Go ");
    break;
    case 2: lcd.print(" Back ");
    break;
    case 3: lcd.print(" Left");
    break;
    case 4: lcd.print(" Right "); 
    break;
    case 5: lcd.print(" stop "); 
    break;
    default: break; 
  }
} 
void show_speed(int Car_speed)
{
  lcd.setCursor(0, 0);
  lcd.print("Speed:"); 
  switch(Car_speed)
  {
    case 100:  lcd.print(" Half ");
    break;
    case 255: lcd.print(" Full ");
    break;
    default: break; 
  }
}
void setup()
{
  M_Control_IO_config();
  delay(500); 
  lcd.init();
  lcd.backlight();
  lcd.clear();
} 
void loop() 
{
  Set_Speed(100,100);
  advance() ;
  delay(2000); turnL() ;
  delay(2000); turnR() ;
  delay(2000); back() ;
  delay(2000); stopp();
  delay(2000); Set_Speed(255,255);
  delay(2000); advance() ;
  delay(2000); turnL() ;
  delay(2000); turnR() ;
  delay(2000); back() ;
  delay(2000); stopp();
  delay(2000);
}

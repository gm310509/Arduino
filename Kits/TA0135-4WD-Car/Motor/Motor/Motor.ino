/****************
*Toby LI@Aus Electronics Direct *
*info@auselectronicsdirect.com.au
*2017.9.14 *
*Copyright*
*
* gm310509
* --------
* 2020-05-18  Added conditional compilation to
*             a) correct the reverse direction of motor running (as compared to the final project)
*             b) Use the final project's wiring configuration.
*             
****************/
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
LiquidCrystal_I2C lcd(0x27,16,2);
/**************************Pin Definition*************************************/

// The following two constants control how this test program is compiled.
// The default (original) test program is created when both of the following two
// symbol definitions are commented out.
//
// NB: There is a precedence to these symbols.
// If FINAL_PROJECT_CONFIGURATION is defined, then the wiring configuration of the final project will be used.
// If FINAL_PROJECT_CONFIGURATION is *not* defined but FINAL_PROJECT_DIRECTION is defined, then the wiring
//    configuration of the test program will be used, but the motors will run in the same direction as the final project
//    (i.e. the opposite direction of the original test program)
// Otherwise (If neither of the symbols are defined), the original test program's wiring and logic will be used (i.e. the motors
//    will run in the opposite direction of the main test program).

// Uncomment this line to enable a configuration of the test program
// that is based upon the final project's wiring configuration (but runs the motors
// in the same direction as the final project).
//#define FINAL_PROJECT_CONFIGURATION

// Uncomment this line to enable a configuration of the test program
// that is based upon the test project's wiring configuration.
//#define FINAL_PROJECT_DIRECTION

/********************************************************************/
/*      NB: Only uncomment one of the above two definitions.        */
/********************************************************************/

#if defined(FINAL_PROJECT_CONFIGURATION)
  int pinIN1=5; // Define the Left Backward wheel output pin 2 (MotorRight1)
  int pinIN2=6; // Define the Left Forward wheel output pin 4 (MotorRight2)
  int pinIN3=10; // Define the Right Backward wheel output pin 7 (MotorLeft1)
  int pinIN4=11; // Define the Right Forward wheel output pin 8 (MotorLeft2)
  int pinLB = pinIN1;
  int pinLF = pinIN2;
  int pinRB = pinIN3;
  int pinRF = pinIN4;
  const char * TEST_CONFIGURATION = "Final assembly wiring - correct directions";
  const char * TEST_CONFIGURATION_SHORT = "Final wiring";
#elif defined( FINAL_PROJECT_DIRECTION)
  int pinIN1=2; // Define the Left Backward wheel output pin 2 
  int pinIN2=4; // Define the Left Forward wheel output pin 4 
  int pinIN3=7; // Define the Right Backward wheel output pin 7 
  int pinIN4=8; // Define the Right Forward wheel output pin 8 
  int pinLB = pinIN2;
  int pinLF = pinIN1;
  int pinRB = pinIN4;
  int pinRF = pinIN3;
  const char * TEST_CONFIGURATION = "Original test wiring - correct directions";
  const char * TEST_CONFIGURATION_SHORT = "Test - correct";
#else
  int pinLB=2; // Define the Left Backward wheel output pin 2 
  int pinLF=4; // Define the Left Forward wheel output pin 4 
  int pinRB=7; // Define the Right Backward wheel output pin 7 
  int pinRF=8; // Define the Right Forward wheel output pin 8 
  const char * TEST_CONFIGURATION = "Original test wiring - reversed directions";
  const char * TEST_CONFIGURATION_SHORT = "Test - original";
#endif

#if ! defined(FINAL_PROJECT_CONFIGURATION)
/**************************PWM Speed Adjustment*******************************/
  int Lpwm_pin = 10; //Pin 10 with PWM function Connect with EBA
  int Rpwm_pin = 5;  //Pin 5  with PWM function Connect with EBB
#endif
/**************************Status flag         *******************************/
int Car_state = 0;
int Car_speed = 200;
/**************************Sub function definition****************************/
void advance() // Forward
{
#if defined(FINAL_PROJECT_CONFIGURATION)
  analogWrite(pinRB,255-Car_speed); //  使馬達（右後）動作 -> Activate the right side motors.
  analogWrite(pinRF,255); 
  analogWrite(pinLB,255-Car_speed); //使馬達（左後）動作 -> Activate the left side motors.
  analogWrite(pinLF,255);
#else
  digitalWrite(pinRB,HIGH); // 使馬達（右後）動作
  digitalWrite(pinRF,LOW); 
  digitalWrite(pinLB,HIGH); //使馬達（左後）動作 
  digitalWrite(pinLF,LOW);
#endif
  Car_state = 1; 
  show_state();
}


void turnR() //Turn right 2 wheel control
{
#if defined(FINAL_PROJECT_CONFIGURATION)
  analogWrite(pinRB,255); //left 2 wheel backward
  analogWrite(pinRF,255-Car_speed);
  analogWrite(pinLB,255-Car_speed); 
  analogWrite(pinLF,255); //right 2 wheel forward
#else
  digitalWrite(pinRB,LOW); //left 2 wheel backward
  digitalWrite(pinRF,HIGH);
  digitalWrite(pinLB,HIGH); 
  digitalWrite(pinLF,LOW); //right 2 wheel forward
#endif
  Car_state = 4; 
  show_state();
}


void turnL() //Left turn 2 wheel control
{
#if defined(FINAL_PROJECT_CONFIGURATION)
  analogWrite(pinRB,255-Car_speed);
  analogWrite(pinRF,255); //right 2 wheel forward
  analogWrite(pinLB,255);
  analogWrite(pinLF,255-Car_speed);//left 2 wheel backward 
#else
  digitalWrite(pinRB,HIGH);
  digitalWrite(pinRF,LOW); //right 2 wheel forward
  digitalWrite(pinLB,LOW);
  digitalWrite(pinLF,HIGH);//left 2 wheel backward 
#endif
  Car_state = 3;
  show_state(); 
} 


void stopp() //Stop
{
  digitalWrite(pinRB,HIGH);         // Stop is the same for all project configurations
  digitalWrite(pinRF,HIGH);
  digitalWrite(pinLB,HIGH);
  digitalWrite(pinLF,HIGH);
  Car_state = 5; 
  show_state();
}


void back() //Backward
{
#if defined(FINAL_PROJECT_CONFIGURATION)
  analogWrite(pinRB,255); //right 2 wheel backward
  analogWrite(pinRF,255-Car_speed);
  analogWrite(pinLB,255); //left 2 wheel forward
  analogWrite(pinLF,255-Car_speed);
#else
  digitalWrite(pinRB,LOW); //right 2 wheel backward
  digitalWrite(pinRF,HIGH);
  digitalWrite(pinLB,LOW); //left 2 wheel forward
  digitalWrite(pinLF,HIGH);
#endif
  Car_state = 2; 
  show_state() ;
}


void M_Control_IO_config(void) // Set Pin Mode 
{
  pinMode(pinLB,OUTPUT); // Pin 2
  pinMode(pinLF,OUTPUT); // Pin 4
  pinMode(pinRB,OUTPUT); // Pin 7
  pinMode(pinRF,OUTPUT); // Pin 8
#if ! defined(FINAL_PROJECT_CONFIGURATION)
  pinMode(Lpwm_pin,OUTPUT); // Pin 10 (PWM)
  pinMode(Rpwm_pin,OUTPUT); // Pin 5 (PWM)
#endif
}


void Set_Speed(int Left,int Right)
{
  /* In the final project configuration, the speed is controlled via PWM on IN1, IN2, IN3 and IN4 not ENA and ENB. */
#if !defined(FINAL_PROJECT_CONFIGURATION)
  analogWrite(Lpwm_pin,Left);
  analogWrite(Rpwm_pin,Right); // analogRead values go from 0 to 1023, analogWrite values from 0 to 255
#endif
//  show_speed(Left);
  Car_speed = Left;
  show_speed();
}

void show_state(void)
{ 
  lcd.setCursor(0, 1);
  lcd.print("State:"); 
  switch(Car_state)
  {
    case 1: lcd.print(" Go  ");     Serial.println("Forward");
    break;
    case 2: lcd.print(" Back ");    Serial.println("Reverse");
    break;
    case 3: lcd.print(" Left");     Serial.println("Left");
    break;
    case 4: lcd.print(" Right ");   Serial.println("Right");
    break;
    case 5: lcd.print(" stop ");    Serial.println("Stop");
    break;
    default: break; 
  }
}


//void show_speed(int Car_speed)
void show_speed()
{
  lcd.setCursor(0, 0);
  lcd.print("Speed:"); 
  Serial.print("Speed: ");
  switch(Car_speed)
  {
    case 100:
      lcd.print(" Half ");
      Serial.print("Half");
    break;
    case 255:
      lcd.print(" Full ");
      Serial.print("Full");
    break;
    default:
      lcd.print(Car_speed);
      Serial.print("unexpected");
    break; 
  }
  Serial.print(" (");  Serial.print(Car_speed);   Serial.println(")");
}


void setup()
{
  Serial.begin(9600);
  while (!Serial) {
    delay(1);
  }
  Serial.println("Motor test");
  Serial.print("using: ");
  Serial.println(TEST_CONFIGURATION);
  Serial.print("pinLB="); Serial.println(pinLB);
  Serial.print("pinLF="); Serial.println(pinLF);
  Serial.print("pinRB="); Serial.println(pinRB);
  Serial.print("pinRF="); Serial.println(pinRF);
#if ! defined(FINAL_PROJECT_CONFIGURATION)
  Serial.print("Lpwm_pin="); Serial.println(Lpwm_pin);
  Serial.print("Rpwm_pin="); Serial.println(Rpwm_pin);
#endif

  M_Control_IO_config();
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print(TEST_CONFIGURATION_SHORT);
  delay(500); 
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

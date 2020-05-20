/****************
*Toby LI@Aus Electronics Direct *
*info@auselectronicsdirect.com.au
*2017.9.14 *
*Copyright*
****************/
/*
 * This program works much better than the main program - it issues proper directives to the motors based upon the
 * position of the black line relative to the sensors.
 * 
 * However, as it stands it is too aggressive when adjusting course.
 * For example if it detects the black line to the left, it aggressively turns to the left and overshoots
 * the black line. It then gets confused as none of the sensors register the black line and so it just stops the car.
 * 
 * History
 * 2020-05-20 gm310509  Updated to get the advance to work correctly.
 *                      Added LCD support to output status of Line tracking sensors.
 */
/*
  L289N Connection:
  ENA = 5V
  ENB = 5V
  IN1 = 5
  IN2 = 6
  IN3 = 10
  IN4 = 11

  LCD1602
  SCL = SCL
  SDA = SDA

  Servo
  Signal = 9
  + = 5V
  - = GND

  Ultrasonic sensor
  Trig = 12
  Echo = 13

  IR control
  Signal = 2
*/
/***********************Motor Pin Definition*************************/
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

int MotorRight1 = 5; //IN1 PWM enabled
int MotorRight2 = 6; //IN2 PWM enabled
int MotorLeft1 = 10; //IN3 PWM enabled
int MotorLeft2 = 11; //IN4 PWM enabled
int counter = 0;
const int SensorLeft = 7;      //左感測器輸入腳
const int SensorMiddle = 4 ;   //中感測器輸入腳
const int SensorRight = 3;     //右感測器輸入腳
/***********************IRcode Definition  *************************/
long IRfront =   0x00FF629D;         //Advance
long IRback  =   0x00FFA857;         //Back
long IRturnright = 0x00FFC23D;       //Right
long IRturnleft = 0x00FF22DD;        //Left
long IRstop  =   0x00FF02FD;         //Stop
long IRcny70 =   0x00FF6897;         //CNY70 line routing
long IRAutorun = 0x00FF9867;         //Ultrasonics
//long IRturnsmallleft= 0x00FF22DD;
/*************************Define Line Track pins************************************/

int SL;    //Status of Left line track sensor
int SM;    //Status of Midd line track sensor
int SR;    //Status of Righ line track sensor
int SLPrev;    // Previous status of Left line track sensor
int SMPrev;    // Previous status of Middle line track sensor
int SRPrev;    // Previous status of Right line track sensor
/*************************Define IR pins************************************/

/*************************Init Ultrasonic sensor******************************/
int inputPin = 13 ; // Echo
int outputPin = 12; // Trig
int Fspeedd = 0; // 前方距離
int Rspeedd = 0; // 右方距離
int Lspeedd = 0; // 左方距離
int directionn = 0; // 前=8 後=2 左=4 右=6
/**************************Init Servo Pin*************************************/
int Servopin = 9; //Define the servo pin

int delay_time = 250; // 伺服馬達轉向後的穩定時間
int Fgo = 8; // 前進
int Rgo = 6; // 右轉
int Lgo = 4; // 左轉
int Bgo = 2; // 倒車
void advance(int a) //Forward
{
  digitalWrite(MotorRight1, LOW);
  digitalWrite(MotorRight2, HIGH);
  digitalWrite(MotorLeft1, LOW);
  digitalWrite(MotorLeft2, HIGH);
  delay(a * 100);
}
void right(int b) //right turn one wheel control
{
  digitalWrite(MotorLeft1, LOW);
  digitalWrite(MotorLeft2, HIGH);
  digitalWrite(MotorRight1, LOW);
  digitalWrite(MotorRight2, LOW);
  delay(b * 100);
}
void left(int c) //Left turn one wheel control
{
  digitalWrite(MotorRight1, LOW);
  digitalWrite(MotorRight2, HIGH);
  digitalWrite(MotorLeft1, LOW);
  digitalWrite(MotorLeft2, LOW);
  delay(c * 100);
}
void turnR(int d) //right turn one wheel control
{
  digitalWrite(MotorRight1, HIGH);
  digitalWrite(MotorRight2, LOW);
  digitalWrite(MotorLeft1, LOW);
  digitalWrite(MotorLeft2, HIGH);
  delay(d * 100);
}
void turnL(int e) //Left turn two wheel control
{
  digitalWrite(MotorRight1, LOW);
  digitalWrite(MotorRight2, HIGH);
  digitalWrite(MotorLeft1, HIGH);
  digitalWrite(MotorLeft2, LOW);
  delay(e * 100);
}
void stopp(int f) //stop
{
  digitalWrite(MotorRight1, LOW);
  digitalWrite(MotorRight2, LOW);
  digitalWrite(MotorLeft1, LOW);
  digitalWrite(MotorLeft2, LOW);
  delay(f * 100);
}
void back(int g) //Back
{
  digitalWrite(MotorRight1, HIGH);
  digitalWrite(MotorRight2, LOW);
  digitalWrite(MotorLeft1, HIGH);
  digitalWrite(MotorLeft2, LOW);;
  delay(g * 100);
}
void s_advance()
{
  analogWrite(MotorLeft1, 0);
//  analogWrite(MotorLeft1, 80);
  analogWrite(MotorLeft2, 150);
  analogWrite(MotorRight1, 0);
//  analogWrite(MotorRight1, 80);
  analogWrite(MotorRight2, 150);
}
void s_left()
{
  analogWrite(MotorRight1, 50);
  analogWrite(MotorRight2, 150);
  analogWrite(MotorLeft1, 0);
  analogWrite(MotorLeft2, 20);
}
void s_right()
{ analogWrite(MotorLeft1, 80);
  analogWrite(MotorLeft2, 150);
  analogWrite(MotorRight1, 0);
  analogWrite(MotorRight2, 20);
}


int updateLineTracks(int pos, int prev, int curr) {
  if (prev != curr) {
    lcd.setCursor(pos, 1);
    lcd.print(curr);
  }
  return curr;
}
void updateLineTracks() {
  SLPrev = updateLineTracks( 3, SLPrev, SL);
  SMPrev = updateLineTracks( 8, SMPrev, SM);
  SRPrev = updateLineTracks(13, SRPrev, SR);
}



/******************Initializition ****************************************/
void setup()
{
  pinMode(MotorRight1, OUTPUT);  // 5 (PWM)
  pinMode(MotorRight2, OUTPUT);  // 6 (PWM)
  pinMode(MotorLeft1,  OUTPUT);  // 10 (PWM)
  pinMode(MotorLeft2,  OUTPUT);  // 11 (PWM)
  pinMode(SensorLeft,  INPUT);    //Init left sensor
  pinMode(SensorMiddle, INPUT); //Init Middle sensor
  pinMode(SensorRight, INPUT);   //Init Right sensor
  
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("TA0135_Ver3.0");
  delay(1000);
  
  lcd.clear();
  lcd.print("Line Track test mode");
  lcd.setCursor(0,1);
        //   01234567890123
  lcd.print("SL=  SM=  SR=");
  SRPrev = SMPrev = SLPrev = -1;      // Force update of LCD status the first time.
}

//******************************************************************************(LOOP)
void loop()
{
  SL = digitalRead(SensorLeft);
  SM = digitalRead(SensorMiddle);
  SR = digitalRead(SensorRight);
  updateLineTracks();

  //right fast 20 200
  if (SM == HIGH && SR == HIGH && SL == LOW)
  {
    turnR(1);
  }
  if (SM == HIGH && SR == LOW && SL == HIGH)
  {
    turnL(1);

  }
  if (SM == HIGH && SR == LOW && SL == LOW)
  {
    // the original s_advance does not move the car.
    // This is because the function uses PWM to control the movement - which is fine.
    // But, it is called constantly inside the loop. So, the PWM is constantly being
    // reset. As a result, the PWM doesn't get a chance to "get started" during the constant
    // reinitialisation.
    // Solutions:
    // a) Call the advance method (which is very aggressive)
    // b) Add a delay after s_advance
    // c) Fix the program to only make a change if there is a change in the sensor readings (I will do this
    //    as a revision and commit it to my GitHub later).
//    s_advance();
    advance(1);
  }
  if(SM == LOW && SR == LOW && SL == HIGH)
  {
    turnL(2);
  }
  if(SM == LOW && SR == HIGH && SL == LOW)
  {
  turnR(2);
  }
  else
  {
    stopp(1);
  }

}

/****************
*Toby LI@Aus Electronics Direct *
*info@auselectronicsdirect.com.au
*2017.9.14 *
*Copyright*
****************/
#include <IRremote.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
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
**********************************************************/
/***********************Motor Pin Definition*************************/
int MotorRight1 = 5; //IN1 PWM enabled
int MotorRight2 = 6; //IN2 PWM enabled
int MotorLeft1 = 10; //IN3 PWM enabled
int MotorLeft2 = 11; //IN4 PWM enabled
int counter = 0;

/***********************IRcode Definition  *************************/
long IRfront =   0x00FF629D;         //Advance
long IRback  =   0x00FFA857;         //Back
long IRturnright = 0x00FFC23D;       //Right
long IRturnleft = 0x00FF22DD;        //Left
long IRstop  =   0x00FF02FD;         //Stop
long IRcny70 =   0x00FF6897;         //CNY70 line routing press 1
long IRAutorun = 0x00FF9867;         //Ultrasonics press 2
long IRBluetooth = 0x00FFB04F;       //Bluetooth run press 3
//long IRreserve = 0x00FF30CF//4
//long IRreserve = 0x00FF18E7//5
//long IRreserve = 0x00FF7A85//6
//long IRreserve = 0x00FF10EF//7
//long IRreserve = 0x00FF38C7//8
//long IRreserve = 0x00FF5AA5//9
//long IRreserve = 0x00FF42BD//*
//long IRreserve = 0x00FF4AB5//0
//long IRreserve = 0x00FF52AD//#
/*************************Define Line Track pins************************************/
const int SensorLeft = 7;      //Left sensor input
const int SensorMiddle = 4 ;   //Midd sensor input
const int SensorRight = 3;     //Right sensor input
int SL;    //Status of Left line track sensor
int SM;    //Status of Midd line track sensor
int SR;    //Status of Righ line track sensor
/*************************Define IR pins************************************/
const int irReceiverPin = 2; //Infrared receiver OUTPUT pin 2
IRrecv irrecv(irReceiverPin);  // define IRrecv receiver pin
decode_results results;       // Decode_results
/*************************Init Ultrasonic sensor******************************/
int inputPin = 13 ; // Echo
int outputPin = 12; // Trig
int Fspeedd = 0; // Forward dis
int Rspeedd = 0; // Right dis
int Lspeedd = 0; // Left dis
int directionn = 0; // Define 4 direction flag
int Fgo = 8; // Forward
int Rgo = 6; // Right
int Lgo = 4; // Left
int Bgo = 2; // Back
/**************************Init Servo Pin*************************************/
int Servopin = 9; //Define the servo pin
Servo myservo; // myservo
int delay_time = 250; // Stable time for servo to turn
/******************Bluetooth Init****************************************/
unsigned char Bluetooth_val;
void Bluetooth_Control()
{
  lcd.setCursor(0, 0);  //光标设置在第一行，第一列，也就是左上角
  lcd.print("BluetoothControl");
  while (Bluetooth_val != 'E') // as long as it is not exit then
  {
    if (Serial.available()) //这一句是用来判断串口是否接收到数据的标志位。
    {
      Bluetooth_val = Serial.read(); //读取（蓝牙）串口数据，并把值赋给val;
      switch (Bluetooth_val)
      {
        case '8': advance(5); Serial.print("\n Car GO "); //UP
          break;
        case '2': back(2);   Serial.print("\n Car back "); //back
          break;
        case '4': left(5);   Serial.print("\n Car Left"); //Left
          break;
        case '6': right(5);   Serial.print("\n Car Right "); //Right
          break;
        case '5': stopp(1);   Serial.print("\n Car stop "); //stop
          break;
        default: stopp(1);
          break;
      }
    }
  }
  lcd.setCursor(0, 0);  //光标设置在第一行，第一列，
  lcd.print("   Wait  Sigal");
  stopp(1);
}
/******************Initializition ****************************************/
void setup()
{
  Serial.begin(9600);
  pinMode(MotorRight1, OUTPUT);  // 5 (PWM)
  pinMode(MotorRight2, OUTPUT);  // 6 (PWM)
  pinMode(MotorLeft1,  OUTPUT);  // 10 (PWM)
  pinMode(MotorLeft2,  OUTPUT);  // 11 (PWM)
  irrecv.enableIRIn();           // Init the IR
  pinMode(SensorLeft,  INPUT);    //Init left sensor
  pinMode(SensorMiddle, INPUT); //Init Middle sensor
  pinMode(SensorRight, INPUT);   //Init Right sensor
  digitalWrite(irReceiverPin, HIGH); //Init IRreceive Pin
  pinMode(inputPin,    INPUT); // Init Ultrasonic Echo Pin
  pinMode(outputPin,   OUTPUT); //Init Ultrasonic Trig Pin
  myservo.attach(Servopin); // Define Servo pin

  lcd.init();
  lcd.backlight();
  lcd.clear();

}
//******************************************************************(Void)
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
  analogWrite(MotorLeft1, 80);
  analogWrite(MotorLeft2, 150);
  analogWrite(MotorRight1, 80);
  analogWrite(MotorRight2, 150);
}
void s_left()
{
  analogWrite(MotorRight1, 50);
  analogWrite(MotorRight2, 150);
  analogWrite(MotorLeft1, 0);
  analogWrite(MotorLeft2, 0);
}
void s_right()
{ analogWrite(MotorLeft1, 80);
  analogWrite(MotorLeft2, 150);
  analogWrite(MotorRight1, 0);
  analogWrite(MotorRight2, 0);
}
void f_left()
{
  analogWrite(MotorRight1, 20);
  analogWrite(MotorRight2, 150);
  analogWrite(MotorLeft1, 0);
  analogWrite(MotorLeft2, 0);
}
void f_right()
{ analogWrite(MotorLeft1, 20);
  analogWrite(MotorLeft2, 150);
  analogWrite(MotorRight1, 0);
  analogWrite(MotorRight2, 0);
}
void detection() //Measure the distance from 60 90 120 degree
{
  int delay_time = 250; // wait from servo to turn
  ask_pin_F(); // detect forward distance

  if (Fspeedd < 10) // less than 10cm then
  {
    stopp(1); // stop
    back(2); // go back
  }
  if (Fspeedd < 30) // less than 30cm then
  {
    stopp(1); // stop
    ask_pin_L(); // detect left dis
    delay(delay_time); // wait servo to turn
    ask_pin_R(); // detect right dis
    delay(delay_time); // wait servo to turn

    if (Lspeedd > Rspeedd) //if left dis is larger than right side
    {
      directionn = Lgo; //go left
    }

    if (Lspeedd <= Rspeedd) //or go right
    {
      directionn = Rgo;
    }

    if (Lspeedd < 15 && Rspeedd < 15)// now you understand what the car is doing
    {
      directionn = Bgo; //back
    }
  }
  else
  {
    directionn = Fgo; //Foward
  }
}
//*********************************************************************************
void ask_pin_F() // measure forward
{
  myservo.write(90);
  digitalWrite(outputPin, LOW); // send ultrasonic wave for 2μs
  delayMicroseconds(2);
  digitalWrite(outputPin, HIGH); //Set high for 10μs，at least 10μs
  delayMicroseconds(10);
  digitalWrite(outputPin, LOW); // Low
  float Fdistance = pulseIn(inputPin, HIGH); // Read the In(difference) time
  Fdistance = Fdistance / 5.8 / 10; // Translate to distance unit Centimeter）
  Serial.print("F distance:"); //Output distance
  Serial.println(Fdistance);
  Fspeedd = Fdistance; // pass the parameter to Fspeedd
}
//********************************************************************************
void ask_pin_L() // measure left
{
  myservo.write(177);
  delay(delay_time);
  digitalWrite(outputPin, LOW);
  delayMicroseconds(2);
  digitalWrite(outputPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(outputPin, LOW);
  float Ldistance = pulseIn(inputPin, HIGH);
  Ldistance = Ldistance / 5.8 / 10;
  Serial.print("L distance:");
  Serial.println(Ldistance);
  Lspeedd = Ldistance;
}
//******************************************************************************
void ask_pin_R() // measure right
{
  myservo.write(20);
  delay(delay_time);
  digitalWrite(outputPin, LOW);
  delayMicroseconds(2);
  digitalWrite(outputPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(outputPin, LOW);
  float Rdistance = pulseIn(inputPin, HIGH);
  Rdistance = Rdistance / 5.8 / 10;
  Serial.print("R distance:");
  Serial.println(Rdistance);
  Rspeedd = Rdistance;
}
//******************************************************************************(LOOP)
void loop()
{
  /*************************Init with read from sensor**********************************/
  SL = digitalRead(SensorLeft);
  SM = digitalRead(SensorMiddle);
  SR = digitalRead(SensorRight);
  //*************************  Remote control mode*****************************************
  if (irrecv.decode(&results)) // If IR receive anything successfully then analysis code
  {
    if (results.value == IRfront)//Front
    {
      advance(10);//Advance
    }
    if (results.value ==  IRback)//Back
    {
      back(10);
    }
    if (results.value == IRturnright)//Right turn
    {
      right(6);
    }
    if (results.value == IRturnleft)//left turn
    {
      left(6);
    }
    if (results.value == IRstop)//stop
    {
      stopp(1);
    }
    //************************************Track line autorun mode************************************
    if (results.value == IRcny70)
    {
      //       lcd.clear();

      while (IRcny70)
      {
        SL = digitalRead(SensorLeft);
        SM = digitalRead(SensorMiddle);
        SR = digitalRead(SensorRight);
        if (SM == HIGH)//middle sensor detects black line
        {
          if (SL == LOW && SR == HIGH) // left white and right black then turn right slowly
          {
            digitalWrite(MotorRight1, LOW);
            digitalWrite(MotorRight2, HIGH);
            analogWrite(MotorLeft1, 0);
            analogWrite(MotorLeft2, 80);
          }
          if (SR == LOW && SL == HIGH) //right black and left white then turn left slowly
          {
            digitalWrite(MotorLeft1, LOW);
            digitalWrite(MotorLeft2, HIGH);
            analogWrite(MotorRight1, 0);
            analogWrite(MotorRight2, 10);
            analogWrite(MotorLeft1, 0);
            analogWrite(MotorLeft2, 50);

          }
          else  // both white then go straight
          {

            digitalWrite(MotorLeft1, LOW);
            digitalWrite(MotorLeft2, HIGH);

            digitalWrite(MotorRight1, LOW);
            digitalWrite(MotorRight2, HIGH);
            analogWrite(MotorLeft1, 200);
            analogWrite(MotorLeft2, 200);
            analogWrite(MotorRight1, 200);
            analogWrite(MotorRight2, 200);

          }
        }
        else // middle sensor detects white
        {
          if (SL == LOW && SR == HIGH)// left white and right black turn left fast
          {

            digitalWrite(MotorRight1, LOW);
            digitalWrite(MotorRight2, HIGH);
            analogWrite(MotorLeft1, 0);
            analogWrite(MotorLeft2, 10);
            analogWrite(MotorRight1, 0);
            analogWrite(MotorRight2, 100);
          }
          else if (SR == LOW && SL == HIGH) // right white and left black turn right fast
          {

            digitalWrite(MotorLeft1, LOW);
            digitalWrite(MotorLeft2, HIGH);
            analogWrite(MotorRight1, 0);
            analogWrite(MotorRight2, 10);
            analogWrite(MotorLeft1, 0);
            analogWrite(MotorLeft2, 100);

          }
          else // all white then stop
          {

            digitalWrite(MotorLeft1, LOW);
            digitalWrite(MotorLeft2, HIGH);
            analogWrite(MotorRight1, 0);
            analogWrite(MotorRight2, 10);
            analogWrite(MotorLeft1, 0);
            analogWrite(MotorLeft2, 100);
          }
        }
        if (irrecv.decode(&results))
        {
          irrecv.resume();
          Serial.println(results.value, HEX);
          if (results.value == IRstop)
          {
            digitalWrite(MotorRight1, HIGH);
            digitalWrite(MotorRight2, HIGH);
            digitalWrite(MotorLeft1, HIGH);
            digitalWrite(MotorLeft2, HIGH);
            break;
          }
        }
      }
      results.value = 0;
    }
    //******************************Ultrasonic autorun******************************************
    if (results.value == IRAutorun ) //press 2 on the remote control
    {
      while (IRAutorun)
      {
        myservo.write(90); //Set the servo back to facing front
        detection(); //detect the distance of left/right
        if (directionn == 8)// Forward
        {
          if (irrecv.decode(&results))
          {
            irrecv.resume();
            lcd.setCursor(0, 0);
            lcd.print("Direction:");
            Serial.println(results.value, HEX);
            if (results.value == IRstop)
            {
              digitalWrite(MotorRight1, LOW);
              digitalWrite(MotorRight2, LOW);
              digitalWrite(MotorLeft1, LOW);
              digitalWrite(MotorLeft2, LOW);
              break;
            }
          }
          results.value = 0;
          advance(1); 
        }
        if (directionn == 2) //Back
        {
          if (irrecv.decode(&results))
          {
            irrecv.resume();
            Serial.println(results.value, HEX);
            if (results.value == IRstop)
            {
              digitalWrite(MotorRight1, LOW);
              digitalWrite(MotorRight2, LOW);
              digitalWrite(MotorLeft1, LOW);
              digitalWrite(MotorLeft2, LOW);
              break;
            }
          }
          results.value = 0;
          back(8);
          turnL(3); //prevent from stucking in the small corner
          //Serial.print(" Reverse ");
        }
        if (directionn == 6) //Right turn
        {
          if (irrecv.decode(&results))
          {
            irrecv.resume();
            Serial.println(results.value, HEX);
            if (results.value == IRstop)
            {
              digitalWrite(MotorRight1, LOW);
              digitalWrite(MotorRight2, LOW);
              digitalWrite(MotorLeft1, LOW);
              digitalWrite(MotorLeft2, LOW);
              break;
            }
          }
          results.value = 0;
          back(1);
          turnR(6); 
         // Serial.print(" Right ");
        }
        if (directionn == 4) //Left turn
        {
          if (irrecv.decode(&results))
          {
            irrecv.resume();
            Serial.println(results.value, HEX);
            if (results.value == IRstop)
            {
              digitalWrite(MotorRight1, LOW);
              digitalWrite(MotorRight2, LOW);
              digitalWrite(MotorLeft1, LOW);
              digitalWrite(MotorLeft2, LOW);
              break;
            }
          }
          results.value = 0;
          back(1);
          turnL(6); 
          //Serial.print(" Left "); 
        }

        if (irrecv.decode(&results))
        {
          irrecv.resume();
          Serial.println(results.value, HEX);
          if (results.value == IRstop)
          {
            digitalWrite(MotorRight1, LOW);
            digitalWrite(MotorRight2, LOW);
            digitalWrite(MotorLeft1, LOW);
            digitalWrite(MotorLeft2, LOW);
            break;
          }
        }
      }
      results.value = 0;
    }
    /******************************Bluetooth Mode***********************/
    if (results.value == IRBluetooth)
    {
      if (Serial.available()) //If bluetooth successfully receive data
      {
        Bluetooth_val = Serial.read();// start to read the first char
        if (Bluetooth_val == 'w') Bluetooth_Control(); // if read in 'w' then start control
      }
      delay(20);
    }
    else // If no command received from the Remote Control then stop
    {
      digitalWrite(MotorRight1, LOW);
      digitalWrite(MotorRight2, LOW);
      digitalWrite(MotorLeft1, LOW);
      digitalWrite(MotorLeft2, LOW);
    }
    irrecv.resume();    // Continue receive IR command and restart all over again
  }


}



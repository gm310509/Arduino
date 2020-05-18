/****************
*Toby LI@Aus Electronics Direct *
*info@auselectronicsdirect.com.au
*2017.9.14 *
*Copyright*
****************/
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// Uncomment this line to enable a configuration of the test program
// that mimics the final configurations.
#define FINAL_PROJECT_CONFIG

#ifdef FINAL_PROJECT_CONFIG
#define SensorLeft 7 //Define Left sensor input Pin6 
#define SensorMiddle 4 //Define Mid sensor input Pin9 
#define SensorRight 3 //Define Right sensor input Pin11
#else
#define SensorLeft 6 //Define Left sensor input Pin6 
#define SensorMiddle 9 //Define Mid sensor input Pin9 
#define SensorRight 11 //Define Right sensor input Pin11
#endif

unsigned char SL; //Status of Left sensor 
unsigned char SM; //Status of Middle sensor  
unsigned char SR; //Status of Right sensor 
unsigned char old_SL,old_SM,old_SR;//Last saved status 
LiquidCrystal_I2C lcd(0x27,16,2); // 定义了一个液晶
void LCD1602_init(void) 
{
  lcd.init(); 
  lcd.backlight(); 
  lcd.clear(); 
}
void Sensor_IO_Config()//TraceMode IO Init
{
  pinMode(SensorLeft,INPUT); 
  pinMode(SensorMiddle,INPUT);
  pinMode(SensorRight,INPUT);
}
void Sensor_Scan(void) // Read Status
{
  SL = digitalRead(SensorLeft);
  SM = digitalRead(SensorMiddle);
  SR = digitalRead(SensorRight);
}
void setup()
{
  LCD1602_init(); //1602 Init
  Sensor_IO_Config();
}
void loop()
{
  Sensor_Scan();//Read in 
  if(old_SL!=SL||old_SM!=SM||old_SR!=SR)// if either sensor status changed, LCD1602 would update info
  { 
    old_SL = SL;
    old_SM = SM;
    old_SR = SR;
    lcd.setCursor(0, 0); 
    lcd.print("SL="); 
    lcd.print(old_SL,DEC); 
    lcd.print(" SM =");
    lcd.print(old_SM,DEC);
    lcd.print(" SR =");
    lcd.print(old_SR,DEC);
  }
}

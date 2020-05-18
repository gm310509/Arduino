/****************
*Toby LI@Aus Electronics Direct *
*info@auselectronicsdirect.com.au
*2017.9.14 *
*Copyright*
****************/
int servopin=3;//Define the Servo Pin3 for Servo signal
int myangle;//Define Var angle
int pulsewidth;//Define Pulsewidth
int val;
void servopulse(int servopin,int myangle)//PWM modulation
{
  pulsewidth=(myangle*11)+500;//Translate to the range: 500-2480 Pulsewidth
  digitalWrite(servopin,HIGH);//Set Servo pin High 
  delayMicroseconds(pulsewidth);//Set pulsewidth microseconds
  digitalWrite(servopin,LOW);//Set Servo Pin Low 
  delay(20-pulsewidth/1000); // 20ms - Pulsewidth/1000
} 
void setup()
{
  pinMode(servopin,OUTPUT);//Set Servo Pin Outpout Mode
  Serial.begin(9600);
  Serial.println("servo=o_seral_simple ready" ) ;
}
void loop()//1 - 9 corresponds to 0 - 180^ degree
{
  val=Serial.read();//Read from Serial port
  if(val>'0'&&val<='9')
  {
    val=val-'0';
    val=val*(180/9);
    Serial.print("moving servo to ");
    Serial.print(val,DEC); 
    Serial.println();
    for(int i=0;i<=50;i++) //wait for completion
    {
     servopulse(servopin,val);
    }
  }
}

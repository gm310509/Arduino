/****************
*Toby LI@Aus Electronics Direct *
*info@auselectronicsdirect.com.au
*2017.9.14 *
*Copyright*
****************/

// Uncomment this line to enable a configuration of the test program
// that mimics the final configurations.
#define FINAL_PROJECT_CONFIG

#ifdef FINAL_PROJECT_CONFIG
int servopin = 9;   //Define the Servo Pin 9 in the final configuration for Servo signal
#else
int servopin=3;//Define the Servo Pin3 for Servo signal
#endif

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
  while (!Serial) {
    delay(1);
  }
  Serial.println("servo=o_seral_simple ready" ) ;
}
void loop()//1 - 9 corresponds to 0 - 180^ degree
{
  val=Serial.read();//Read from Serial port
  if (val == 'c' || val == 'C') {
    Serial.println("Moving servo to center 90°");
    for (int i = 0; i <= 50; i++) {
      servopulse(servopin, 90);
    }
  } else if(val>='0'&&val<='9')
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

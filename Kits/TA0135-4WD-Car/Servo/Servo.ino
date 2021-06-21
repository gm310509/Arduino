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

// Uncomment this line to enable a configuration of the test program
// that is based upon the final project's wiring configuration.
//#define FINAL_PROJECT_CONFIG

#ifdef FINAL_PROJECT_CONFIG
int servopin = 9;     // Define the Servo as Pin 9 which matches the final wiring configuration.
#else
int servopin=3;//Define the Servo Pin3 for Servo signal
#endif

int myangle;//Define Var angle
int pulsewidth;//Define Pulsewidth

void servopulse(int servopin,int myangle)//PWM modulation
{
  pulsewidth=(myangle*11)+500;//Translate to the range: 500-2480 Pulsewidth
  digitalWrite(servopin,HIGH);//Set Servo pin High 
  delayMicroseconds(pulsewidth);//Set pulsewidth microseconds
  digitalWrite(servopin,LOW);//Set Servo Pin Low 
  delay(20-pulsewidth/1000); // 20ms - Pulsewidth/1000
}


void outputHelp() {
  Serial.println("Enter 'C' to center the servo");
  Serial.println("Enter a digit (N) '0' through '9' to position the servo at N * 180/9° (i.e. N * 20°)");
  Serial.println();
}



void setup()
{
  pinMode(servopin,OUTPUT);//Set Servo Pin Outpout Mode
  Serial.begin(9600);
  while (!Serial) {
    delay(1);
  }
  Serial.println("servo=o_seral_simple ready" ) ;
  outputHelp();
}


void loop()//1 - 9 corresponds to 0 - 180^ degree
{
 char val;
  val=Serial.read();//Read from Serial port
  if (val == 'c' || val == 'C') {
    Serial.println("Moving servo to center 90°");
    for (int i = 0; i <= 50; i++) {
      servopulse(servopin, 90);
    }
  } else if(val>='0'&&val<='9')  {
    val=val-'0';
    val=val*(180/9);
    Serial.print("moving servo to ");
    Serial.print(val,DEC); 
    Serial.println("°");
    for(int i=0;i<=50;i++) //wait for completion
    {
     servopulse(servopin,val);
    }
  } else if (val != '\n' && val != '\r' && val != -1) {
    Serial.print("\nInvalid input: '"); Serial.print(val); Serial.println("'");
    outputHelp();
  }
}

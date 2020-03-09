
#define PIN_A 7
#define PIN_B 8


void ledOff() {
  digitalWrite(PIN_A, LOW);
  digitalWrite(PIN_B, LOW);    
}


void ledOn(int pin) {
  
  if (pin == PIN_A) {
    digitalWrite(PIN_A, HIGH);
    digitalWrite(PIN_B, LOW);    
  } else {
    digitalWrite(PIN_A, LOW);
    digitalWrite(PIN_B, HIGH);
  }
}


void setup() {

  Serial.begin(9600);

  while (!Serial) {
    delay(1);
  }

  pinMode(PIN_A, OUTPUT);
  pinMode(PIN_B, OUTPUT);

  ledOff();

  Serial.print("LED Pin A: "); Serial.print(PIN_A);
  Serial.print(" Pin B: "); Serial.println(PIN_B);
  
  Serial.println("Ready");
}



void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Colour 1");
  ledOn(PIN_A);
  delay(500);
  
  Serial.println("Off");
  ledOff();
  delay(500);
  
  Serial.println("Colour 2");
  ledOn(PIN_B);
  delay(500);
  
  Serial.println("Off");
  ledOff();
  delay(250);

  Serial.println("Both");
  for (int i = 0; i< 500; i++) {
    ledOn(PIN_A);
    delay(1);
    ledOn(PIN_B);
    delay(1);
  }

  Serial.println("Off");
  ledOff();
  delay(250);
}

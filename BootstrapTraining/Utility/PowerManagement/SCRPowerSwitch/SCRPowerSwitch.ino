#define POWER_CONTROL_PIN   2
#define POWER_INDICATOR_PIN 3
#define BUTTON_PIN          8
#undef RUSSELL_ANSWER
#define ANDYC_ANSWER

void power(int status) {
  digitalWrite(POWER_CONTROL_PIN, status);      // Turn on the transistor to maintain the power flow.
  digitalWrite(POWER_INDICATOR_PIN, status);
  Serial.print("Power set to: ");
  if (status) {
    Serial.println("HIGH");
  } else {
    Serial.println("LOW");
  }
}

void setup() {
  // put your setup code here, to run once:

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(POWER_CONTROL_PIN, OUTPUT);
  pinMode(POWER_INDICATOR_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  power(HIGH);
  
  Serial.begin(9600);
  while (!Serial) {
    delay(10);
  }
  Serial.println("Arduino Controlled power switch - Ready");
#ifdef RUSSELL_ANSWER
  Serial.println("Russel McMahaon's version");
#endif
#ifdef ANDYC_ANSWER
  Serial.println("Andy C's version");
#endif
}

// Rapidly blink the inbuilt LED to show that the Arduino is "alive"
void blinkLED(int duration) {
  digitalWrite(LED_BUILTIN, HIGH);
  delay (duration);
  digitalWrite(LED_BUILTIN, LOW);
  delay (duration);
}

void loop() {

  Serial.println("Operations mode - rapid blink");
  for (int i = 0; i < 10; i++) {
    blinkLED(150);
  }

  Serial.println("Shutting down mode");
  digitalWrite(LED_BUILTIN, HIGH);            // Signal that we are about to shutdown by holding the inbuilt LED on for two seconds.
  delay (2000);
  if (digitalRead(BUTTON_PIN) == LOW) {
    power(LOW);                                 // Turn off the transistor to terminate power flow.
  } else {
    Serial.println("Button pressed, remaining on");
  }

  digitalWrite(LED_BUILTIN, LOW);             // We should never get here ('cos the power has been turned off), but just in case,
  delay (250);                                // repeat the rapid led blinking to show that we are still "alive".
}

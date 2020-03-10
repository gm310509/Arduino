#define POWER_CONTROL_PIN   7


void setup() {
  // put your setup code here, to run once:

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(POWER_CONTROL_PIN, OUTPUT);
  digitalWrite(POWER_CONTROL_PIN, HIGH);      // Turn on the transistor to allow power to flow.

  Serial.begin(9600);
  while (!Serial) {
    delay(10);
  }
  Serial.println("SCR Controlled power switch - Ready");
}

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
  digitalWrite(LED_BUILTIN, HIGH);
  delay (2000);
  digitalWrite(POWER_CONTROL_PIN, LOW);      // Turn on the transistor to allow power to flow.
  digitalWrite(LED_BUILTIN, LOW);
  delay (250);
}

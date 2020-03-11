#define POWER_CONTROL_PIN   2

void setup() {
  // put your setup code here, to run once:

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(POWER_CONTROL_PIN, OUTPUT);
  digitalWrite(POWER_CONTROL_PIN, HIGH);      // Turn on the transistor to maintain the power flow.

  Serial.begin(9600);
  while (!Serial) {
    delay(10);
  }
  Serial.println("Arduino Controlled power switch - Ready");
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
  digitalWrite(POWER_CONTROL_PIN, LOW);       // Turn off the transistor to terminate power flow.

  digitalWrite(LED_BUILTIN, LOW);             // We should never get here ('cos the power has been turned off), but just in case,
  delay (250);                                // repeat the rapid led blinking to show that we are still "alive".
}

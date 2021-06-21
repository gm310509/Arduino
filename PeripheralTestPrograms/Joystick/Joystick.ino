/*
 * Joystick
 * ========
 * 
 * Test program for the Joystick
 * 
 */

// If unset, output is suitable for the Serial Plotter.
//#define OUTPUT_SERIAL_MONITOR

#define JOYSTICK_X  A0
#define JOYSTICK_Y  A1
/* NB: The button is connected to A2, not DIO Pin 2 */
#define JOYSTICK_BUTTON A2

#define JOYSTICK_READ_INTERVAL  250

void setup() {

  Serial.begin(115200);
  while (!Serial) {
    delay(1);
  }
#ifdef OUTPUT_SERIAL_MONITOR
  Serial.println("Joystick test program");
#else
  Serial.println("X Y B");
  Serial.println("0 511 1023");     // Set the scale on the plotter with some dummy values
#endif
  pinMode (JOYSTICK_BUTTON, INPUT_PULLUP);
}

unsigned long lastRead = 0;


void loop() {

  unsigned long _now = millis();

  if (_now - lastRead >= JOYSTICK_READ_INTERVAL) {
    lastRead = _now;
    int x = analogRead(JOYSTICK_X);
    int y = analogRead(JOYSTICK_Y);
    int b = digitalRead(JOYSTICK_BUTTON);
#ifdef OUTPUT_SERIAL_MONITOR
    Serial.print("(X, Y, B) = (");
      Serial.print(x); Serial.print(", ");
      Serial.print(y); Serial.print(", ");
      Serial.print(b); Serial.println(")");
#else
      Serial.print(x); Serial.print(" ");
      Serial.print(y); Serial.print(" ");
      Serial.println(b *1023);
#endif
  }
}

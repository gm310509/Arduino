/***************************************
 * 0300-LED-Direct-Cntr
 * --------------------
 * 
 * Test program to drive an LED display directly connected to the Arduino's digital I/O's
 * A strobing mechanism is used to drive the display which is assumed to be a 4 digit,
 * 7 segment, common cathode display.
 * 
 * It is designed to work on an Arduino Uno only as the program uses I/O ports. The configuration
 * of I/O ports to pins seems to vary from one Arduino to another.
 * 
 * The display is connected as follows:
 * 
 * Pin 13    PortB.5        Digit 4 (Left most / thousands) cathode
 * Pin 12    PortB.4        Digit 3 (hundreds) cathode
 * Pin 11    PortB.3        Digit 2 (tens) cathode
 * Pin 10    PortB.2        Digit 1 (Right most / Units) cathode.
 * 
 * Pin 8     PortB.0        G segment
 * Pin 7     PortD.7        F segment
 * Pin 6     PortD.6        E segment
 * Pin 5     PortD.5        D segment
 * Pin 4     PortD.4        C segment
 * Pin 3     PortD.3        B segment
 * Pin 2     PortD.2        A segment
 * 
 * Optionally, you can connect this also
 * Pin 9     PortB.1        DP segment
 * However, it has not been tested.
 * 
 * 
 * This program works by setting the data to be displayed in a buffer.
 * A strobeLED routine is called as part of the main look to update the display.
 */


void setup() {
  Serial.begin(9600);
#ifdef ARDUINO_AVR_LEONARDO
  int tOut = 200;    // Wait up to 2000 ms (2 seconds) for the Serial port to initialise
  while (tOut && !Serial) {
    tOut--;
    delay(10);
  }
  Serial.println("Leonardo Serial initialisation complete.");
#else
  Serial.println("Not Leonardo");
#endif

  for (int i = 0; i < 8; i++) {       // Initialise the 7 segments + 1 Decimal point...
    pinMode(2 + i, OUTPUT);           // ...  for output.
    digitalWrite(2 + i, HIGH);        // turn them off.
  }

  for (int i = 0; i < 4; i++) {
    pinMode(10 + i, OUTPUT);
    digitalWrite(10 + i, LOW);
  }

}

void output (int c, int b, int a) {
  digitalWrite(10, a);
  digitalWrite(11, b);
  digitalWrite(12, c);
  delay(500);
}

void loop() {
  // put your main code here, to run repeatedly:

  output(0,0,0);
  output(0,0,1);
  output(0,1,0);
  output(0,1,1);
  output(1,0,0);
  output(1,0,1);
  output(1,1,0);
  output(1,1,1);

}

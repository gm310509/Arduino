/* LED With push button
 *  
 *  This is a simple program that illustrates.
 *  - How to initialise digital I/O PINS for intput or output.
 *  - How to use the Serial object for debugging.
 *  - How to blink an LED the easy way.
 *  
 *  Some important things to note:
 *  - A current limiting resistor should be used to manage the power going to the LED.
 *    - So as not to overload the LED
 *    - So as to allow a balanced distribution of power throughout "the system".
 *  - Depending upon the Arduino, the initialisation of Serial should be more sophisticated.
 *    - Hardware based Serial's usually initialise straightaway - no matter what (no need for loop).
 *    - External USB h/w (i.e. Software) based Serial's (e.g. Leonardo) attempt to establish a connection with the host
 *      - This can take a little while, so there is a need to wait until it is ready (needs the loop).
 *      - When running standalone, it is not possible to establish a connection (needs the "timeout").
 *    A good generalised routine is as follows:
 
  Serial.begin(9600);
  int tOut = 20;    // Wait up to 20 ms for the Serial port to initialise
  while (tOut && !Serial) {
    tOut--;
    delay(1);
  }
 
 *    I will use the above initialisation goining forward (and possible enhance it even more).
 */

#define LED 8
#define SWITCH 2



void setup() {
  pinMode(SWITCH, INPUT);
  pinMode(LED, OUTPUT);
  Serial.begin(9600);
  Serial.println("Boot 0 - LED Blink");
  Serial.println("Ready...");
}


void loop() {
  Serial.println("LED on");
  digitalWrite (LED, HIGH);
  delay(500);

  Serial.println("LED off");
  digitalWrite (LED, LOW);
  delay(500);
}

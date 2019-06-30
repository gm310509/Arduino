/* LED With push button
 *  
 *  This is a simple program that illustrates pushing a button turns on the LED.
 *  
 *  Some important things to note:
 *  - A pull down resistor is used to prevent the input from floating when the
 *    button is not pressed.
 *  - There is an up to 2 second lag between pressing the button and the LED responding.
 */

#define LED 8         // The pin for the LED
#define SWITCH 2      // The pin for the SWITCH / Push button / wires shorted together


void setup() {
  pinMode(SWITCH, INPUT);
  pinMode(LED, OUTPUT);

  Serial.begin(9600);
  int tOut = 20;    // Wait up to 20 ms for the Serial port to initialise
  while (tOut && !Serial) {
    tOut--;
    delay(1);
  }
  Serial.println("Boot 0 - LED controlled by switch");
  Serial.println("Ready...");
}


void loop() {
  int reading = digitalRead(SWITCH);
  if (reading == HIGH) {
    digitalWrite(LED, HIGH);
    Serial.println("LED On");
  } else {
    digitalWrite(LED, LOW);
    Serial.println("LED Off");
  }

  /*********************************************************************
   * This delay seems to, errr, delay things....
   * What happens if you comment it out?
   * 
   * Try it. But before you do, can you guess what the result will be???
   *********************************************************************/
  delay(2000);
}

/* LED With push button
 *  
 *  This is a simple program that illustrates pushing a button turns on the LED.
 *  It is an extension from the previous program in that it tracks the state of the
 *  button and *only* alters the LED status if it detects a change in the state
 *  of the button (i.e. was pressed - now released OR was released - now pressed).
 *  
 *  This resolves the problems observed in the previous program of either:
 *  - processing being delayed by the use of delay().
 *    and/or
 *  - Continous messages being displayed on the Serial Monitor
 *  
 *  About delay():
 *  - Generally delay is not a good way of introducing delays.
 *  - delay() basically is an "infinite loop" that runs for *roughly* the
 *    number of milliseconds that you requested.
 *  - Very little "other stuff" can happen when a delay is active - as can be seen
 *    in the "response time" of the LED.
 *
 *  In this program:
 *  - status variables are used to track the previous state of the push button.
 *  - If the status of the push button changes, the LED is switched to the new state.
 *  
 *  Some important things to note:
 *  - Sometimes when you press (or release) the button, there are multiple messages displayed...
 *    - Why?
 *  - To dramatically see the effect, try bypassing the switch by touching the 
 *    +5 wire (VCC) to the (button side) of the pull down resistor.
 *  - This is known as bounce and is caused by a variety of things,
 *    - Switch being dirty or rough and actually bouncing off and on at the points of contact.
 *    - A charge being built up which is enough to register momentarily as a value (e.g. closed)
 *      and then not being measurable as a value (i.e. open).
 *      This is sort of like a spark, but so tiny and short you can not see it.
 */

#define LED 8
#define SWITCH 2

int prevButtonState;

void setup() {
  Serial.begin(9600);
  int tOut = 20;    // Wait up to 20 ms for the Serial port to initialise
  while (tOut && !Serial) {
    tOut--;
    delay(1);
  }

  Serial.println("Boot 0 - LED controlled by switch - with state");
  
  pinMode(SWITCH, INPUT);
  pinMode(LED, OUTPUT);
  prevButtonState = digitalRead(SWITCH);
  Serial.println("Ready...");
}


void loop() {
  int reading = digitalRead(SWITCH);
  if (reading != prevButtonState) {
    digitalWrite(LED, reading);
    prevButtonState = reading;
      Serial.print("LED ");
      Serial.println(reading ? "On" : "Off");
  }
}

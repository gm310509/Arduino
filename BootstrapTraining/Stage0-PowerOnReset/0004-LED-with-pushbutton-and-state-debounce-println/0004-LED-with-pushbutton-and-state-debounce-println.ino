/* LED With push button
 *  
 *  This is a simple program that illustrates pushing a button turns on the LED.
 *  It is an extension from the previous one in that it manages the bounce that is
 *  occuring when we press (or release the switch).
 *  
 *  Debouncing works as follows:
 *  - There is a new variable introduced "officialButtonState".
 *    This reflects the official state of the button as pressed or not pressed.
 *    It also reflects the current state of the LED. That is, "officialButtonState" is regarded as being
 *    pressed (a HIGH value) when the LED is on and not pressed (a LOW value) when the LED is off
 *    
 *  The algorithm is:
 *  - read the current state of the button and compare it to the state when we last checked (prevButtonState).
 *  - If there is a difference between the current reading and the last checked state,
 *    Take a note of the time in milliseconds (switchTime).
 *  - If the current reading of the button is different to the currently tracked state of the button (officialButtonState)
 *    *and* the DEBOUNCE_TIME_MS has passed, then
 *    - determine that the current reading is the new official state of the button.
 *    - switch the LED to reflect the current official state of the button.
 *  - Otherwise the state of the button is no longer different to the official state,
 *    or insufficient time has passed to determine if there has been an official change in the state of the button,
 *    so, do nothing just now.
 *  
 *  I've also introduced conditional compilation.
 *  - Only Leonardo (I think) uses an external USB for managing the Serial communications.
 *  - Therefore only Leonardo needs the delay with timeout for initialising the
 *    Serial communications.
 *    On other Arduinos, this is a waste of program memory (not that it is very much).
 *  - So, it is conditionally compuiled.
 *  
 *  Some important things to note:
 *  - Conditional compilation of Serial initialisation code.
 *  - There are two checks to see if the button has changed.
 *    - 1) did the switch change its state from the very last time we checked (prevButtonState if)
 *    - 2) If sufficient time has passed from 1) is the switch state the opposite of the "official state"
 *         of the switch as reflected by the LED?
 *  
 */

#define LED 8
#define SWITCH 2

#define DEBOUNCE_DELAY_MS 10    // Delay time to debounce switch.

int prevButtonState;
int pressCount = 0;
int releaseCount = 0;
unsigned long msCount = 0;      // Initialising this to zero should force a message
                                // to be displayed on the first call to loop.
unsigned long switchTime;
int officialButtonState;

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


  Serial.println("Boot - LED controlled by switch - with debounce");
  
  pinMode(SWITCH, INPUT);
  pinMode(LED, OUTPUT);
  prevButtonState = LOW;
  officialButtonState = prevButtonState;
  Serial.println("Ready");
}


void loop() {
    // Read the switch.
  int reading = digitalRead(SWITCH);
    // Has it changed since the last time loop was called?
  if (reading != prevButtonState) {
    // Yes, record the time.
    switchTime = millis();
  }
    // Remember the state of the switch in this iteration of the loop.
    // We will check it again in the if statement above to see if the timer
    // must be reset.
  prevButtonState = reading;

  if (reading != officialButtonState && millis() - switchTime > DEBOUNCE_DELAY_MS) {
    officialButtonState = reading;
    
    digitalWrite(LED, reading);
    if (reading == HIGH) {
      pressCount += 1;      
    } else {
      releaseCount += 1;
    }
  }
  
  if (millis() > msCount) {
    Serial.print("Press count = ");
    Serial.print(pressCount);
    Serial.print(" release count = ");
    Serial.println(releaseCount);
    msCount = millis() + 1000;
  }
}

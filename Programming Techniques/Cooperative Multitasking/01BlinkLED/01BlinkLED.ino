/******************************************************************************
 * Cooperative Multitasking
 *   01 - Blink LED
 * 
 * This is the first in a series of programs to illustrate the benefits of
 * a simple multitasking mechanism for Arduino.
 * 
 * This program (sketch) starts with the most basic non-cooperative multitasking example.
 * There are two subtasks:
 * - A "blink LED" task that blinks the LED. It will hold the CPU for the entire duration of a single
 *   LED blink operation.
 * -  A "check for button press" task. It will hold the CPU for as long as you hold the button down.
 * 
 * When running this program we will note that:
 * a) The messages relating to the LED Blinking will continue to be displayed - even if the button is pressed
 *    until such time as the blink operation completes.
 * b) The messages relating to the LED Blinking and the blinking itself will cease being displayed if the
 *    button is pressed (once the "check button press" task gets a hold of the CPU).
 * 
 * In short, neither routine is sharing the CPU (blinking stops during button press), even though the
 * "check button press" task isn't actually doing anything except waiting for the button to be released.
 * Similarly, the "Blink LED" task isn't doing anything useful once it turns the LED off or on. Indeed it
 * justs wastes CPU cycles courtesy of the "delay" function calls.
 * 
 * In this simplistic example, we could code it differently to avoid much of the "system hanging" symptoms,
 * but that is not the purpose this example. The purposes of this example is to show how easily it is to
 * code a routine that is "blocking" other tasks from doing their thing.
 * 
 * By the time we get to example 5, we will address this. Examples 2 and 3 build upon this example, but
 * still in a non-cooperative multi-tasking way.
 */

// Define the pin for the input button
#define BUTTON_PIN 2
// define the LED PIN for blinking
#define LED_PIN       6


void setup() {

  Serial.begin (9600);
  while (!Serial)
  ;

  Serial.println("Blinker will be on pin: ");
  Serial.println(LED_PIN);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);     // Turn the LED off.

  // Set the push button's pin as an input.
  Serial.print("Setting input for push button on pin: ");
  Serial.println(BUTTON_PIN);
  pinMode(BUTTON_PIN, INPUT);
  
  Serial.println("Ready");
}


/**********************
 * Check Button Pressed.
 * Checks to see if the button is pressed.
 * If it is, return true.
 * 
 * The function will first check to see if the button is pressed, then wait for a bit to check
 * if the button is still pressed. If it is, then the button is "debounced" and this function
 * returns true (Button pressed).
 * 
 * Otherwise this function returns false (Button not pressed).
 * 
 * Return: true if button pressed, false otherwise.
 * 
 */
boolean checkButtonPressed() {
  Serial.println("Check button press");
        // The complexity of this code is to "debounce" the button press.
        // Has the button been pressed.
  if (digitalRead(BUTTON_PIN) == HIGH) {
    // Check for a short period of time, that the button remains pressed.
    // In this case 50 x 1 ms checks.
    const int numChecks = 50;
    int i = 0;
    while (digitalRead(BUTTON_PIN) == HIGH && i < numChecks) {
      delay(1);
      i++;
    }
    // Did we exit the loop before the required time (i.e. was the button released / still being debounced)?
    if (i < numChecks) {
      return false;       // Return "Button not pressed"
    }

    // At this point, we confirm that we have a button press.
    // Wait for the button to be released. During this time, whatever else was happening
    // on the Arduino (e.g. blinking an LED) will be suspended.
    Serial.println("Button pressed, waiting for release");
    while (digitalRead(BUTTON_PIN) == HIGH) {
      delay(10);
    }
    return true;          // Return "Button pressed"
  }
  return false;           // Return "Button not pressed"
}

/**********************
 * Blink LED
 * 
 * Blink the LED a single time.
 * 
 */
void blinkLed() {
  Serial.println("LED On");
  digitalWrite(LED_PIN, LOW);
  delay(2000);

  Serial.println("LED Off");
  digitalWrite(LED_PIN, HIGH);
  delay(2000);
}

/**********************
 * Loop
 * 
 * Continuously call the active routine (chase of blink LED).
 * Upon completion of the active routine, check to see if the button has been pressed.
 * If it has, print a message.
 */
void loop() {
  // Blink our LED for 2 seconds on, 2 seconds off.
  blinkLed();
  if(checkButtonPressed()) {
    Serial.println("Button was pressed (now it is released).");
  }
}

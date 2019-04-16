/******************************************************************************
 * Cooperative Multitasking
 *   02 - LED Chasser
 * 
 * This is the second in a series of programs to illustrate the benefits of
 * a simple multitasking mechanism for Arduino.
 * 
 * This program (sketch) builds upon the blink programs by combining converting it into a chaser.
 * There are two subtasks:
 * - A "chase LED" task that causes each LED to light up one after the other. It will hold the CPU for 
 *   the entire duration of a single chase operation. That is, individually light up the led's in one direction
 *   then the other.
 * -  A "check for button press" task. It will hold the CPU for as long as you hold the button down.
 * 
 * When running this program we will note that:
 * a) The messages relating to the Chase operation will continue to be displayed - even if the button is pressed
 *    until such time as the chase operation completes.
 * b) The messages relating to the LED Chase and the chase itself will cease being displayed if the
 *    button is pressed (once the "check button press" task gets a hold of the CPU).
 * 
 * In short, neither routine is sharing the CPU (chase stops during button press), even though the
 * "check button press" task isn't actually doing anything except waiting for the button to be released.
 * Similarly, the "Chase LED" task isn't doing anything useful once it turns the LED off or on. Indeed it
 * justs wastes CPU cycles courtesy of the "delay" function calls.
 * 
 * In this simplistic example, we could code it differently to avoid much of the "system hanging" symptoms,
 * but that is not the purpose this example. The purposes of this example is to show how easily it is to
 * code a routine that is "blocking" other tasks from doing their thing.
 * 
 * By the time we get to example 5, we will address this. Examples 2 and 3 build upon this example, but
 * still in a non-cooperative multi-tasking way.
 * 
 */

// Define the pin for the input button
#define BUTTON_PIN 2

// Define the pins to be used in tracing mode.
unsigned int ledPins [] = {6, 7, 8, 9, 10, 11, 12, 13};

void setup() {

  Serial.begin (9600);
  while (!Serial)
  ;

  // Initialise the LED Pins for output and set them to High (turn the LED's off).
  Serial.print("Initialising tracer pin: ");
  for (int i = 0; i < sizeof (ledPins) / sizeof(ledPins[0]); i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], HIGH);     // Turn the LED off.
    if (i > 0) {
      Serial.print(", ");
    }
      Serial.print(ledPins[i]);
  }
  Serial.println();
  
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
 * Otherwise this function returns false (Button not pressed)
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
 * Tracer
 * 
 * Cause the LED's to chase one another along the list of pins defined in "ledPins".
 * The chase will go from "left to right" as defined by the order of the pins in "ledPins". Then
 * "right to left".
 * 
 * The chase will be executed one time.
 */
void tracer() {
  Serial.println("Tracer - up");
  for (int i = 1; i < sizeof (ledPins) / sizeof(ledPins[0]); i++) {
    digitalWrite(ledPins[i - 1], HIGH);   // Off
    digitalWrite(ledPins[i], LOW);        // On
//    Serial.print(ledPins[i]);
//    Serial.println(" On");
    delay(250);
  }
  
  Serial.println("Tracer - down");
  for (int i = sizeof (ledPins) / sizeof(ledPins[0]) - 2; i >= 0; i--) {
    digitalWrite(ledPins[i + 1], HIGH);   // Off
    digitalWrite(ledPins[i], LOW);        // On
//    Serial.print(ledPins[i]);
//    Serial.println(" On");
    delay(250);
  }
}

/**********************
 * Loop
 * 
 * Continuously call the active routine (chase of blink LED).
 * Upon completion of the active routine, check to see if the button has been pressed.
 * If it has, switch modes.
 */
void loop() {
  tracer();
  if (checkButtonPressed()) {
    Serial.println("Button was pressed (now it is released).");
  }
}

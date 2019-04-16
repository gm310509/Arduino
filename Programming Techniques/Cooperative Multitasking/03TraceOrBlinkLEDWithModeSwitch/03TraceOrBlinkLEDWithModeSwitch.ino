/******************************************************************************
 * Cooperative Multitasking
 *   03 - Blink and Trace
 * 
 * This is the third in a series of programs to illustrate the benefits of
 * a simple multitasking mechanism for Arduino.
 * 
 * This program (sketch) builds upon the blink and trace programs by combining them into one.
 * Additionally the button's function has been altered to do something useful.
 * The button will cause a switch between the two modes. That is, if the program is blinking an LED,
 * a button press will switch to chase mode.
 * 
 * The desired outcome of this program is to verify that the LED's are working and illustrate the
 * effect of the sequential processing when multi-tasking is not enabled.
 * Specifically: 
 * - Button presses are only detected when the program polls the button.
 *   Otherwise button presses are simply ignored.
 * - If the button is pressed and detected by the Arduino, all other activity will cease
 *   i.e. the LED's will freeze.
 *   
 * Of interest, but outside the scope of this activity, we could still achieve the desired result
 * of allowing the LED's to continue blinking and detect the button press without two much complicating code.
 * This would require the use of interrupts and toggling the mode of operation in the Interrupt 
 * Service Routine. If I get the time do a preemptive multi-tasking instructable, we will cover this
 * capability there.
 * 
 */

// Define the pin for the input button
#define BUTTON_PIN 2
// define the LED PIN for blinking
#define LED_PIN       ledPins[0]

// Define the pins to be used in tracing mode.
unsigned int ledPins [] = {6, 7, 8, 9, 10, 11, 12, 13};
// Start off in the tracer mode (because it's slightly cooler than just blinking).
boolean tracerMode = true;

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
  
  Serial.println("Blinker will be on pin: ");
  Serial.println(LED_PIN);

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
  // Blink our LED for 2 seconds on, 2 seconds off.
  if (tracerMode) {
    tracer();
  } else {
    blinkLed();
  }
  if (checkButtonPressed()) {
    tracerMode = ! tracerMode;
    Serial.print("Switching modes: tracermode = ");
    Serial.println(tracerMode);
  }
}

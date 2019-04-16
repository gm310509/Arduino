/******************************************************************************
 * Cooperative Multitasking
 *   04 - Interleaved LED Blinking
 * 
 * This is the fourth in a series of programs to illustrate the benefits of
 * a simple multitasking mechanism for Arduino.
 * 
 * This program (sketch) illustrates the complexity of managing multiple independent
 * tasks (blinking of LED's).
 * 
 * When *modifying* this program note that:
 * a) a change to the cycle of one LED, requires recalculating aspects of the neighbouring LED activity.
 * b) if not impossible, it is very difficult to manage the on/off sequence if the LED's blink at different
 *    rates and their cycles differ in duration with this single threaded approach.
 * 
 * In example 5, we will enable multitasking to overcome the above problems. Hopefully, you will agree that
 * the solution is relatively simple - you may have even done something along similar lines. In example 5,
 * we will blink the LED's at randomly determined intervals.
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

#define LED_A   ledPins[0]
#define LED_B   ledPins[1]
#define LED_C   ledPins[2]


/**********************
 * LED Cycle Z
 * 
 * Blink the LED's so that they are on for 2 seconds and off for 2 seconds.
 * LED A (ledPins[0]) comes on first.
 * LED B (ledPins[1]) comes on 500ms after A.
 * LED C (ledPins[2]) comes on 500ms after B.
 * After another 1 second:
 * LED A turns off.
 * 500ms later LED B turns off.
 * 500ms later LED C turns off.
 * After another 1 second, the sequence is complete, so control is returned.
 */
void cycleA() {
  Serial.println("Cycle A");
  digitalWrite(LED_A, HIGH);
  delay (500);
  digitalWrite(LED_B, HIGH);
  delay (500);
  digitalWrite(LED_C, HIGH);
  delay (1000);
  
  digitalWrite(LED_A, LOW);
  delay (500);
  digitalWrite(LED_B, LOW);
  delay (500);
  digitalWrite(LED_C, LOW);
  delay (1000);
}


void cycleB() {
  Serial.println("Cycle B");
  digitalWrite(LED_A, HIGH);
  delay (750);
  digitalWrite(LED_B, HIGH);
  delay (750);
  digitalWrite(LED_C, HIGH);
  delay (500);
  
  digitalWrite(LED_A, LOW);
  delay (750);
  digitalWrite(LED_B, LOW);
  delay (750);
  digitalWrite(LED_C, LOW);
  delay (500);
}


boolean modeA = true;
/**********************
 * Loop
 * 
 * Continuously call the active routine (chase of blink LED).
 * Upon completion of the active routine, check to see if the button has been pressed.
 * If it has, switch modes.
 */
void loop() {
  if (modeA) {
    cycleA();
  } else {
    cycleB();
  }
  if (checkButtonPressed()) {
    Serial.println("Button was pressed (now it is released).");
    modeA = ! modeA;
  }
}

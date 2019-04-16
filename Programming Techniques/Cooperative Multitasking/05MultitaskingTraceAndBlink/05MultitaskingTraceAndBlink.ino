/******************************************************************************
 * Cooperative Multitasking
 *   05 - Multitasking trace and blink
 * 
 * This is the fifth in a series of programs to illustrate the benefits of
 * a simple multitasking mechanism for Arduino.
 * 
 * This program (sketch) shows how to multitask a few different tasks using "cooperative
 * multitasking". The tasks that are being multitasked include:
 * - Monitoring a switch / button press
 * - Executing an LED trace pattern
 * - Randomly blinking multiple LEDs.
 *   The blink task is actually 1 task per LED being blinked. So for 8 LED's, it
 *   is 8 sub tasks.
 * 
 * The big differences between the previous non-multitasking examples and this one include:
 * - use of delay () and long running loops (e.g. while (digitalRead(BUTTON_PIN) == HIGH);) are
 *   removed.
 * - The tasks track what they are doing in what is known as it's context and just let the hardware
 *   get on with doing what it does all by itself.
 *   For example, once an LED is turned ON (or OFF), it will remain on (or off) all by itself.
 *   While the LED is remaining on (or off) all by itself, the tasks' code simply returns (exits). 
 *   This allows the Arduino's CPU to go on and do something else (e.g. turn a different LED on or off).
 *   The task's context consists of whatever information is needed to track what we have done so far and
 *   work out what to do next.
 */

// Define the pin for the input button
#define BUTTON_PIN 2

// Define the pins to be used in tracing mode.
unsigned int ledPins [] = {6, 7, 8, 9, 10, 11, 12, 13};
#define LED_COUNT     (sizeof (ledPins) / sizeof(ledPins[0]))
// Context information for sub-tasks
unsigned long timePrev = 0;


#define BLINK_MODE      0
#define CHASER_MODE     1
boolean mode = BLINK_MODE;    // The current mode of operation. By using an ID to define the
                              // mode, we can have any number of display modes.


// Chaser context.
              // Number of milliseconds between chaser actions.
#define CHASER_DELAY  200
unsigned long chaserNextEventTime;
unsigned long chaserTimerCnt;
int chaserIndex = 0;
boolean chaserIndexGoingUp = true;


// Blink context.
typedef struct {
  unsigned long timerCnt;     // Number of milliseconds that have passed since the last activity.
  unsigned long nextEventTime;  // Number of millisends that must pass until the next activity.

  boolean ledOn;              // tracks whether the LED is currently on or not.
  int ledPin;                 // the Pin to which the LED is connected.
  unsigned long onTime;       // number of milliseconds that this LED should remain on.
  unsigned long offTime;      // number of milliseconds that the LED should remain off.
} BlinkContext;

// Declare 1 blink context for each LED being controlled.
// This is used by the blink subtasks to determine what to do next and when.
BlinkContext blinkCtx[LED_COUNT];


// Button context.
boolean isButtonPressed = false;
int prevButtonState = LOW;
unsigned long buttonNextEventTime = 10;
unsigned long buttonTimerCnt = 0;
unsigned long debounceCnt = 0;


void printBlinkContext(BlinkContext *ctx) {
  Serial.print("Blink Context: Next Evt Time: ");
  Serial.print(ctx->nextEventTime);
  Serial.print(", cur tim: ");
  Serial.print(ctx->timerCnt);
  Serial.print(", led on: " );
  Serial.print(ctx->ledOn);
  Serial.print(", on time: ");
  Serial.print(ctx->onTime);
  Serial.print(", off time: ");
  Serial.print(ctx->offTime);
  Serial.print(", LED Pin: " );
  Serial.println(ctx->ledPin);
}


void randomiseBlink(BlinkContext *ctx) {
  ctx->onTime = 500 + random(1500);         // On time is a random number between 500 and 2000 ms.
  ctx->offTime = 500 + random(1500);        // Off time is a random number between 500 and 2000 ms.
  ctx->nextEventTime = ctx->offTime;       // Specify the next action time.
}


void setup() {
  Serial.begin (9600);
  while (!Serial)
  ;

  // Initialise the LED Pins for output and set them to High (turn the LED's off).
  Serial.print("Initialising LED pin: ");
  for (int i = 0; i < LED_COUNT; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], HIGH);       // Turn the LED off.
    blinkCtx[i].ledOn = false;            // track that the LED is off.
    blinkCtx[i].ledPin = ledPins[i];      // track the pin that this task's LED is attached to.
    blinkCtx[i].timerCnt = 0;             // We are at the beginning of time.

    if (i > 0) {
      Serial.print(", ");
    }
      Serial.print(ledPins[i]);
  }
  Serial.println();

  for (int i = 0; i < LED_COUNT; i++) {
    randomiseBlink(&blinkCtx[i]);
    printBlinkContext(&blinkCtx[i]);
  }
  
  // Set the push button's pin as an input.
  Serial.print("Setting input for push button on pin: ");
  Serial.println(BUTTON_PIN);
  pinMode(BUTTON_PIN, INPUT);
  
  Serial.println("Ready");
  timePrev = millis();              // Initialise the "previous time" value to the current time.
}


void turnAllLed(int state) {
  for (int i = 0; i < LED_COUNT; i++) {
    digitalWrite(ledPins[i], HIGH);       // Turn off the LED.
  }
}


unsigned long blinkLEDTask(BlinkContext *ctx) {
  unsigned long nextEventTime;

  // Are we in blink mode?
  if (mode != BLINK_MODE) {
    // If we are not in the blink mode, then there is nothing to do here.
    return 10000000;        // return a high value to effectively prevent further calls
                            // to this task. Note that the randomise routine will reset
                            // this to a proper value when we switch modes.
  }

  // Check our current state.
  if (ctx->ledOn) {      // Is the LED currently on?
      Serial.print("Turn ON  - "); printBlinkContext(ctx);
      digitalWrite(ctx->ledPin, HIGH);   // Yes, so turn it off
    nextEventTime = ctx->offTime;
  } else {
      Serial.print("Turn OFF - "); printBlinkContext(ctx);
      digitalWrite(ctx->ledPin, LOW);   // Yes, so turn it off
    nextEventTime = ctx->onTime;
  }
  ctx->ledOn = !ctx->ledOn;
  return nextEventTime;
}


unsigned long chaserTask() {

  if (mode != CHASER_MODE) {
    return 10000000;      // Return a high value to effectively prevent further calls to
                          // this task. The mode switch (handleButtonPress) will reset
                          // the next time value to a sensible value.
  }

  if (chaserIndexGoingUp) {
    digitalWrite(ledPins[chaserIndex], HIGH);   // Turn the previous LED OFF.
    chaserIndex += 1;                           // Point to the next higher LED.
    digitalWrite(ledPins[chaserIndex], LOW);    // Turn the LED ON.
                                                // Go up as long as the index is < the
                                                // highest index in the ledPins array.
    chaserIndexGoingUp = (chaserIndex < LED_COUNT - 1);
  } else {
    digitalWrite(ledPins[chaserIndex], HIGH);   // Turn the previous LED OFF.
    chaserIndex -= 1;                           // Point to the next lower LED.
    digitalWrite(ledPins[chaserIndex], LOW);    // Turn the LED ON.
    chaserIndexGoingUp = (chaserIndex <= 0);    // When we reach zero, switch to going up. 
  }
  
  return CHASER_DELAY;
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



unsigned long checkButtonTask() {
  boolean currButtonState = digitalRead(BUTTON_PIN);

  isButtonPressed = false;        // Assume button is not pressed until determined otherwise
  if (prevButtonState == LOW) {
    if (currButtonState == HIGH) { // prev = LOW and curr = LOW
                                  // prev = LOW and curr = HIGH
                                  // state is changing.
      debounceCnt = 0;            // debounce the button.
        Serial.println("Button press detected");
    }
  } else {                        // the previous state of the button was HIGH (pressed)
    if (currButtonState == HIGH) {
      debounceCnt += 1;           // count a debounce.
    } else {                      // prev state = HIGH, curr = LOW - button released.
                                  // if we have passed the debounce threshold, then
                                  // the is button pressed will be set to true.
      isButtonPressed = (debounceCnt > 10);
        Serial.print("Button released, debounceCnt: ");
        Serial.println(debounceCnt);
      debounceCnt = 0;
    }
  }
        // Check the button more frequently if it is pressed.
  prevButtonState = currButtonState;
  return currButtonState == HIGH ? 1 : 10;
}


boolean wasButtonPressedAndReset() {
  boolean result = isButtonPressed;
  isButtonPressed = false;
  return result;
}


void handleButtonPress() {
  switch (mode) {
    case BLINK_MODE:
        Serial.println("Switching to chase mode");
      turnAllLed(HIGH);
      mode = CHASER_MODE;
      chaserIndex = 0;
      chaserTimerCnt = 0;
      chaserNextEventTime = CHASER_DELAY;
      break;
    case CHASER_MODE:
        Serial.println("Switching to blink mode. Contexts:");
      for(int i = 0; i < LED_COUNT; i++) {
        randomiseBlink(&blinkCtx[i]);
        printBlinkContext(&blinkCtx[i]);
        turnAllLed(HIGH);
      }
      mode = BLINK_MODE;
      break;
    default:
      mode = BLINK_MODE;
      break;
  }
}



/**********************
 * Loop
 * 
 */
void loop() {
  unsigned long timeNow = millis();

  if (timeNow != timePrev) {
    unsigned long timeDelta = timeNow - timePrev;
    timePrev = timeNow;

    // Execute the blink sub tasks.
    for (int i = 0; i < LED_COUNT; i++) {
      BlinkContext *ctx = &blinkCtx[i];
      ctx->timerCnt += timeDelta;
      
      if (ctx->timerCnt >= ctx->nextEventTime) {
        ctx->nextEventTime = blinkLEDTask(ctx);
        ctx->timerCnt = 0;
      }
    }
    
    // Execute the chaser sub task.
    chaserTimerCnt += timeDelta;
    if (chaserTimerCnt >= chaserNextEventTime) {
      chaserNextEventTime = chaserTask();
      chaserTimerCnt = 0;
    }

    // Execute the button subtask.
    buttonTimerCnt += timeDelta;
    if (buttonTimerCnt >= buttonNextEventTime) {
      buttonNextEventTime = checkButtonTask();
      buttonTimerCnt = 0;

      if (wasButtonPressedAndReset()) {
        handleButtonPress();
      }
    }
  }
}

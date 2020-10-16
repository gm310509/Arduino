/******************************************************************************
 * Automatic Stair light controller
 * 
 * Detects motion via a PIR (pin 2 digital) and gradually turns on an LED Light strip (pin 3 digital).
 * When motion ceases the LED light strip is gradually turned off.
 * 
 * An LDR (Analog A0) is used to detect ambient light levels. The LED is only turned on if the ambient
 * light level is lower than a specified threshold.
 * 
 * Change history
 * --------------
 * 2020-10 GMc  1.01.01.00
 *   Added print of PIR configuration and Version.
 *
 * 2020-10 GMc  1.01.00.00  
 *   Added Debug constant+debug messages
 *   Expanded the PIR checking to support multiple PIR's
 *   Added version history block
 *
 * 2016-04 GMc  1.00.00.00  
 *   Initial Version
 */

#define VERSION "1.01.01.00"

// Uncomment the following for debug messages.
//#define DEBUG

// Define a constant that determines at what light level should
// should the LED strip be turned on. If the light level is above
// this threshold, the LED strip will not be turned on when motion is triggered.
#ifdef DEBUG
  #define LIGHT_ON_THRESHOLD 700
#else
  #define LIGHT_ON_THRESHOLD 200
#endif


// The pins to which the various peripherals are connected.
// NB: The pirPins array defines the pins to which the PIR signal (or out) pins are connected to.
// IMPORTANT: The contents of this array need to be modified to reflect the number of PIR's that you have.
//            each entry in the pirPins array defines that a PIR is connected to the specified pin.
//            below are three examples that show 1 to 3 PIRs connected to the Arduino.

const int pirPins [] = {2};       // Define a single PIR connected to DIO pin 2
//const int pirPins [] = {2, 4};    // Define dual PIRs. One is connected to DIO pin 2, the other to DIO pin 4.
//const int pirPins [] = {2, 4, 5};   // Define triple PIRs. They are connected to DIO pins 2, 4 and 5.

const int pirCount = sizeof(pirPins) / sizeof(pirPins[0]);    // The number of PIR's defined.
const int ledStripPin = 3;        // Output: PWM signal to the MOSFET.
const int lightSensorPin = A0;    // Input: The reading from the ligh sensor (LDR)


// Cooperative multi-tasking data.
// Cooperative multi-tasking works by activating a task from time to time.
// The period of time between activations depends upon the task.
// This variables is used to track the point in time that we last checked the
// Arduino clock. Our main loop will continually check to see if the Arduino's clock
// has ticked over (> 1ms has passed since we last did anything). If it has, then
// the individual tasks are checked to see if it is time that they should be activated.
//
// You can visually see this working if you activate the Serial monitor in the Arduino IDE.
// What you will see is that the ambient light level reporting will seemingly randomly appear
// in amongst any other debugging messages you might add to the program (e.g. outputting the
// brightness level during a fade).
//
unsigned long timePrev = 0;

// Multi-tasking sub tasks:
// - Read the PIR once every 10 ms.
// - If turning on the LED's, adjust the brightness ever 10 ms
// - It turning off the LED's, adjust the brightness every 30 ms
// - Report the light leveel once every second (this is primarily for testing).
#define PIR_TIME 10               // Check the PIR every 10 ms
unsigned int pirReadTmr = 0;      // A counter to measure how long since we last checked the PIR

#define FADE_ON_TIME 10           // LED's "fade on", the brightness is increased every 10 ms.
#define FADE_OFF_TIME 30          // LED's "fade off", the brightness is decreased every 30 ms.
unsigned int faderTime = FADE_ON_TIME;  // will be set to either FADE_ON_TIME or FADE_OFF_TIME depending upon
                                        // whether we are turning the LED's on or off.
unsigned int faderDelayTmr = 0;   // A counter to measure how long since we last adjusted the LED brightness.

#define LIGHT_CHECK_TIME 1000     // Check the ambient light level once every 1000 ms (once per second)
unsigned int lightCheckTmr = 0;   // A counter to measure how long since we last reported the ambient light level.

/*****************************************************************************
 * Setup:
 * - Initialise the Serial comm's just in case we are debugging/testing.
 * - set up the MOSFET (ledStripPin) and builtin LED PIN to output
 * - Initialise the multi-tasking timer (timePrev),
 */
void setup() {
  
  Serial.begin(115200);
  int cnt = 0;
  // Wait for the the Serial port to initialise (but don't wait forever)
  while (!Serial && cnt < 100) {
      cnt++;
      delay(10);
  }
  // Set the digital outputs (LED_BUILTIN and LED Strip MOSFET)
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(ledStripPin, OUTPUT);

#ifdef DEBUG
  Serial.println(F("Debug mode on"));
#endif
  Serial.print(F("Stairlight - "));
  Serial.println(VERSION);
  Serial.print(pirCount); Serial.print(F(" PIR configured on pin(s): "));
  for (int i = 0; i < pirCount; i++) {
    if (i != 0) {
      Serial.print(F(", "));
    }
    Serial.print(pirPins[i]);
  }
  Serial.println();

  // Initialise the time Keeper
  timePrev = millis();
}

/*****************************************************************************
 * Loop:
 * - Check to see if the time has changed.
 *   - If the time has changed, work out how long has elapsed since the last
 *     time we checked (should always be 1 ms)
 *   - Update the 3 sub-task time counters.
 *   - Check each sub-task time counter to see if it has passed it's respective
 *     threshold.
 *     - If we have passed the threshold, call the appropriate sub-task.
 *     - reset the appropriate time counter to 0.
 */
void loop() {

  // Get the current time in millis.
  // Note that the value is unsigned long. If you do not use unsigned long, then
  // after a few days your multi-tasking might "freak out" (the technical term for
  // not performing as expected).
  unsigned long timeNow = millis();

  // Has the time progressed since last time we checked?
  if (timeNow != timePrev) {
    // Yep, so work out how long has passed (normally 1 ms)
    unsigned int delta = timeNow - timePrev;
    // Capture the current time so we can check if time has moved on from "right now".
    timePrev = timeNow;

    // Update the sub-task time counters.
    pirReadTmr += delta;
    faderDelayTmr += delta;
    lightCheckTmr += delta;

    // Has the pir timer passed it's threshold? If so, check the PIR.
    if (pirReadTmr >= PIR_TIME) {
      processPir();
      pirReadTmr = 0;
    }

    // Has the fader timer passed it's threshold? If so, adjust the brightness of the LED's
    if (faderDelayTmr >= faderTime) {
      processFade();
      faderDelayTmr = 0;
    }

    // Has the ambient light level timer passed it's threshold? If so, report the ambient light level.
    if (lightCheckTmr >= LIGHT_CHECK_TIME) {
      processLightCheck();
      lightCheckTmr = 0;
    }
  }
}

/*******************************************************
 * check PIR Array
 * Check each of the PIRs for motion detection.
 * If any of them report motion detected, return a HIGH value (motion detected).
 * Otherwise if none of them have reported any motion detected (i.e. all of them are LOW), then return a LOW value.
 */
int checkPirArray() {
        // Step through the array of PIRs one by one
  for (int i = 0; i < pirCount; i++) {
    int triggeredInd = digitalRead(pirPins[i]);   // Read the PIR
    if (triggeredInd) {                           // Is it triggered (i.e. triggeredInd is HIGH)?
#ifdef DEBUG
      Serial.print("pir "); Serial.print(i); Serial.print(F(" (DIO pin ")); Serial.print(pirPins[i]); Serial.println(F(") triggered."));
#endif
      return HIGH;                                // Yes, it is triggered, we only need one, so return immediately with a "motion detected" status.
    }
  }
  return LOW;                                     // None of the PIRs are triggered, so return a no motion detected status.
}

/*****************************************************************************
 * Multi-tasking sub tasks start here
 */
 // Variable to track the last known state of the PIRs - initially this should be OFF.
int pirState = LOW;

// Variables to track what the LED control is doing.
bool fadeDisabled = true;     // Are we fading? true = NO, false = YES
bool fadeUp = true;           // Which direction are we fading? true = ON (i.e. getting brighter), false = OFF (i.e. getting dimmer)
int brightness = 0;           // The current brightness. Initially the LED's are off, therefore the brightness is 0.

/*************************************
 * Process PIR
 * - Called when it is time to check the PIR to see if it has changed state.
 * - If the PIR has changed state, initiate the appropriate action for the LED.
 * 
 * Note. It is possible that the PIR will report that "motion has stopped". This might happen
 * part way through the LED turning off cycle. If this occurs, then the fadeOFf will immediately be
 * stopped and the fadeOn will be initiated.
 * This could happen if someone moves out of sight of the PIR, but returns shortly after the PIR
 * stops reporting motion.
 * The overall result is the that the LED's will smoothly transition from whatever brightness level
 * they happened to be at back to full brightness with no ugly blinking, flashing or sudden jump to
 * an extreme level of brightness.
 * 
 * While the above scenario can be handled without multi-tasking, it is much easier (IMHO) to do with
 * this form of multi-tasking. That is checking the PIR state changes while simultaneously adjusting the
 * LED brightness (and as it happens, simultaneously reporting the ambient brightness once every second).
 */
void processPir() {
  // Read the PIR and see if it's state has changed from last time we checked.
  int pirTriggeredInd = checkPirArray();        // Call the function that checks the PIR array for motion.
  if (pirTriggeredInd != pirState) {            // has the motion status changed since we last checked?
    pirState = pirTriggeredInd;                 // Yes, PIR state has changed, so record this new state for subsequent state change checking.
    if (pirState == HIGH) { // Has the PIR detected motion?
      Serial.println("Motion Detected");        // Yep!
                            // Signal that motion has been detected by turning the BUILTIN_LED on.
      digitalWrite(LED_BUILTIN, HIGH);
                            // Determine if we need to turn on the LED Strip.
                            // we will turn on the LED strip if:
                            // a) the ambient light level is less than the LIGHT_ON_THRESHOLD, or
                            // b) the current state of the LED's is on (brightness > 0)
      if (getLightLevel() < LIGHT_ON_THRESHOLD || brightness > 0) {
        fadeOn();           // initiate the turn LED's on sub-task
      }
    } else {                // Motion no longer detected.
      Serial.println("Motion ended");
                            // Signal that motion is no longer detected by turning the BUILTIN_LED off.
      digitalWrite(LED_BUILTIN, LOW);
      fadeOff();            // initiate the turn LED's off sub-task
    }
  }
}

/*************************************
 * Fade On
 *   Initiates the turning ON of the LED strip.
 */
void fadeOn() {
  fadeDisabled = false;       // Enable the fader sub-task.
  fadeUp = true;              // Set the direction to "turn on LED's" (i.e. "get brighter").
  faderTime = FADE_ON_TIME;   // Set the timer threshold that controls how frequently we increase the brightness.
}

/*************************************
 * Fade Off
 *   Initiates the turning OFF of the LED strip.
 */
void fadeOff() {
  fadeDisabled = false;       // Enable the fader sub-task.
  fadeUp = false;             // Set the direction to "turn off LED's" (i.e. "get dimmer").
  faderTime = FADE_OFF_TIME;  // Set the timer threshold that controls how frequently we decrease the brightness.
}

/*************************************
 * Fader sub-task
 *   Adjusts the brightness of the LED strip.
 *   Note that the brightness of the LED's is written to the LED strip PIN using analogWrite.
 *   This means that the Arduino uses PWM to rapidly switch the LED's on and off during the period of time
 *   between calls to this sub-task (either 10 or 30 ms). The proportion of time that the LED's are ON compared
 *   to the time that they are off is directly related to the value written to the port (i.e. the value in brightness).
 *   A value of 0 means the LED's are ON for 0% of the time and OFF 100% of the time (i.e. totally off).
 *   A value of 63 or 64 means roughly 25% of the time ON and 75% of the time off (i.e. one quarter brightness).
 *   A value of 127 means roughly 50% of the time ON and 50% of the time OFF (i.e. half brightness).
 *   A value of 255 means 100% of the time ON and 0% of the time OFF (i.e. full brightness).
 *   
 *   Each time this sub-task is called, it will increase (or decrease) the brightness by 1 if the sub-task is enabled.
 */
void processFade() {
  // First, check to see if the sub-task is active?
  if (fadeDisabled) {
    return;                   // No, just return (i.e. do nothing).
  }

// Debug: Output the current brightness level every 32 steps and when it reaches the maximum.
//  if ((brightness & 0x1F) == 0 || brightness == 255) {
//      Serial.print(" ** brightness = ");
//      Serial.println(brightness);
//  }

  // The sub-task is active (i.e. we are "fading" the LED's)
  if (fadeUp) {               // Are we increasing the brightness?
    if (brightness < 255) {   // Yep, is the current brightness less than the maximum possible value?
      brightness++;           // Yep, increase the brightness and write it to the ledStripPin
      analogWrite(ledStripPin, brightness);
    } else {
      fadeDisabled = true;    // We have reached the maximum brightness (=255) so this task is done. Disable it.
//      Serial.println("Fade on done.");
    }
  } else {                    // We are decreasing the brightness (turning the LED's off)
    if (brightness > 0) {     // Is the current brightness more than the minimum possible value?
      brightness--;           // Yep, decrease the brightness by 1 and write it to the ledStripPin.
      analogWrite (ledStripPin, brightness);
    } else {
      fadeDisabled = true;    // We have reached the minimum brightness (= 0) so this task is done. Disable it.
//      Serial.println("Fade off done.");
    }
  }
}

/*************************************
 * get Light Level
 *  Check and return the ambient light level in a range from 0 (darkest) to 1023 (brightese)
 */
int getLightLevel() {
  // Calculate the light level as the inverse of the value read.
  // The circuit (LDR connected to ground and resistor connected to +5V) means that as it get's darker, the
  // LDR resistance get's higher. Therefore the reading at A0 gets closer to +5V which is read as higher values at A0.
  // Similarly as it get's brighter, the LDR's resistance get's closer to zero, therefore the voltage at A0 gets lower,
  // which is read as lower values at A0.
  // This is, IMHO, counter intuitive, so the easy solution is to subtract the A0 reading from 1023 which means
  //    brighter ambient light (low reading) gives a higher result (e.g. 1023 - 0 = 1023)
  //    darker ambient light (high reading) gives lower result (e.g. 1023 - 1023 = 0)
  int lightLevel = 1023 - analogRead(lightSensorPin);      // Photo resistor with 10K voltage divider
  return lightLevel;  
}

/*************************************
 * process Light check
 *  sub task that periodically reports the current ambient light level.
 */
void processLightCheck() {
  int lightLevel = getLightLevel();
  Serial.print("Light level: ");
  Serial.println(lightLevel);
}

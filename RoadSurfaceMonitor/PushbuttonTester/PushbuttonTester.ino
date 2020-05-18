#define SWITCH_PIN A15

#define DEBOUNCE_TIME 50
#define REPORT_INTERVAL 5000

unsigned long cnt = 0;
int currentState;                     // How we are officially viewing the state of the switch.
int previousReading;                  // What we observed as the condition/setting of the switch the last time we checked.
unsigned long checkChangeStateTime;   // If we register a change in setting of the switch, the time that must pass to register an official change in the state.

unsigned long nextReportTime;

void setup() {
  Serial.begin(38400);
  while (!Serial) {
    delay(1);
  }
  Serial.println("Switch tester program");

  pinMode(SWITCH_PIN, INPUT);
  currentState = digitalRead(SWITCH_PIN);
  previousReading = currentState;

  nextReportTime = millis() + REPORT_INTERVAL;
  Serial.println("Ready");
}

void loop() {
  unsigned long now = millis();
  if (now >= nextReportTime) {
    Serial.print("CNT=");   Serial.println(cnt);
    nextReportTime = now + REPORT_INTERVAL;
  }

  int currentReading = digitalRead(SWITCH_PIN);
  now = millis();
  if (currentReading != previousReading) {        // has the switch reading changed since last time we looked?
    checkChangeStateTime = now + DEBOUNCE_TIME;   // Yes, so set the timer 
    previousReading = currentReading;             // And remember the new reading of the switch.

                                                  // If the reading is different from the current state AND
                                                  // we have made it past the debounce time.
  } else if (currentReading != currentState && now >= checkChangeStateTime) {
    currentState = currentReading;                // Track the change in the *state* of the switch.
    Serial.print("switch: "); Serial.println(currentState); // Report the new state of the switch.
    if (currentState == LOW) {                    // If the switch is pressed
      cnt++;                                      // Increment the counter.
    }
  }
}

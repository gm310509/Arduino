/* LED With push button
 *  
 *  This is a simple program that illustrates pushing a button turns on the LED.
 *  It is an extension from the previous program in that it lets us see the full
 *  extent of the bounce issue due to the removal of the lengthy to execute "print"
 *  statements.
 *  
 *  In this program:
 *  - When the button state changes, a counter is incremented.
 *  - A message is output to the console every 1 second to show the press/release count.
 *  
 *  The period message uses a timing mechanism to avoid with the problems associated
 *  with delays.
 *  Also, as we will see later, any delays introduced by println (which helps solve the
 *  bounce issue) are taken out of the code that handles the switch.
 *  The basic algorithm to periodically print a message is:
 *  - Initialise a variable (mCount) with the program start time (or 0).
 *  - If the desired period of time (1 second) has passed:
 *    - output the desired messages (current counts).
 *    - set the variable (msCount) to the current time in milliseconds + the delay
 *      interval (1000 ms).
 *  
 *  Some important things to note:
 *  - The use of millis and a base time (msCount) to determine when something should
 *    happen. In this case, when a message should be printed.
 */
#define LED 8
#define SWITCH 2

int prevButtonState;

int pressCount = 0;
int releaseCount = 0;
unsigned long msCount = 0;      // Initialising this to zero should force a message
                                // to be displayed on the first call to loop.

void setup() {
  Serial.begin(9600);
  int tOut = 20;    // Wait up to 20 ms for the Serial port to initialise
  while (tOut && !Serial) {
    tOut--;
    delay(1);
  }
  Serial.println("Boot 0 - LED controlled by switch");
  
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

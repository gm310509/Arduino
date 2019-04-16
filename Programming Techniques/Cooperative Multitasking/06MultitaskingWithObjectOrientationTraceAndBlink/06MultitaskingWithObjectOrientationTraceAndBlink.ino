/******************************************************************************
 * Cooperative Multitasking
 *   06 - Multitasking trace and blink with OO
 * 
 * This is the sixth in a series of programs to illustrate the benefits of
 * a simple multitasking mechanism for Arduino.
 * The enhancement on this occassion is to use Object Oriented concepts to make
 * the program simpler.
 * 
 * This program (sketch) shows how to multitask a few different tasks using "cooperative
 * multitasking". The tasks that are being multitasked include:
 * - Monitoring a switch / button press
 * - Executing an LED trace pattern
 * - Randomly blinking multiple LEDs.
 *   The blink task is actually 1 task per LED being blinked. So for 8 LED's, it
 *   is 8 sub tasks.
 * 
 * The big differences between the previous multitasking example and this one include:
 * - less lines of code (about 30 less ~ 9%).
 * - More functionality including:
 *    - named tasks
 *    - disable / enable task support
 *    - bespoke code required to manage timers for the individual types of tasks is
 *      encapsulated into an abstract class extended by the 3 task types.
 * - encapsulation of the context of a task within the specific task's class.
 * - support for a "do not change" the interval between subtask executions return value.
 * - ability to easily add more Task types to support additional animations.
 * - (hopefully) easier to debug and follow what is going on.
 */

// Define the pin for the input button
#define BUTTON_PIN 2

// Define the pins to be used in tracing mode.
unsigned int ledPins [] = {6, 7, 8, 9, 10, 11, 12, 13};
#define LED_COUNT     (sizeof (ledPins) / sizeof(ledPins[0]))

// Context information for sub-tasks
unsigned long timePrev = 0;


// Define constants (as opposed to a boolean) to define which mode we are operating in.
// To add a new MODE, simply extend the list of constants. Make sure that "MODE_CNT" is
// one more than the highest task ID.
#define BLINK_MODE      0
#define CHASER_MODE     1
#define MODE_CNT        2
boolean mode = BLINK_MODE;

/************************************************
 * Class TimedTask.
 * 
 * An abstract (incomplete) class that manages the scheduling of sub tasks.
 * 
 * This class tracks the elapsed amount of time on behalf of it's subclasses.
 * When the alotted time has passed, the "execute" method will be invoked to
 * allow the task to do whatever it needs to do.
 * NB: This TimedTask class would ideally be "factorised" out into a library.
 *     If we did this, then the entire functionality could be accessed by a single
 *     line of code (as opposed to the 70 odd lines here. The single line of code
 *     would be something like: #include <TimedTask.h>.
 */
class TimedTask {
  public:
    // Constructor - capture the time that has to pass until the task needs to be invoked.
    TimedTask(unsigned long nextEventTime) {
      this->nextEventTime = nextEventTime;
    }

    // Set the next event time.
    void setNextEventTime(unsigned long nextEventTime) {
      this->nextEventTime = nextEventTime;
    }

    // Abstract methods which must be implemented (defined) in a subclass.
    virtual unsigned long execute();        // Execute the task.
    virtual String getName();               // Retrieve the name of the task.
    virtual void disableTask();             // Invoked when this task is being disabled.
    virtual void enableTask();              // Invoked when this task is being enabled.

    // Enable this task.
    void enable() {
      enabled = true;
      timeSinceLastEvent = 0;               // Reset the elapsed time counter.
      enableTask();                         // Notify the subclass that the task has been enabled.
    }

    // disable this task.
    void disable() {
      enabled = false;
      disableTask();                        // Notify the subclass that the task has been disabled.
    }

    // Return the enabled/disabled state of the task.
    boolean isEnabled() {
      return enabled;
    }

    // Record the fact that time has passed.
    void recordTime(unsigned long delta) {
      timeSinceLastEvent += delta;          // Record the time and check if this task is due to be
                                            // executed. NB: the task is only executed if it is enabled.
      if (timeSinceLastEvent >= nextEventTime && enabled) {
//        Serial.print ("Executing task: ");
//        Serial.println(getName());
        unsigned long nev = execute();      // Notify the subclass to do it's thing.
        if (nev > 0) {                      // Record the next event time if it is non zero.
          nextEventTime = nev;
        }
        timeSinceLastEvent = 0;             // Reset the time counter.
      }
    }

  private:
    unsigned long nextEventTime;            // Time that must pass before we invoke the subtask.
    unsigned long timeSinceLastEvent = 0;   // The time has passed since the last invocation of the subtask.
    boolean enabled = true;
};

/************************************************
 * Class BlinkTask.
 *    Extends TimedTask.
 * 
 * An implementation (i.e. complete) of a TimedTask that blinks a single LED.
 * 
 * This class toggles the state of the LED when it is invoked.
 */
class BlinkTask : public TimedTask {
  public:
      // Constructor needs to know which pin the LED is connected to.
    BlinkTask(int ledPin)
      : TimedTask(0){
      pinMode(ledPin, OUTPUT);          // Initialise the PIN as output.
      digitalWrite(ledPin, HIGH);       // Turn the LED off.
      this->ledPin = ledPin;            // track the pin.
      setNextEventTime(offTime);        // set the time to the next invocation.
      outputDetails();                  // print the details of this BlinkTask.
      taskName.reserve(15);
      taskName = "blink ";
      taskName += ledPin;
    }

    // Execute the Blink Task
    // This simply toggles the current state of the LED and returns the period
    // of time that must pass before the next invocation.
    unsigned long execute () {
      ledOn = !ledOn;                   // Toggle the LED on/off flag.
//      Serial.print("Turning LED ");
//      Serial.println(ledOn ? "on" : "off");
      digitalWrite(ledPin, ledOn ? LOW : HIGH);   // Set the LED state.
      return ledOn ? onTime : offTime;  // return the next delay time.
    }

    // Retrieve the name of the task as "Blink " + the led's Digital Pin number.
    String getName() {
      return taskName;
    }

    // disable the blink task
    void disableTask() {
      digitalWrite(ledPin, HIGH);       // Turn the LED off.
    }

    // enable the blink task - nothing to do here.
    void enableTask() {
      onTime = randomInterval();      // The time the led will be on (ms)
      offTime = randomInterval();     // The time the led will be off (ms)
      setNextEventTime(offTime);
      digitalWrite(ledPin, HIGH);
      Serial.print("Enabling: ");
      outputDetails();
    }

    // print the details of this blink task.
    void outputDetails() {
      Serial.print("Blink task pin: ");
      Serial.print(ledPin);
      Serial.print(", onTime: ");
      Serial.print(onTime);
      Serial.print(", offTime: ");
      Serial.println(offTime);
    }

  private:
    unsigned long randomInterval() {
      return 500 + random(1500);
    }
    unsigned long onTime = randomInterval();      // The time the led will be on (ms)
    unsigned long offTime = randomInterval();     // The time the led will be off (ms)
    boolean ledOn = false;                        // Initially the LED will be off.
    int ledPin;                                   // The digital I/O pin for the LED.
    String taskName;                              // the name of the task
};


/************************************************
 * Class ChaserTask.
 *    Extends TimedTask.
 * 
 * An implementation (i.e. complete) of a TimedTask that execute a LED chaser.
 * 
 * This causes each LED in a chain to turn on. The task moves the LED up the chain
 * then back down again.
 */
class ChaserTask : public TimedTask {
  public:
    // constructor requires the delay time between updates to the chaser display.
    ChaserTask (unsigned long nextEventTime)
      : TimedTask(nextEventTime) {
        
      }

    // Move the LED up (or down) one step.
    unsigned long execute() {
      if (chaseUp) {              // If going up, turn of the current LED.
        digitalWrite(ledPins[chaserIndex], HIGH);
        chaserIndex += 1;         // point to the next one up the line and turn it on.
        digitalWrite(ledPins[chaserIndex], LOW);
                                  // Go up as long as the index is < the
                                  // Highest index in the LEDpins array.
        chaseUp = chaserIndex < LED_COUNT - 1;
      } else {                    // Going down. turn off the current LED.
        digitalWrite(ledPins[chaserIndex], HIGH);
        chaserIndex -= 1;         // point to the next one down the line and turn it on.
        digitalWrite(ledPins[chaserIndex], LOW);
                                  // Continue going down until we reach
                                  // the beginning of the LEDpins array.
        chaseUp = chaserIndex <= 0;
      }
      return 0;                   // do not change the scheduling interval.
    }

    // Enable the chaser task.
    // Reset to the "beginning" state.
    void enableTask() {
      chaserIndex = 0;            // Resume from the beginning;
      chaseUp = true;             // Go up.
    }

    // disable the chaser task.
    // Turn off the current LED
    void disableTask() {
                                  // Turn the current LED OFF.
      digitalWrite(ledPins[chaserIndex], HIGH);
    }

    // Return the name of the task as "chaser".
    String getName() {
      return "Chaser";
    }

  private:
    boolean chaseUp = true;       // direction of travel
    int chaserIndex = 0;          // currently illuminated LED.
};

/************************************************
 * Class ButtonTask.
 *    Extends TimedTask.
 * 
 * An implementation (i.e. complete) of a TimedTask that detects a button press.
 * 
 * Ancillary methods may be invoked to ascertain if the button has been pressed or not.
 */
class ButtonTask : public TimedTask {
  public:
    // Constructor:
    //   button Pin - the pin the button to be monitored is connected to.
    //   nextEventTime - the time delay between checks to see if the button has been pressed.
    ButtonTask(int buttonPin, unsigned long nextEventTime)
      : TimedTask(nextEventTime) {
        pinMode(buttonPin, INPUT);
        this->buttonPin = buttonPin;

        taskName.reserve(15);
        taskName = "****  button ";
        taskName += buttonPin;
      }

    // Checks to see if the button has been pressed.
    // If it has been pressed, it will debounce the press and
    // set appropriate indicators recording the press when the button is released.
    unsigned long execute() {
                                  // Read the current state of the button.
      int currButtonState = digitalRead(buttonPin);
      if (prevButtonState == LOW) {
        if (currButtonState == HIGH) {    // Button was just pressed.
          debounceCnt = 0;                // Start the debounce count.
          Serial.println("Button pressed");
        }
      } else {                    // Previously the button has tracked as "pressed".
        if (currButtonState == HIGH) {    // Is it still pressed?
          debounceCnt += 1;               // Count the number of intervals that it has remained pressed
        } else {                  // Button has been released.
          Serial.print ("Button released. Debounce Cnt: ");
          Serial.println(debounceCnt);
                                  // Set the button pressed flag to true, if the button has remained pressed
                                  // for the required amount of time.
          buttonPressedInd = (debounceCnt > debounceThreshold);
          debounceCnt = 0;        // reset the debounce count.
        }
      }
      prevButtonState = currButtonState;    // Remember this button state for next time.
      return currButtonState == HIGH ? 1 : 10;  // Check every 1 ms while button is pressed,
                                            // otherwise just check once every 10 ms.
    }

    // Retrieve the task name as "button " + digital pin I/O number.
    String getName() {
      return taskName;
    }

    // Enable the task - nothing special to do.
    void enableTask() {
    }

    // disable the task - nothing special to do.
    void disableTask() {
    }

    // Ancilliary method to query the state of the button.
    boolean isButtonPressed() {
      return buttonPressedInd;
    }

    // Ancilliary method to query the state of the button
    // and reset it's state to false (not pressed)
    boolean isButtonPressedAndReset() {
      boolean result = buttonPressedInd;
      buttonPressedInd = false;
      return result;
    }

    // The number of times (ms) that the button must remain pressed to
    // count as an actual press. Any "presses" less than this duration
    // are ignored as noise.
    const int debounceThreshold = 10;
    
  private:
    int buttonPin;                // The digital I/O pin to which the button is connected.
    int debounceCnt = 0;          // How many contiguous "pressed" readings have we observed?
    int prevButtonState = LOW;    // State of the button - last time we checked.
    boolean buttonPressedInd = false;
    String taskName;              // the name of this task.
};




/*****************************************************
 * The task list.
 * 
 * Establish an array of tasks. All of the tasks go into this array irrespective of their
 * specific type.
 * We seperately track the button task and chaser task, although not strictly necessary, in
 * individual variables (as well as in the array) so we can more cleanly interact with them.
 * 
 * The array is an array of pointers to TimedTasks. This is to allow any class that extends TimedTask
 * to be placed into the array. The draw back is that technically we do not know what the task types are,
 * so we can only directly invoke the methods in TimedTask. Fortunately, through inheritence and abstraction,
 * the TimedTask can invoke the specific methods that are defined as abstract to obtain the specific tasks
 * function (i.e. the TimedTask can activate/call the specific task's individual "execute" method to get it to
 * do whatever it does).
 */
// Define how many tasks there are in total.
#define TASK_COUNT (sizeof taskList / sizeof taskList[0])
TimedTask *taskList[LED_COUNT + 2];
ButtonTask *buttonTask;
ChaserTask *chaserTask;

// Create as many blink tasks as defined in the ledPins array. There will be one
// Blink Task for each defined pin.
// This is called from setup and when the button is pressed (activating Blink Mode)
void resetBlinkers(boolean create) {
  for (int i = 0; i < LED_COUNT; i++) {
    if (create) {
      taskList[i] = new BlinkTask(ledPins[i]);
    } else {
      taskList[i]->enable();              // Enable the blink task and recalculate its operating parameters.
    }
  }
}


// disable all of the blinker tasks and prevent them from executing.
// This is called when the button is pressed (disabling Blink Mode).
void disableBlinkers() {
  for (int i = 0; i < LED_COUNT; i++) {
    taskList[i]->disable();
  }
}


/***********************************************
 * Setup.
 * Initialise the serial monitor
 * Create all of the tasks.
 * Capture the starting time.
 */
void setup() {
  Serial.begin (9600);
  
  int cnt = 0;                    // Initialise the Serial port - but don't wait too long.
  while (!Serial && cnt < 100) {
    cnt++;
    delay(1);
  }
  Serial.println("Initialising");

  resetBlinkers(true);               // Create the blink tasks.
  chaserTask = new ChaserTask(250); // Create the chaser task
  taskList[LED_COUNT] = chaserTask; // add it to the list of all tasks.
  chaserTask->disable();            // initially, disable the chaser.
  
  buttonTask = new ButtonTask(2, 10);   // Create the button task.
  taskList[LED_COUNT + 1] = buttonTask; // add it to the list of all tasks.
  Serial.println("Ready");

  timePrev = millis();              // Initialise the "previous time" value to the current time.
}



/**********************
 * Loop
 * 
 */
void loop() {
  unsigned long timeNow = millis();   // Obtain the "current time".
  if (timeNow != timePrev) {          // Has time moved on?
    unsigned long timeDelta = timeNow - timePrev;
    timePrev = timeNow;               // Work out how much time has passed and capture "now" for next time.
                                      // For each and every task (8 Blink + 1 chaser + 1 button monitor)
                                      // record the fact that time has passed.
                                      // This will invoke the individual tasks' "execute" method if it is due
                                      // to be invoked.
    for (int i = 0; i < TASK_COUNT; i++) {
      taskList[i]->recordTime(timeDelta);
    }
                                      // Check if the button has been pushed.
    if (buttonTask->isButtonPressedAndReset()) {
      mode = (mode + 1) % MODE_CNT;   // Move on to the next mode.
      switch(mode) {                  // Turn on the new mode (and turn off the previous mode).
        case BLINK_MODE:
          Serial.println("Switching to Blink Mode");
          resetBlinkers(false);         // When moving into BLINK_MODE, generate new blinkers (with new random on/off times)
          chaserTask->disable();      // disable the chaser.
          break;
        case CHASER_MODE:
          Serial.println("Switching to Chaser Mode");
          disableBlinkers();          // When moving into CHASER_MODE, disable the blinkers
          chaserTask->enable();       // enable the chaser.
          break;
        default:                      // Unexpected mode??? - should never happen
          Serial.print("Warning: Unexepected mode: ");
          Serial.print(mode);
          Serial.println(" - resetting to blink mode.");
          mode = BLINK_MODE;          // If it does, revert to BLINK mode.
          resetBlinkers(false);
          chaserTask->disable();
          break;
      }
    }
  }
}
 

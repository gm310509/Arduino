
/**
 * StatusLed class.
 * 
 * A class that manages a two leg bi-colour LED.
 * 
 * The use case is on projects that do not have any other form
 * of user interface (and of course those that do) to use a single
 * LED to communicate various status conditions based upon sequences of
 * colour and duration of LED illumination.
 * 
 * An example might be a Red / Green LED might be used as follows:
 * - Solid Green LED - All OK.
 * - 50/50 blinking red - communications error.
 * - 60/10/20/10 Green/Off/Red/Off - Low battery
 * or any other combination (up to MAX_STEPS steps) to indicate
 * any other statuses as required.
 * 
 */
class StatusLed {
  public:
    /**
     * Construct a StatusLed object.
     * 
     * @param a the pin that one leg of the LED is connected to (default pin 7).
     * @param b the pin that the second leg of the LED is connected to (default pin 8).
     */
    StatusLed(const int a = 7, const int b = 8) {
      _a = a;
      _b = b;
      pinMode(_a, OUTPUT);
      pinMode(_b, OUTPUT);
    }

    /**
     * Retrieve the ID of the A pin.
     */
    int getA() {
      return _a;
    }

    /**
     * Retrieve the ID of the B pin.
     */
    int getB() {
      return _b;
    }

    /**
     * Clear any sequence of steps that have previously been established
     * to control the LED.
     * The LED will be turned off.
     * 
     * @return the current number of steps in the sequence (i.e. 0).
     */
    int clearSequence() {
      _numSteps = 0;
      _colourIndex = 0;
      _off();
      _nextStepTime = 0;
      return _numSteps;
    }

    /**
     * Load a sequence of steps that control the LED after clearing any previous sequence.
     * 
     * The number of elements in steps is assumed to match the number of elements in durations
     * as defined by the numSteps parameter.
     * 
     * @param steps (e.g. OFF, A on, B on, Both on) the LED will follow
     * @param durations the durations (in ms) that each step will be active.
     * @param numSteps the number of entries in the "steps" and "durations" arrays.
     * @return the number of steps actually loaded (which may be less than numSteps).
     */
    int loadSequence(int steps[], int durations [], int numSteps) {
      int n = min(numSteps, MAX_STEPS);
      clearSequence();
      for (int i = 0; i < n; i++) {
        _colourSeq[i] = steps[i];
        _colourTime[i] = durations[i];
      }
      _colourIndex = n - 1;     // Load the last step so that when the timerEvent is first called it will wrap to the beginning.
      _numSteps = n;
      return _numSteps;
    }



    /**
     * Add a step to the sequence that has the LED turned off
     * for the specified duration.
     * 
     * @param duration the number of milliseconds that the LED will be turned off.
     * @return the number of steps in the sequence.
     */
    int off(int duration) {
      return _addStep(OFF, duration);
    }

    /**
     * Add a step to turn the LED on as specified by the colour.
     * 
     * @param colour the "colour" of the LED 
     * @param duration the duration for the colour to be active.
     * @return the number of steps in the sequence.
     */
    int on(int colour, int duration) {
      return _addStep(colour, duration);
    }

    /**
     * Return the number of steps in the sequence.
     * @return the number of steps in the sequence.
     */
    int getStepCount() {
      return _numSteps;
    }

    /**
     * Call frequently to ensure that the sequence is executed.
     * This method is used to determine when the next step in the LED sequence
     * is to be activated. The "when" is based upon the duration of the current
     * step and when it was started.
     * 
     * At the end of the sequence, this method will wrap around back to the first
     * step of the sequence.
     * 
     * 
     */
    void timerEvent() {
        // If there are no steps, then just return.
      if (_numSteps <= 0) {
        return;
      }
      
//      if (millis() % 1000 == 0) {
//        Serial.print("millis: "); Serial.print(millis());
//        Serial.print(" next step time: "); Serial.print(_nextStepTime);
//        Serial.print(" Colour Index: "); Serial.println(_colourIndex);
//      }

      // Is it time for the next step to be setup?
      if (millis() >= _nextStepTime) {
          // Yes, so point to the next colour (wrapped at the number of steps).
        _colourIndex = (_colourIndex + 1) % _numSteps;
          // Set the Both timer to now just in case the next colour is BOTH.
        _nextBothTime = millis();
          // Work out the next step time.
        _nextStepTime = millis() + _colourTime[_colourIndex];
          // Finally set the LED's according to the step that we have just established.
        _setNextColour();
      }

      // If the colour sequence is "BOTH", then alternate rapidly between
      // both colours. For example, in the case of a RED / GREEN bicolour LED,
      // This will make the LED take on a sort of yellowish colour.
      if (_colourSeq[_colourIndex] == BOTH) {
        if (millis() > _nextBothTime) {
          _setNextColour();
          _nextBothTime = millis() + 1;
        }
      }
    }

    /** Cause current to flow from pin A to pin B - i.e. illuminate the A side */
    static const int A = 0;
    /** Cause current to flow from pin B to pin A - i.e. illuminate the B side */
    static const int B = 1;
    /** Rapidly alternate the current flow from A to B then B to A - i.e. illuminate both LED's at effectively the same time */
    static const int BOTH = 2;
    /** Disable current flow - i.e. turn the LED off */
    static const int OFF = -1;
    /** 
     *  The maximum number of steps that the LedStatus can execute.
     *  
     *  NB: the "OFF" state counts as a step.
     */
    static const int MAX_STEPS = 10;

  private:
    int _a;                 // Pin that one leg of the BiColour LED is connected to.
    int _b;                 // Pin that the other leg og the LED is connected to.
    int _currActiveLed;     // Only used when alternating between BOTH LED's
    int _colourIndex = 0;   // The current index in the sequence of steps.
    int _numSteps = 0;      // The number of steps in the sequence.
    int _colourSeq[MAX_STEPS];    // The sequence of LED illumination steps.
    int _colourTime[MAX_STEPS];   // The duration of a step. This array aligns with _colourSeq.

    unsigned long _nextStepTime;  // The time (ms) when the next step in the sequence will be activated.
    unsigned long _nextBothTime;  // Only applies if the state is BOTH - determines when the current flow must next be reversed.

    /**
     * Add the colour step to the sequence of steps if there is space.
     */
    int _addStep(int colour, int duration) {
      if (_numSteps >= MAX_STEPS) {
        return -1;
      }
      _colourSeq[_numSteps] = colour;
      _colourTime[_numSteps] = duration;
      return ++_numSteps;
    }

    /**
     * Turn the LED off.
     */
    void _off() {
      digitalWrite(_a, LOW);
      digitalWrite(_b, LOW);    
    }

    /**
     * Turn the A LED On.
     */
    void _aOn() {
      digitalWrite(_a, HIGH);
      digitalWrite(_b, LOW);
    }

    /**
     * Turn the B LED on.
     */
    void _bOn() {
      digitalWrite(_a, LOW);
      digitalWrite(_b, HIGH);    
    }

    /**
     * Determine the LED to show based upon the current step.
     */
    void _setNextColour() {
      switch (_colourSeq[_colourIndex]) {
        case OFF:
          _off();
          break;

        case A:
          _aOn();
          break;
          
        case B:
          _bOn();
          break;
          
        case BOTH:
          if (_currActiveLed == A) {
            _bOn();
            _currActiveLed = B;
          } else {
            _aOn();
            _currActiveLed = A;
          }
      }
    }
};



StatusLed led;

// Blinking green 3/4
int seq1Steps [] = { led.A, led.OFF };
int seq1Times [] = { 750, 250 };

// Long red, short red.
int seq2Steps [] = { led.B, led.OFF, led.B, led.OFF };
int seq2Times [] = { 600, 150, 100, 150 };

// Fast red flash.
int seq3Steps [] = { led.B, led.OFF };
int seq3Times [] = { 100, 100 };

// Long green, short red.
int seq4Steps [] = { led.A, led.OFF, led.B, led.OFF };
int seq4Times [] = { 800, 200, 200, 200 };

// Traffic light
int seq5Steps [] = { led.A, led.BOTH, led.B };
int seq5Times [] = { 2000, 750, 1500 };



void setup() {

  Serial.begin(9600);

  while (!Serial) {
    delay(1);
  }

  led.clearSequence();

  Serial.println();
  Serial.print("LED Pin A: "); Serial.print(led.getA());
  Serial.print(" Pin B: "); Serial.println(led.getB());
  Serial.print("A = "); Serial.println(led.A);
  Serial.print("B = "); Serial.println(led.B);
  Serial.print("Both = "); Serial.println(led.BOTH);
  
  Serial.println("Ready");
}


int duration = 1000;

void showDuration(int duration) {
  Serial.print("duration = "); Serial.println(duration);
}

void showCmd(int cmd, int duration) {
  Serial.print("Step: "); Serial.print(led.getStepCount()); Serial.print("  ");
  if (cmd == led.OFF) {
    Serial.print("led.off(");
  } else {
    Serial.print("led.on(");
    switch (cmd) {
      case led.A:
        Serial.print("led.A");
        break;
      case led.B:
        Serial.print("led.B");
        break;
      case led.BOTH:
        Serial.print("led.BOTH");
        break;
      default:
        Serial.print("led.UNEXPECTED:");
        Serial.print(cmd);
        break;
    }
    Serial.print(", ");
  }
  Serial.print(duration);
  Serial.println(");");
}


// Some interesting command sequences:
// zd2b0c2b0  - long red / short off / short red / short off
// zb12 - red / green
// za12 - short red / short green
//
void loop() {
  int chRead;
  
  led.timerEvent();

  if ( (chRead = Serial.read()) != -1) {
    if (chRead > ' ') {
      switch (chRead) {
        case 'A':
        case 'a':
          duration = 100;
          showDuration(duration);
          break;
        case 'B':
        case 'b':
          duration = 250;
          showDuration(duration);
          break;
        case 'C':
        case 'c':
          duration = 500;
          showDuration(duration);
          break;
        case 'D':
        case 'd':
          duration = 1000;
          showDuration(duration);
          break;
          
        case 'Z':
        case 'z':
          led.clearSequence();
          Serial.println("Clear Steps");
          break;

        case '!':       // Shift 1.
          led.loadSequence(seq1Steps, seq1Times, sizeof (seq1Steps) / sizeof(seq1Steps[0]));
          break;
          
        case '@':       // Shift 2.
          led.loadSequence(seq2Steps, seq2Times, sizeof (seq2Steps) / sizeof(seq2Steps[0]));
          break;
 
        case '#':       // Shift 3.
          led.loadSequence(seq3Steps, seq3Times, sizeof (seq3Steps) / sizeof(seq3Steps[0]));
          break;
 
        case '$':       // Shift 4.
          led.loadSequence(seq4Steps, seq4Times, sizeof (seq4Steps) / sizeof(seq4Steps[0]));
          break;
 
        case '%':       // Shift 5.
          led.loadSequence(seq5Steps, seq5Times, sizeof (seq5Steps) / sizeof(seq5Steps[0]));
          break;
          
        case '0':
          led.off(duration);
          showCmd(led.OFF, duration);
          break;
        case '1':
          led.on(led.A, duration);
          showCmd(led.A, duration);
          break;
        case '2':
          led.on(led.B, duration);
          showCmd(led.B, duration);
          break;
        case '3':
          led.on(led.BOTH, duration);
          showCmd(led.BOTH, duration);
          break;
        default:
          Serial.println("Huh?");
          break;
      }
    }
  }
}

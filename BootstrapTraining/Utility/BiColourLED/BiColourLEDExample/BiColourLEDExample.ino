
/**
 * StatusLed class.
 * 
 * A class that manages a two or three leg bi-colour LED.
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
     * @param threePinMode whether the LED being controlled is a three pin (common Cathode) or not (default false = 2 pin mode)
     */
    StatusLed(const int a = 7, const int b = 8, const bool threePinMode = false) {
      _a = a;
      _b = b;
      pinMode(_a, OUTPUT);
      pinMode(_b, OUTPUT);
      _threePinMode = threePinMode;
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
     * Return the number of steps in the sequence.
     * @return the number of steps in the sequence.
     */
    int getStepCount() {
      return _numSteps;
    }

    /**
     * Is the StatusLed operating in 3 pin (common cathode) mode?
     * @return true if in 3 pin mode - false if in 2 pin mode.
     */
    bool isThreePinMode() {
      return _threePinMode;
    }

    /**
     * Is the StatusLed operating in 2 pin mode?
     * @return true if in 2 pin mode - false if in 3 pin mode.
     */
    bool isTwoPinMode() {
      return !_threePinMode;
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
      if (_colourSeq[_colourIndex] == BOTH && ! _threePinMode) {
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
    bool _threePinMode;     // True if the (bi-colour) LED has three pins. False indicates two pins.

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
          if (_threePinMode) {
            digitalWrite(_a, HIGH);
            digitalWrite(_b, HIGH);
          } else if (_currActiveLed == A) {   // Not three pin mode, so alternate.
            _bOn();
            _currActiveLed = B;
          } else {
            _aOn();
            _currActiveLed = A;
          }
      }
    }
};



StatusLed led2;
StatusLed led3 (9, 10, true);

// Blinking green 3/4
int seq1Steps [] = { StatusLed::A, StatusLed::OFF };
int seq1Times [] = { 750, 250 };

// Long red, short red.
int seq2Steps [] = { StatusLed::B, StatusLed::OFF, StatusLed::B, StatusLed::OFF };
int seq2Times [] = { 1000, 150, 100, 150 };

// Fast red flash.
int seq3Steps [] = { StatusLed::B, StatusLed::OFF };
int seq3Times [] = { 100, 100 };

// Long green, short red.
int seq4Steps [] = { StatusLed::A, StatusLed::OFF, StatusLed::B, StatusLed::OFF };
int seq4Times [] = { 800, 200, 200, 200 };

// Traffic light
int seq5Steps [] = { StatusLed::A, StatusLed::BOTH, StatusLed::B };
int seq5Times [] = { 2000, 750, 1500 };

// Blinking red 10/11 and off 1/11 (duty cycle 10:1)
int seq6Steps [] = { StatusLed::B, StatusLed::OFF };
int seq6Times [] = { 1000, 100 };


void showLedConfig(StatusLed *led) {
  Serial.print(F("Pin A: ")); Serial.print(led->getA());
  Serial.print(F(" Pin B: ")); Serial.print(led->getB());
  Serial.print(F(" 3 pin mode: ")); Serial.println(led->isThreePinMode());
  
}

void setup() {

  Serial.begin(9600);

  while (!Serial) {
    delay(1);
  }

  Serial.println();
  Serial.print("LED2: "); showLedConfig(&led2);
  Serial.print("LED3: "); showLedConfig(&led3);
  
  led2.clearSequence();
  led3.clearSequence();

  
  Serial.println("Ready");
}


int duration = 1000;

void showDuration(int duration) {
  Serial.print(F("duration = ")); Serial.println(duration);
}

void showCmd(int cmd, int duration) {
  Serial.print(F("Step: ")); Serial.print(led2.getStepCount()); Serial.print(F("  "));
  if (cmd == StatusLed::OFF) {
    Serial.print(F("led.off("));
  } else {
    Serial.print(F("led.on("));
    switch (cmd) {
      case StatusLed::A:
        Serial.print(F("led.A"));
        break;
      case StatusLed::B:
        Serial.print(F("led.B"));
        break;
      case StatusLed::BOTH:
        Serial.print(F("led.BOTH"));
        break;
      default:
        Serial.print(F("led.UNEXPECTED:"));
        Serial.print(cmd);
        break;
    }
    Serial.print(F(", "));
  }
  Serial.print(duration);
  Serial.println(F(");"));
}


// Some interesting command sequences:
// zd2b0c2b0  - long red / short off / short red / short off
// zb12 - red / green
// za12 - short red / short green
//
void loop() {
  int chRead;
  int x;
  
  led2.timerEvent();
  led3.timerEvent();

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

 
        case 'X':
        case 'x':
          x = analogRead(A0);
          Serial.print(F("A0: ")); Serial.println(x);
          break;

        case 'Z':
        case 'z':
          led2.clearSequence();
          led3.clearSequence();
          Serial.println(F("Clear Steps"));
          break;

        case '!':       // Shift 1.
          led2.loadSequence(seq1Steps, seq1Times, sizeof (seq1Steps) / sizeof(seq1Steps[0]));
          led3.loadSequence(seq1Steps, seq1Times, sizeof (seq1Steps) / sizeof(seq1Steps[0]));
          break;
          
        case '@':       // Shift 2.
          led2.loadSequence(seq2Steps, seq2Times, sizeof (seq2Steps) / sizeof(seq2Steps[0]));
          led3.loadSequence(seq2Steps, seq2Times, sizeof (seq2Steps) / sizeof(seq2Steps[0]));
          break;
 
        case '#':       // Shift 3.
          led2.loadSequence(seq3Steps, seq3Times, sizeof (seq3Steps) / sizeof(seq3Steps[0]));
          led3.loadSequence(seq3Steps, seq3Times, sizeof (seq3Steps) / sizeof(seq3Steps[0]));
          break;
 
        case '$':       // Shift 4.
          led2.loadSequence(seq4Steps, seq4Times, sizeof (seq4Steps) / sizeof(seq4Steps[0]));
          led3.loadSequence(seq4Steps, seq4Times, sizeof (seq4Steps) / sizeof(seq4Steps[0]));
          break;
 
        case '%':       // Shift 5.
          led2.loadSequence(seq5Steps, seq5Times, sizeof (seq5Steps) / sizeof(seq5Steps[0]));
          led3.loadSequence(seq5Steps, seq5Times, sizeof (seq5Steps) / sizeof(seq5Steps[0]));
          break;
          
        case '^':       // Shift 6.
          led2.loadSequence(seq6Steps, seq6Times, sizeof (seq6Steps) / sizeof(seq6Steps[0]));
          led3.loadSequence(seq6Steps, seq6Times, sizeof (seq6Steps) / sizeof(seq6Steps[0]));
          break;

        case '0':
          led2.off(duration);
          led3.off(duration);
          showCmd(StatusLed::OFF, duration);
          break;
        case '1':
          led2.on(StatusLed::A, duration);
          led3.on(StatusLed::A, duration);
          showCmd(StatusLed::A, duration);
          break;
        case '2':
          led2.on(StatusLed::B, duration);
          led3.on(StatusLed::B, duration);
          showCmd(StatusLed::B, duration);
          break;
        case '3':
          led2.on(StatusLed::BOTH, duration);
          led3.on(StatusLed::BOTH, duration);
          showCmd(StatusLed::BOTH, duration);
          break;
        default:
          Serial.print(F("??? -> ")); Serial.print((char) chRead); Serial.print(F(" (")); Serial.print(chRead); Serial.println(F(")"));
          break;
      }
    }
  }
}

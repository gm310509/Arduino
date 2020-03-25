#include "Arduino.h"
#include "StatusLed.h"


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
/**
 * Construct a StatusLed object.
 * 
 * @param a the pin that one leg of the LED is connected to (default pin 7).
 * @param b the pin that the second leg of the LED is connected to (default pin 8).
 * @param threePinMode whether the LED being controlled is a three pin (common Cathode) or not (default false = 2 pin mode)
 */
StatusLed::StatusLed(const int a = 7, const int b = 8, const bool threePinMode = false) {
  _a = a;
  _b = b;
  pinMode(_a, OUTPUT);
  pinMode(_b, OUTPUT);
  _threePinMode = threePinMode;
}

/**
 * Retrieve the ID of the A pin.
 */
int StatusLed::getA() {
  return _a;
}

/**
 * Retrieve the ID of the B pin.
 */
int StatusLed::getB() {
  return _b;
}

/**
 * Return the number of steps in the sequence.
 * @return the number of steps in the sequence.
 */
int StatusLed::getStepCount() {
  return _numSteps;
}

/**
 * Is the StatusLed operating in 3 pin (common cathode) mode?
 * @return true if in 3 pin mode - false if in 2 pin mode.
 */
bool StatusLed::isThreePinMode() {
  return _threePinMode;
}

/**
 * Is the StatusLed operating in 2 pin mode?
 * @return true if in 2 pin mode - false if in 3 pin mode.
 */
bool StatusLed::isTwoPinMode() {
  return !_threePinMode;
}

/**
 * Clear any sequence of steps that have previously been established
 * to control the LED.
 * The LED will be turned off.
 * 
 * @return the current number of steps in the sequence (i.e. 0).
 */
int StatusLed::clearSequence() {
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
int StatusLed::loadSequence(int steps[], int durations [], int numSteps) {
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
int StatusLed::off(int duration) {
  return _addStep(OFF, duration);
}

/**
 * Add a step to turn the LED on as specified by the colour.
 * 
 * @param colour the "colour" of the LED 
 * @param duration the duration for the colour to be active.
 * @return the number of steps in the sequence.
 */
int StatusLed::on(int colour, int duration) {
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
void StatusLed::timerEvent() {
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


/**
 * Add the colour step to the sequence of steps if there is space.
 */
int StatusLed::_addStep(int colour, int duration) {
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
void StatusLed::_off() {
  digitalWrite(_a, LOW);
  digitalWrite(_b, LOW);    
}

/**
 * Turn the A LED On.
 */
void StatusLed::_aOn() {
  digitalWrite(_a, HIGH);
  digitalWrite(_b, LOW);
}

/**
 * Turn the B LED on.
 */
void StatusLed::_bOn() {
  digitalWrite(_a, LOW);
  digitalWrite(_b, HIGH);    
}

/**
 * Determine the LED to show based upon the current step.
 */
void StatusLed::_setNextColour() {
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

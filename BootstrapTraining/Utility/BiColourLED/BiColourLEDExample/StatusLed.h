#ifndef StatusLed_h
#define StatusLed_h

#include <Arduino.h>

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
    StatusLed(const int a = 7, const int b = 8, const bool threePinMode = false);

    /**
     * Retrieve the ID of the A pin.
     */
    int getA();

    /**
     * Retrieve the ID of the B pin.
     */
    int getB();

    /**
     * Return the number of steps in the sequence.
     * @return the number of steps in the sequence.
     */
    int getStepCount();

    /**
     * Is the StatusLed operating in 3 pin (common cathode) mode?
     * @return true if in 3 pin mode - false if in 2 pin mode.
     */
    bool isThreePinMode();

    /**
     * Is the StatusLed operating in 2 pin mode?
     * @return true if in 2 pin mode - false if in 3 pin mode.
     */
    bool isTwoPinMode();

    /**
     * Clear any sequence of steps that have previously been established
     * to control the LED.
     * The LED will be turned off.
     * 
     * @return the current number of steps in the sequence (i.e. 0).
     */
    int clearSequence();

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
    int loadSequence(int steps[], int durations [], int numSteps);

    /**
     * Add a step to the sequence that has the LED turned off
     * for the specified duration.
     * 
     * @param duration the number of milliseconds that the LED will be turned off.
     * @return the number of steps in the sequence.
     */
    int off(int duration);

    /**
     * Add a step to turn the LED on as specified by the colour.
     * 
     * @param colour the "colour" of the LED 
     * @param duration the duration for the colour to be active.
     * @return the number of steps in the sequence.
     */
    int on(int colour, int duration);

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
    void timerEvent();
    
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
    int _addStep(int colour, int duration);

    /**
     * Turn the LED off.
     */
    void _off();

    /**
     * Turn the A LED On.
     */
    void _aOn();

    /**
     * Turn the B LED on.
     */
    void _bOn();

    /**
     * Determine the LED to show based upon the current step.
     */
    void _setNextColour();
};

#endif

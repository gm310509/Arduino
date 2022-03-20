#ifndef CLOCKDISPLAY_LIB
#define CLOCKDISPLAY_LIB

/* Use interrupts - program configuration constant
 *  If defined (the value is unimportant) then the program
 *  will be configured to use interrupts to control the
 *  refresh of the clock display.
 *  
 *  If not defined, the program will be configured to use
 *  polling to refresh the clock display. This will have side
 *  effects when long running processes are active.
 *  
 * Best option: define the USE_INTERRUPTS symbol.
 */
#define USE_INTERRUPTS  1
#define LED_STROBE_INTERVAL 1         /* Interval between clock LED strobe steps = 1 millisecond (or 1000 times per second */

extern volatile int colonDisplayTime;

#ifdef __cplusplus
extern "C" {
#endif

  /**
   * Function to set the time shown on the clock display.
   * 
   * hour is an integer in the range 0 to 23.
   * minute is an integer in the range 0 to 59.
   * 
   * Values supplied outside of these ranges will result in unpredictable behaviour.
   */
    void setTime(int hour, int minute);

  /**
   * Reset the timer handling the blinking of the colon on the display.
   * This should be called every time the seconds value changes.
   * For example when the seconds value of the RTC clock changes from 1 to 2, or 59 to 0 etc, call this function.
   */
  void resetClockColonDisplayTime(int dispTime);

  /**
   * Set up the clock display DIO pins.
   * If we are using Interrupts, this also sets up the
   * Timer Interrupt.
   */
  void initClockDisplay();


#if ! defined(USE_INTERRUPTS)
  /* 
   *  Strobe the Clock LED.
   *  
   *  This function is called continuously to determine when it is time to strobe the clock LED display.
   *  If it is time to strobe the clock display, then the function determines the next time that it must be
   *  strobed and calls the strobe function.
   *  
   *  This routine can have a side effect when long running operations are executed. Long running operations
   *  prevent this routine from being called in a timely fashion. This can result in the clock display flickering
   *  or freezing on a single digit.
   *  
   *  ** If we are using interrupts to manage the clock display, we don't need this function. 
   */
  void strobeClockLed(unsigned long _now);
#endif      // ! USE_INTERRUPTS


#ifdef __cplusplus
} // extern "C"
#endif      // cplusplus

#endif      //CLOCKDISPLAY_LIB

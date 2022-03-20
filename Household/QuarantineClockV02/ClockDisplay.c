#include <Arduino.h>
#include "ClockDisplay.h"


#if ! defined( USE_INTERRUPTS)
unsigned long strobeClockTO = 0;    // Variable used to track the next time we need to strobe the LED display.
                                    // The strobeClockTO is only relevant for the non-Interrupt driven version of the program.
#endif

volatile int colonDisplayTime = 0;  // How long the clock's colon has been on.
                                    // When the second clicks over, this is set to CLOCK_COLON_DISPLAY_TIME
                                    // and the clock display routine counts it down to zero.
                                    // if the value is non zero, the colon is tuned on. When zero
                                    // the colon is turned off.


// Variables for the LED strobe

/* The ledFont array defines the "font" for the LED's.
 * 
 * Each byte corresponds to the "image" of a "character" that is displayed on a 7 segment LED digit. 
 * On the Arduino Mega, the font value is simply output to Port A (digital pins 22-29 inclusive).
 * 
 *                   font
 *             font  value
 *             value  binary
 *  Character  hex    X G F E  D C B A
 *      0       3f    0 0 1 1  1 1 1 1
 *      1       06    0 0 0 0  1 0 1 0
 *      2       5b    0 1 0 1  1 0 1 1
 *      3       4f    0 1 0 0  1 1 1 1
 *      4       66    0 1 1 0  1 0 1 0
 *      5       6d    0 1 1 0  1 1 0 1
 *      6       75    0 1 1 1  0 1 0 1
 *      7       07    0 0 0 0  0 1 1 1
 *      8       7f    0 1 1 1  1 1 1 1
 *      9       67    0 1 1 0  0 1 1 1
 *      -       40    0 1 0 0  0 0 0 0
 *    space     00    0 0 0 0  0 0 0 0
 *    
 * The value 1 turns an LED segment on, a 0 turns it off. The X bit is unused for the font.
 * The segments map to the Arduino mega's digital outputs as follows:
 * 
 *   Clock    Mega 2560  Arduino
 *   Display  Port A     Digital
 *   Segment  bit        I/O Port
 *      A       0           22
 *      B       1           23
 *      C       2           24
 *      D       3           25
 *      E       4           26
 *      F       5           27
 *      G       6           28
 *      DP      n/c         n/c
 *    L1,L2     7           29
 *      
 */
int ledFont[] = {
    0x3f, 0x06, 0x5b, 0x4f,     // 0, 1, 2, 3
    0x66, 0x6d, 0x7d, 0x07,     // 4, 5, 6, 7
    0x7F, 0x67, 0x40, 0x00      // 8, 9, -, space
  };

volatile int clockDisplay[4] = {0, 0, 0, 0};     // The display image to be displayed on each of the clock LEDs.
int currLed = 0;      // The LED currently being shown:
                      //   0 = tens of hours, 1 = hours, 2 = tens of minutes, 3 = minutes.
                      // This is used by the clock display routine as an index into the clockDisplay array.


const int NUM_LEDS = sizeof (clockDisplay) / sizeof (clockDisplay[0]);    // the number of clock display digits as defined by the size of clockDisplay.

const int clockLedPin[] = { 3, 4, 5, 6 };     // The DIO pins to which the common cathode of the 7 segment LED's is attached.
                                              // [0] is the tens of hours.
                                              // [1] is the units of hours.
                                              // [2] is the tens of minutes.
                                              // [3] is the units of minutes.
/* IMPORTANT NOTE: In V2 (as compared to V1), the common cathode of the 7 segment
 * LEDs is NO LONGER connected to this DIO pin. Instead, to support the brightness function, 
 * each LED's cathode is connected to the brightness digiPot.
 * To support the turning on and off of the individual LEDs, a transistor is used.
 * The Transistor is a NPN transistor (I used BC337). The base of the transistor is connected to
 * the pins defined in clockLEDPin.
 * 
 * Because an NPN transistor is used, the logic on the pin is inverted when compared to V1.
 * In V1, the DIO pin was set low to select an individual 7 segment LED.
 * In this version (V2), an individual 7 segment LED is selected when the DIO pin is set to HIGH.
 */


/*
 * Strobe the clock LED.
 * 
 * Strobing involves turning off the current clock digit (as defined by currLed).
 * This is achieved by setting the associated pin (from clockLedPin) to HIGH.
 * 
 * Then apply the next digit's image from the fonts (clockDisplay) and turn on that LED so
 * its image can be displayed.
 * 
 * This function also manages the blinking of the clock panels colon LED.
 */
void _strobeClockLed() {
  digitalWrite(clockLedPin[currLed], LOW);      // Turn the current digit off (see IMPORTANT NOTE above)
  
  currLed = (currLed + 1) % NUM_LEDS;           // identify the next digit (wrap around to 0 when NUM_LEDs is reached)
  int ledImage = clockDisplay[currLed];         // Get the image of the next digit from the fonts array.
  if (colonDisplayTime != 0) {                  // Work out whether the colon should be on or off.
    ledImage |= 0x80;                           // It should be on, so set bit 7 of the display image to 1
    colonDisplayTime--;                         // count this display time.
  }
  PORTA = ledImage;                             // Output the entire image to PORTA (DIO pins 22-29 on the Arduino Mega)
//    Serial.print("Clock LED "); Serial.print(currLed); Serial.print(": Writing: "); Serial.print(clockDisplay[currLed], HEX);
//    Serial.print(", pin: "); Serial.println(clockLedPin[currLed]);
  digitalWrite(clockLedPin[currLed], HIGH);     // Finally turn on the digit that should display the image.
}


// Timer 2 compare match Interrupt Service Routine
// -----------------------------------------------
// This routine controls the strobing of the LED display.
// What does strobing mean? basically we turn just one digit of the LED's on at any one
// time (the other three LED's are off).
// Why would we do this?
// Because the LED digits all share the control signals (wires) that determine which
// LED segment(s) must be turned on to display the information we need.
// We only have 8 control signals, but we have 32 (4 digits x 8 LED's per digit). So, we strobe,
// or turn on just one of the LED's one at a time, and simultaneously output the correct "image"
// data to the 8 LED segment control lines.
// This routine is called very rapidly - see the description in setup () for how rapidly. This
// gives the illusion of a clear steady display.
#if defined(USE_INTERRUPTS)
SIGNAL(TIMER2_COMPA_vect) {
  _strobeClockLed();
}
#endif



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
#if ! defined( USE_INTERRUPTS)
void strobeClockLed(unsigned long _now) {
  if (_now >= strobeClockTO) {                    // Is it time to strobe the clock display?
    strobeClockTO = _now + LED_STROBE_INTERVAL;     // Yep, calculate the next time to do so and
    _strobeClockLed();                              // strobe the display.
  }
}
#endif


/**
 * Function to set the time shown on the clock display.
 * 
 * hour is an integer in the range 0 to 23.
 * minute is an integer in the range 0 to 59.
 * 
 * Values supplied outside of these ranges will result in unpredictable behaviour.
 */
void setTime(int hour, int minute) {
      // Work out what characters (digits) to put into the clock display.
  if (hour < 10) {                      // is the hour a signle digit?
    clockDisplay[0] = ledFont[11];      // Yes, output a blank (as opposed to a leading zero.
  } else {
    clockDisplay[0] = ledFont[hour / 10]; // No, work out what digit image to display for the 10's of the hour.
  }
  clockDisplay[1] = ledFont[hour % 10];   // The units digit for the hours.
  clockDisplay[2] = ledFont[minute / 10]; // The 10's digit for the minutes (this will display a leading zero if needed).
  clockDisplay[3] = ledFont[minute % 10]; // The units digit for the minutes.
}

/**
 * Reset the timer handling the blinking of the colon on the display.
 * This should be called every time the seconds value changes.
 * For example when the seconds value of the RTC clock changes from 1 to 2, or 59 to 0 etc, call this function.
 */
void resetClockColonDisplayTime(int dispTime) {
  colonDisplayTime = dispTime;  // Yep, reset the colon blink time
}



/**
 * Set up the clock display DIO pins.
 * If we are using Interrupts, this also sets up the
 * Timer Interrupt.
 */
void initClockDisplay() {
  DDRA = 0xff;      // Set Port A to be output. Port A is used to output the image to display on one of the clock digits.
  PORTA = 0x00;     // turn the clock LED off.

        // Set the digital I/O pins for the clock display's common cathodes.
  for (int i = 0; i < NUM_LEDS; i++) {
    pinMode(clockLedPin[i], OUTPUT);
    digitalWrite(clockLedPin[i], HIGH);
  }

#if defined (USE_INTERRUPTS)
      // Setup an interrupt Service Routine to manage the display
      // of the digits on the clock (which must be stobed).
      // This is handled by our Interrupt Service Routine (ISR).
      //
      // To begin, disable interrupts temporarily. Why bother with this?
      // If we didn't disable interrupts, the registers that control interrupts will
      // be active. This means that we could risk an interrupt being fired while we are
      // part way through setting up the registers. Should that happen, the results will
      // be unpredictable, random and in the worst case disastrous. Granted this is low
      // probability, but if you are unlucky, you will regret not having done this!
      // So, we turn of interrupts, configure what we need and finally, renable interrupts.
  noInterrupts();
  
      // We will setup up Timer2 (which is also used by the tone function) to trigger an
      // interrupt 1,000 times per second.
      // Out interrupt Service Routine (ISR) will stobe the Clock digits to give us a nice
      // crisp display.
      //
      // Why would we bother with this complexity?
      //
      // The easiest way to explain why is via an experiment:
      // 1) Set the Serial monitor to a low speed (e.g. 1200 baud).
      //      Remember to change this in the Serial.begin as well.
      // 2) use the non-interrupt version of this program.
      //    Comment out the following line near the top of the program
      //      #define USE_INTERRUPTS 1
      // This may be enough, but if not:  
      // 3) Cause a large volume of data to be displayed on the serial monitor (the easiest way
      //    is to use the "status" command or enter an invalid command such as an "X" or to
      //    display the help information).
      // 
      // What you should see is that the clock display will pause from time to time with just one LED lit,
      // or it may or have similar undesirable characteristics.
      //
      // Finally:
      // 4) repeat the test using the "interrupt enabled" version of the program (uncomment the #define
      //    from step 2.
      // 5) Reset the Serial.begin and serial monitor to a more sensible baud rate (e.g. 115200).
      //
      // So, what is happening is that the strobing of the LED's needs to be performed at a consistant rate to maintain
      // the crispness of the display. If the "refresh rate" slows down too much (or stops altogether), then the clock
      /// display may start to flicker or even freeze periodically.
      // This will happen as a result of other activity (e.g. Serial.println, delay and long running code).
      // To avoid this, the interrupt mechanism will reliably call our LED strobing ISR 1,000 times per second
      // no matter what other long running tasks may be running.
      //
      // After all of the above, you might be thinking:
      //  - don't set the Serial monitor to be so slow
      //  - don't output so many messages.
      //    or
      //  - some other work around.
      // Yes, you could do those things. But, messages are useful. And, workarounds can tend to grow out of proportion 
      // in the "difficulty department" as you try to deal with other side affects.
      // Maybe it is just me, but I feel that I can sometimes see the clock flicker slightly even at higher baud rates.
      // Indeed at 115200 baud, when you run the status command or display the help, the display will flicker.
      // Finally, and this is probably the best reason, I'm stuck in quarantine, so I haven't got much else
      // to do! That is also probably why I am using the single line comment for all of this rather than a
      // /* multi-line comment block */     !!!!!! Although that is starting to get tedious, so I might switch
      // to multi-line comment blocks for the rest of this documentation! :-)
      // And if you don't appreciate the quarantine reason, this use case (i.e. ensuring things happen when they are supposed
      // to happen despite other things that are going on) is what interrupts are for.
      //
      // ------------------------
      //
      // Details about the timer registers we will be using can be found in the AVR data sheet.
      // In the case of the Mega (AVR Mega2560 data sheet dated 02/2014):
      // - Chapter 20 describes all aspects of the features of timer 2 (Chapter 18 on Uno)
      // - Chapter 20.10 has a detailed description of the timer 2 registers (chapter 18.11 on Uno)
      // - Chapter 33 has a summary of all registers (chapter 36 on Uno)
      // 
      // Set the TCCR2 registers to 0. This effectively cancels any other configuration
      // that might be left over in the timer 2 registers.
      // TCCR2 is the Timer Counter / Control register for timer2.
      // TCCR2 is in 2 parts (i.e. A and B).
  TCCR2A = 0;       // set timer 2 control registers 0 to turn off
  TCCR2B = 0;       // all functions.
  TCNT2  = 0;       // initialize the timer 2 counter value to 0
  
                // set compare match register for 1khz increments
                    // = (16*10^6) / (1000*64) - 1 (must be <256)
                    // = 16 MHz clock / (1000 hz  * 64x prescaler) - 1
                    // = 16,000,000 / 64,000 - 1
                    // = 250 - 1
                    // = 249
                    // OCR2A is a single byte (and thus must be < 256)
                    // so a combination of "prescaler" and frequency is used
                    // to determine how high to count before generating an interrupt
  OCR2A = 249;      // = Clock speed / (desired frequency * prescaler value).
  
                // Set CS22 bit for 32x prescaler
                    // - Refer to section 17.10 of the datasheet
                    // Basically this divides the 16MHz clock by 32 (= 0.5MHz)
                    // for the purposes of driving Timer2.
                    // (CS22 = 1, CS21 = 0, CS20 = 0)
  TCCR2B |= (1 << CS22);   
                    // The above two settings cause the counter (TCCNT2) to be incremented by
                    // one every time the scaled clock (16MHz / 64x) ticks.
                    // this will occur roughly once every 16,000,000 / 64,000 = 1/250th of a second
                    // or put another way, 250 times per second.
                    // so once we get to 249 (remember, we started counting from 0) 1 second will have
                    // passed.
 
                // Turn on CTC (Clear Timer on Compare match) mode
                    // Basically, when TCNT2 reaches OCR2A (i.e. 249) TCNT2 is reset to zero.
                    // This is not terribly useful for our purposes, but stay tuned for more.
                    // Refer to chapter 20.10.1 for register setting specfics
                    // (WGM2 = 0, WGM1 = 1, WGM0 = 0) and chapter 20.4 for a 
                    // on the various modes of operation.
  TCCR2A |= (1 << WGM21);

                // Enable timer compare (Timer 2 Output Compare Match A) interrupt.
                    // This is "the good bit", it is what we have been working towards.
                    // Basically when the counter (TCNT2) reaches our limit (OCR2A) an interrupt
                    // will be generated.
                    // The interrupt is the Timer 2 Output Compare Match A Interrupt.
                    // This interrupt is interrupt vector 14 on the Arduino Mega AVR2560
                    // (on the UNO, it is vector 8).
                    // In our code, the Interrupt Service Routine (ISR) is nominated by the rather
                    // curious looking function definition as follows:
                    //   SIGNAL(TIMER2_COMPA_vect) {
                    //     // ISR code goes here
                    //   }
  TIMSK2 |= (1 << OCIE2A);
  
                    // Now that everything is setup, reenable the interrupts.
                    // From this point on, the ISR will be called every 1KHz (i.e. 1000 times per second).
                    // Irrespective of anything else that is going on (such as slow Serial.println calls).
  interrupts();
#endif
}

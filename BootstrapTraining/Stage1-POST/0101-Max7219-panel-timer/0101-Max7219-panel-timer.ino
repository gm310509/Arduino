/**************************
 * 0101-Max7219 Panel Timer
 * -------------------------
 * 
 * This program builds on the previous one (0100-Max7219 Panel Tester) by
 * adding a function to output an arbitrary unsigned integer onto the
 * 8 digit LED Panel controlled by a Max7219 LED controller.
 * 
 * 
 * This program displays a timer counting upwards in second increments (approximately).
 * 
 * To enable this program, import the following library into the Arduino IDE:
 * - LED Control, by Eberhard Fahle
 *   http://wayoda.github.io/LedControl/
 * 
 * The outcomes of this program are to learn:
 * - How to connect an SPI/I2C module to Arduino
 * - How to import a library to manage the module
 * - How to use the library and
 * - Test that the above basics are all working before proceeding
 *   to a more interesting example.
 * 
 * Examples of the LED module include:
 * - https://www.jaycar.com.au/8-digit-7-segment-display-module/p/XC3714
 * - https://hobbycomponents.com/displays/597-max7219-8-digit-seven-segment-display-module
 * - http://www.logicware.com.au/max7219-8-digit-red-led-digital-display-module-boa
 * 
 * Arduino connections as follows:
 * - Pin 10   ->   CS (enable/LODA/LOAD)
 * - Pin 11   ->   CLK
 * - Pin 12   ->   DIN / Data
 * 
 */
#include "LedControl.h"

/*
 Now we need a LedControl to work with.

   pin 12 is connected to the DataIn (or DIN)
   pin 11 is connected to the CLK 
   pin 10 is connected to LOAD (or CS)

 We have only a single MAX72XX.
 */
LedControl lc=LedControl(12,11,10,1);

void setup() {
  Serial.begin(9600);
  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  lc.shutdown(0,false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,4);
  /* and clear the display */
  lc.clearDisplay(0);
}

/*************************************************
 * output (int number)
 * 
 * Function to output an integer to the LED panel.
 * The number may be negative.
 * If the number requires more than 8 digits (including
 * the negative sign if required) will simply be truncated
 * at the most significant digits.
 * 
 * For example, 1234567890 will be displayed as 34567890
 * and -1234567890 will be displayed as -4567890.
 * 
 */
void output(int number) {
  int wrk = number;        // Put the number into a work variable
  bool negative = wrk < 0; // Is the number negative?
  int digitLimit = 8;      // Set the limit of digits to display.
  if (negative) {          // Adjust everything if the number is -ve.
    wrk = -wrk;            // Make it positive (so the math works)
    digitLimit = 7;        // Leave space for the - sign.
  }

  lc.clearDisplay(0);      // Clear remnants from last time.

  int digit = 0;           // Point to the digit we will output.
  do {                     // We must execute at least once.
    int val = wrk % 10;    // calculate the display value.
    wrk = wrk / 10;        // remove the least significant (units) digit.
                           // output it.
    lc.setDigit(0, digit, (byte) val, false);
    digit++;               // Point to the next digit.
                           // loop as long as there is more to display
                           // and we didn't overflow.
  } while (wrk > 0 && digit < digitLimit);
  
  if (negative) {          // if negative, output a minus sign.
    lc.setChar(0, digit, '-', false);
  }
}

// Start the counter at a negative value to test negative numbers
int cntr = -16;


/**************************************
 * loop function
 * 
 * output the counter every one second to simulate a timer. 
 * Note that the interval between updates will not be exactly
 * one second (despite the delay being 1000ms)...
 * 
 * Why?
 * (This is a question for you to think about).
 * 
 * What would be a better way to do this?
 * (You have already seen the answer in some of the Boot 0 - Power on Reset modules)
 * 
 */
void loop() {
  output(cntr);           // Output our number
  cntr++;                 // increment it
  delay(1000);            // and wait for 1 second
}

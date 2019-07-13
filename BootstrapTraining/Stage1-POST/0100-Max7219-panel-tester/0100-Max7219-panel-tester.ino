/**************************
 * 0100-Max7219 Panel Tester
 * -------------------------
 * 
 * A simple program to test the connectivity between Arduino
 * and the 8 digit LED Panel controlled by a Max7219 LED controller.
 * 
 * This program simply displays the 16 hexadecimal digits on the right most
 * digit of the display panel.
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
 * Now we need a LedControl to work with.
 * 
 *   pin 12 is connected to the DataIn (or DIN)
 *   pin 11 is connected to the CLK 
 *   pin 10 is connected to LOAD (or CS)
 * 
 * We have only a single MAX72XX display panel.
 */
LedControl lc=LedControl(12,11,10,1);

/*********************
 * setup
 * -----
 * Initialise the LED Panel and the Serial monitor.
 */
void setup() {
  Serial.begin(9600);
#ifdef ARDUINO_AVR_LEONARDO
  int tOut = 200;    // Wait up to 2000 ms (2 seconds) for the Serial port to initialise
  while (tOut && !Serial) {
    tOut--;
    delay(10);
  }
  Serial.println("Leonardo Serial initialisation complete.");
#else
  Serial.println("Not Leonardo");
#endif
/* 
 *   The MAX72XX is in power-saving mode on startup,
 *   we have to do a wakeup call
 */
  lc.shutdown(0,false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,4);
  /* and clear the display */
  lc.clearDisplay(0);
  
  Serial.println("POST - Max7219 panel - 0100 - test pattern");
}

/* The "hexadecimal" counter value that we will display on the LED Panel. */
int cntr = 0;


/*********************
 * loop
 * ----
 * 
 * The main loop. It simply increments the counter once every 1/2 second
 * and outputs the value onto the left most digit of the display.
 * 
 * Once the counter overflows (reaches 16 or 0x0f), it is wrapped back to 0.
 */
void loop() {
  /* 
   *  setDigit(panelID, digit, value (0x00-0x0F), dpInd);
   *    panelID = id of panel when daisy chaining them together.
   *    digit   = which digit to place the value onto.
   *    value   = the value to be displayed.
   *    dbInd   = true to turn on the decimal place indicator.
   */
  lc.setDigit(0, 0, (byte) cntr, cntr % 2 == 0);
  cntr = ++cntr % 16;
  delay(500);
}

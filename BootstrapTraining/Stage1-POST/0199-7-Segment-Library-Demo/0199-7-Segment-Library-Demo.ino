/*******************************
 * 0199-7-Segment Library Demo
 * 
 * Demonstration of some of the capabilities of the 8 digit 7 Segment
 * LED Display panel controlled by a Max7219 LED controller.
 * 
 * Examples of this module include:
 * - https://www.jaycar.com.au/8-digit-7-segment-display-module/p/XC3714
 * - https://hobbycomponents.com/displays/597-max7219-8-digit-seven-segment-display-module
 * - http://www.logicware.com.au/max7219-8-digit-red-led-digital-display-module-boa
 * 
 * Arduino connections as follows:
 * - Pin 10   ->   CS (enable/LODA/LOAD)
 * - Pin 11   ->   CLK
 * - Pin 12   ->   DIN / Data
 * 
 * 
 * This program displays the word "Arduino", then cycles through a four digit display
 * of hexadecimal values.
 * 
 */
#include "LedControl.h"

/*
 Define the LedControl object to work with.

 pin 12 is connected to the DataIn 
 pin 11 is connected to the CLK 
 pin 10 is connected to LOAD 
 We have only a single MAX72XX.
 */
LedControl lc = LedControl(12,11,10,1);

/* Wait a bit between updates of the display (so it is readable) */
unsigned long delaytime=250;

/*
 * setup
 * -----
 * 
 * Initialise the LED Panel.
 */
void setup() {
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


/*
 * writeArduinoOn7Segment
 * ----------------------
 * 
 * This method will display the characters for the
 * word "Arduino" one after the other on digits 7 (left most)
 * through 1 (digit 0 is the rightmost digit). 
 */
void writeArduinoOn7Segment() {
  lc.setChar(0,7,'a',false);  // The letter A
  delay(delaytime);
  lc.setRow(0,6,0x05);        // The letter r
  delay(delaytime);
  lc.setChar(0,5,'d',false);  // 'd'
  delay(delaytime);
  lc.setRow(0,4,0x1c);        // 'u'
  delay(delaytime);
  lc.setRow(0,3,B00010000);   // 'i'
  delay(delaytime);
  lc.setRow(0,2,0x15);        // 'n'
  delay(delaytime);
  lc.setRow(0,1,0x1D);        // 'o'
  delay(delaytime * 10);      // Delay a bit longer to let ppl read the
                              // wonderful message!
  lc.clearDisplay(0);
  delay(delaytime);
} 

/*
 * scrollDigits
 * ------------
 * 
 * This method will scroll all the hexa-decimal
 * numbers and letters on the display.
 *
 */
void scrollDigits() {
  for(int i=0;i<13;i++) {
    lc.setDigit(0, 3, (byte) i,false);
    delay(50);
    lc.setDigit(0, 2, (byte) i+1,false);
    delay(50);
    lc.setDigit(0, 1, (byte) i+2,false);
    delay(50);
    lc.setDigit(0, 0, (byte) i+3,false);
    delay(delaytime);
  }
  lc.clearDisplay(0);
  delay(delaytime);
}

/*
 * loop
 * ----
 * 
 * The main loop simply repeats the writing of "Arduino" and the digits on the display.
 */
void loop() { 
  writeArduinoOn7Segment();
  scrollDigits();
}

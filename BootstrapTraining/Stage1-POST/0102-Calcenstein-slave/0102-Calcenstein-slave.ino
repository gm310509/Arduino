/**************************
 * 0102-Calcenstein slave
 * ----------------------
 * 
 * This program builds on the previous one (0101-Max7219 Panel Timer) by
 * implementing a simply calculator. 
 * It leverages the function to output an arbitrary signed integer onto the
 * 8 digit LED Panel controlled by a Max7219 LED controller.
 * 
 * The program only implements the addition and clear functions. There are plenty of
 * TODO's. Some of which include:
 * - implement the other operators (-, *, /)
 * - implement unary minus (to allow entry of negative numbers)
 * - implement floating point numbers
 * - implement precedence (i.e. 1 + 2 * 3 results in 9 which is incorrect)
 * - implement parenthesis (e.g. (1 + 2) * 3 is equal to 9)
 * - fix 32767 + 1.
 * - fix 32767 + (any number >=) 2
 * - implement operator replacement e.g. 1 + 2 * (oops, I meant +, not *)
 * - implement backspace (to remove a digit from the current input)
 * - implement CE (to clear the current entry)
 * - allow for chained operations e.g. 1 + 2 = + 3 = (should produce 3 then 6)
 * - implement a memory (MS, M+, MR and MC - for set, plus, recall and clear)
 *   - implement 10 memories identified by either a single digit (0-9) or letter (A-J)
 * - implement support for a keypad
 * - and no doubt more.... 
 *
 * To enable this program, import the following library into the Arduino IDE:
 * - LED Control, by Eberhard Fahle
 *   http://wayoda.github.io/LedControl/
 *   
 * To use the program, a terminal emulator that can operate over "com" ports will
 * be helpful. Examples of suitable terminal emultors include:
 * - putty
 * - coolterm
 * - screen (linux, mac)
 * - hyperterminal
 * - and any one of a number of other options.
 * You can also use the Arduino Serial monitor, but it is not as convenient as a terminal emulator.
 * This is because after each character typed, you must click the send key in the Arduino monitor.
 * 
 * The outcomes of this program are to learn:
 * - To understand that a PC can be used to send input to Arduino for it to action.
 * - To understand that the Arduino can be used as a peripheral to a PC.
 *   - To extrapolate that understanding to other connectivity options (e.g. Bluetooth).
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
LedControl lc = LedControl(12,11,10,1);

int wrk = 0;               // Work value used to accumulate digits.
int acc = 0;               // The accumulator is the running "total"
char op = '\0';            // The most recently entered operator.



/*************************************************
 * echo (char ch)
 *
 * output a character to the Serial device.
 * Handles certain special characters as follows:
 *    newline (0x0d) - add a linefeed (0x0a)
 */
void echo(char ch) {
  Serial.print(ch);
  if (ch == 0x0d) {       // If incoming character is a newline
    Serial.print('\n');   // output a linefeed (required for screen)
  }
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


/*************************************************
 * eval ()
 *
 * Function to evaluate any pending operations.
 * This will: 
 * - update the accumulator in accordance with the pending operation
 * - clear the work variable (wrk) so it is ready to receive new data
 * - update the display with the new accumulator value. 
 * - clear the pending operator.
 */
void eval() {
  switch (op) {
    case '+':         // TODO: Add -, * and /
      acc = acc + wrk;
      break;
    default:          // when there is no pending operator (i.e. after = or C)
      acc = wrk;      // set the accumulator to the working value.
      break;
  }
  output(acc);
  wrk = 0;
  op = '\0';
}

/*************************************************
 * process (char ch)
 * 
 * Process a character received from the input.
 * 
 * If the character is a: 
 * - control character (e.g. newline), do nothing.
 * - digit, accumulate it to the work variable.
 * - operator (+ etc), evaluate the current pending operation
 *   and record the operator.
 * - equals (or space), evaluate the current pending operation
 *   and reset for a new calculation.
 *  
 *  - anything else, output a ? character.
 */

void process(char ch) {
  if (ch < ' ') {
    return;
  }

  if (ch >= '0' && ch <= '9') {
    wrk = wrk * 10 + ch - '0';
    output(wrk);
  } else {
    switch (ch) {
      case 'C':         // Clear?
      case 'c':
        wrk = 0;
        acc = 0;
        output(0);
        break;
      case '+':         // TODO: Add -, * and /
        eval();
        op = ch;
        break;
      case '=':
      case ' ':
      // case 0x0d:
        eval();
          Serial.print("\nAnswer is: ");
        Serial.println(acc);
        acc = 0;
        break;
      default:
        echo('?');
        break;
    }
  }
}


/*************************************************
 * setup ()
 * 
 * Initialise the hardware.
 * 
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
   * The MAX72XX is in power-saving mode on startup,
   * we have to do a wakeup call
   */
  lc.shutdown(0,false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,4);
  /* Initialise the display with a zero */
  output(0);
  
  delay (100);
  Serial.println("POST - Calcenstein slave - uses Serial for input");
  Serial.println("If using screen as client hit ^a^k to quit the emulator");
}


/*************************************************
 * eval ()
 * 
 * Read characters and process them one by one.
 * 
 */
void loop() {
  while (Serial.available()) {
    char incoming = Serial.read();
    echo (incoming);
    process (incoming);
  }
}

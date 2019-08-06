/***************************************
 * 0300-LED-Direct-Cntr
 * --------------------
 * 
 * Test program to drive an LED display directly connected to the Arduino's digital I/O's
 * A strobing mechanism is used to drive the display which is assumed to be a 4 digit,
 * 7 segment, common cathode display.
 * 
 * It is designed to work on an Arduino Uno only as the program uses I/O ports. The configuration
 * of I/O ports to pins seems to vary from one Arduino to another.
 * 
 * The display is connected as follows:
 * 
 * Pin 13    PortB.5        Digit 4 (Left most / thousands) cathode
 * Pin 12    PortB.4        Digit 3 (hundreds) cathode
 * Pin 11    PortB.3        Digit 2 (tens) cathode
 * Pin 10    PortB.2        Digit 1 (Right most / Units) cathode.
 * 
 * Pin 8     PortB.0        G segment
 * Pin 7     PortD.7        F segment
 * Pin 6     PortD.6        E segment
 * Pin 5     PortD.5        D segment
 * Pin 4     PortD.4        C segment
 * Pin 3     PortD.3        B segment
 * Pin 2     PortD.2        A segment
 * 
 * Optionally, you can connect this also
 * Pin 9     PortB.1        DP segment
 * However, it has not been tested.
 * 
 * 
 * This program works by setting the data to be displayed in a buffer.
 * A strobeLED routine is called as part of the main look to update the display.
 */

int cntr = 990;

// interval between increments of the counter. 
#define CNTR_INTERVAL 1000

// Timer for a debug message
unsigned long nextMsgTime = 0;
// Try different values here. For example 1000, 10 and 1.
#define MSG_INTERVAL 1000


// When set to 1, there is a tiny amount of flicker (barely noticeble, but visible).
// set it to 100 to see how the strobing works.
// Set it to 10 to see the strobing as a flicker.
// Set it to 0 for a clear display.
#define STROBE_INTERVAL 0

// Variables to track when to increment the counter and strobe the LED.
unsigned long cntrTimer = 0;
unsigned long strobeTimer = 0;


#define FONT_MINUS 10
#define FONT_SPACE 11
// The "font" to be used to display various characters on the LED panel.
byte font[] = {
  0x3F,       /* 0 */
  0x06,       /* 1 */
  0x5B,       /* 2 */
  0x4F,       /* 3 */
  0x66,       /* 4 */
  0x6D,       /* 5 */
  0x7D,       /* 6 */
  0x07,       /* 7 */
  0x7F,       /* 8 */
  0x67,       /* 9 */
  0x40,       /* - */
  0x00        /* Space */
};

// A Buffer to contain the current "image" to be displayed on the LED.
// Each byte is a bit map which defines which segments to light up.
// The least significant bit is the A segment, the most significant bit is the DP.
byte ledDigits[] = { 0, 0, 0, 0 };

// Which digit are we currently outputing to the digital I/O pins?
byte selectedDigit = -1;


/*************************************
 * StrobeLED
 * ---------
 * 
 * Point to the next digit on the LED and output the rendered display from ledDigits.
 * Note, this routine does not convert numeric values for display. It assumes that the ledDigits
 * buffer contains "rendered" data. That is, strobeLED will simply write whatever is in ledDigits
 * directly to the I/O ports.
 * 
 * This routine uses direct Port manipulation for speed.
 * As such, a specific hardware model is assumed (Arduino Uno).
 * 
 * The routine could be sped up if the fontValue and digitSelect were computed outside of this
 * routine. i.e. when the routine is called, set a flag, then in the main loop calculate the next
 * output values if the flag is set and then clear it.
 * 
 * This would make the routine a little more complicated (due to the split logic) but more suitable
 * to be used as an ISR. Currently this routine compiles to about 40 machine instructions.
 */
void strobeLED() {
  selectedDigit = (selectedDigit + 1) % sizeof(ledDigits);

  // Set the bit for the selected digit.
  byte digitSelect = 1 << selectedDigit;
  // Ones complement because we need "low" to light up the selected digit.
  digitSelect = ~digitSelect;
  // Shift left 2 more places as the LED's start connecting at PIN 10 (or Port B.2 on Uno)
  digitSelect <<= 2;

  byte fontValue = ledDigits[selectedDigit];
  byte fontHigh2Bits = fontValue & 0xc0;    // Mask out the 2 high order bits
  fontHigh2Bits >>= 6;        // Shift the 2 high order bits down to position 0 for inclusion in Port B.

  // Work out the low order 6 bits - which are connected to PortD 2 -> 7 on Uno.
  fontValue = fontValue << 2;
  fontValue = fontValue & 0xFC; // Clear the low order 2 bits (0 & 1) so we do not interfere with
                                // the Serial I/O.

  byte portDValue = PIND;
  byte portBValue = PINB;
  portBValue &= 0xc0;           // Clear bits 0-5 in anticipation of the LED digit selection
                                // And the 2 high order bits from the output.
  portBValue |= digitSelect;    // Set the LED digit selection value.
  portBValue |= fontHigh2Bits;  // Set the high order bits of the output.

  portDValue &= 0x03;           // Clear bits 2-7 in Port D in anticipation of writing the low 6 bits of the font.
  portDValue |= fontValue;      // Set the font bits to be displayed.

  PORTB = portBValue;           // Set the digital I/O pins for ports B and D (which is where the LED is connected).
  PORTD = portDValue;
}


/********************************
 * clearDisplay
 * ------------
 * 
 * Write blanks to the display buffer.
 * 
 */
void clearDisplay() {
  for (int i = 0; i < sizeof(ledDigits); i++) {
    ledDigits[i] = font[FONT_SPACE];
  }
}


/**********************************
 * setFont
 * -------
 * 
 * Write the specified rendered character to the output buffer.
 * This method provides full control over which segments of the LED
 * are turned on or off.
 * 
 * The supplied fontValue is written to the appropriate digit in the buffer.
 */
void setFont(int digit, int fontValue) {
  if (digit < 0 || digit > 3) {
      Serial.print("Invalid digit: ");
    Serial.println(digit);
    return;
  }

  ledDigits[digit] = fontValue;
}


/*********************************
 * setChar
 * -------
 * 
 * Given an index, this method will write lookup the appropriate
 * font value from the fonts array and place the rendered value into
 * the buffer at the specified digit.
 */
void setChar(int digit, int fontIndex) {
  if (fontIndex < 0 || fontIndex > sizeof(font)) {
      Serial.print("Invalid font Index: ");
    Serial.println(fontIndex);
    return;
  }
  ledDigits [digit] = font[fontIndex];
}


void output (int number) {
  int wrk = number;
  bool negative = wrk < 0;
  int digitLimit = 8;
  if (negative) {
    wrk = -wrk;
    digitLimit = 7;
  }
  
  clearDisplay();
  int digit = 0;
  do {
    int val = wrk % 10;
    wrk = wrk / 10;
    setChar(digit, (byte) val);
    digit++;
  } while (wrk > 0 && digit < digitLimit);
  
  if (negative) {
    setFont(digit, font[FONT_MINUS]);
  }
}


/****************************
 * setup
 * -----
 * 
 * Initialise the hardware:
 * - Set the digital I/O pins as output and turn them off so nothing is displayed on the LED.
 * - Turn on Timer2 interrupts (CCT) mode at 8 KHz (the ISR updates the LED display)>
 * 
 * Refer to the Arduino Uno's CPU's data sheet for details on the registers (e.g. TCCR2A, TCNT2, OCR2A etc)
 * used in the following code.
 * http://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf
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

  Serial.println("Boot 0300-LED-Direct-Cntr-Strobe");

  for (int i = 0; i < 8; i++) {       // Initialise the 7 segments + 1 Decimal point...
    pinMode(2 + i, OUTPUT);           // ...  for output.
    digitalWrite(2 + i, HIGH);        // turn them off.
  }

  for (int i = 0; i < sizeof(ledDigits); i++) {
    pinMode(10 + i, OUTPUT);
    digitalWrite(10 + i, HIGH);
  }

  clearDisplay();
  
  Serial.println("Ready");
  unsigned long now = millis();
  cntrTimer = now + CNTR_INTERVAL;
  strobeTimer = now + STROBE_INTERVAL;
}


void loop() {
  // put your main code here, to run repeatedly:

  unsigned long now = millis();
  if (now >= cntrTimer) {
    cntr++;
    if (cntr > 9999) {
      cntr = -999;
    }
    cntrTimer += CNTR_INTERVAL;
    output(cntr);
//      Serial.print("Cntr: ");
//    Serial.println(cntr);
  }

  if (now >= strobeTimer) {
    strobeLED();
    strobeTimer += STROBE_INTERVAL;
  }
  
  if (now > nextMsgTime) {
    nextMsgTime = now + MSG_INTERVAL;
    //Serial.println("Hello there.");
    //Serial.println("Hello there, what's up?");

    int sd = selectedDigit;
    int fnt = ledDigits[sd];
    Serial.print("Selected Digit: ");
    Serial.print(sd);
    Serial.print("-");
    Serial.print(fnt,HEX);
    Serial.print(", dynamic SD = ");
    Serial.print(selectedDigit);
    Serial.print("-");
    Serial.println(ledDigits[selectedDigit],HEX);
  }
}

#include <Wire.h>
#include <RTClib.h>

/*
 * Quarantine Clock.
 * ----------------
 * 
 * By gm310509
 *    2020-09-26
 * 
 * A simple program built for hotel quarantine.
 * Form some reason, my hotel did not have a clock. I find it very convenient to have a clock
 * so that I can just glance at the current time without having to pick up and activate a handheld device such as
 * a phone or tablet (and I couldn't be bothered putting on my watch every day, and I had nothing better to do while in
 * quarantine).
 * 
 * So, I made one from some parts that I had with me.
 * 
 * I happened to have a 10 segment bar LED, so I decided to add this to show the number of days remaining in my quarantine.
 * Hence this quarantine clock program was created.
 * 
 * The program needs a lot of I/O ports (I didn't have any multiplexors or shift registers), so it can only run
 * on an Arduino with a high I/O port count - such as the Arduino Mega.
 * All up, it requires 20 I/O pins.
 * Additionally with the current design, it assumes that at least one MCU port (port A in this
 * version - DIO pins 22-29 on the Arduino Mega) is fully accessible via the Arduino's Digital I/O connectors.
 * 
 */
#define VERSION "1.00.00.00"


#define CHECK_TIME_INTERVAL 1000     /* Interval between RTC time checks = 1 second */
#define LED_STROBE_INTERVAL 1         /* Interval between clock LED strobe steps = 1 millisecond (or 1000 times per second */
#define CLOCK_COLON_DISPLAY_TIME  500 /* How long the clock colon is turned on = 500 millisecond (or 1/2 a second) */

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

/*
 * This is the quarantine end date.
 * Modify this to reflect your quarantine end date.
 * Mine was 9th of October 2020.
 */
DateTime quarantineEndDate (2020, 10, 9, 0, 0, 0);      // The date that quarantine will end.


unsigned long checkTimeTO = 0;      // Variable used to track the *next* time we check the RTC.
unsigned long systemUpTime = 0;     // Counts the number of seconds the system has been running.

#if ! defined( USE_INTERRUPTS)
unsigned long strobeClockTO = 0;    // Variable used to track the next time we need to strobe the LED display.
                                    // The strobeClockTO is only relevant for the non-Interrupt driven version of the program.
#endif

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

volatile int colonDisplayTime = 0;              // How long the clock's colon has been on.
                                                // When the second clicks over, this is set to CLOCK_COLON_DISPLAY_TIME
                                                // and the clock display routine counts it down to zero.
                                                // if the value is non zero, the colon is tuned on. When zero
                                                // the colon is turned off.

const int NUM_LEDS = sizeof (clockDisplay) / sizeof (clockDisplay[0]);    // the number of clock display digits as defined by the size of clockDisplay.
const int clockLedPin[NUM_LEDS] = { 3, 4, 5, 6 };     // The DIO pins to which the common cathode of the 7 segment LED's is attached.
                                                      // [0] is the tens of hours.
                                                      // [1] is the units of hours.
                                                      // [2] is the tens of minutes.
                                                      // [3] is the units of minutes.

// Definition of the number of days remaining in quarantine bar graph LED panel.
const int barGraphLed[] = {                     // The digital I/O pins used to control the 10 segment BAR LED.
    34, 35, 36, 37, 38,                         // [0] is the least significant (rightmost in my design) LED used to represent 1 day left.
    39, 40, 41, 42, 43                          // [9] is the most significant (leftmost in my design) LED used to represent 10 days left.
    };
const int BAR_GRAPH_NUM_LEDS = sizeof(barGraphLed) / sizeof (barGraphLed[0]);     // Number of bar LED's as defined by barGraphLed.
                                                // If you wanted to add more LED's (e.g. to show the full 14 days,
                                                // it should be a simple matter of extend barGraphLed with the extra DIO pin numbers.
                                                // NB: I haven't tested that, but the program is intended to adapt to the number
                                                // of elements in barGraphLed to define the number of quarantine day LEDs.


RTC_DS1307 rtc;                                 // The realtime clock interface.


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
  digitalWrite(clockLedPin[currLed], HIGH);     // Turn the current digit off.
  
  currLed = (currLed + 1) % NUM_LEDS;           // identify the next digit (wrap around to 0 when NUM_LEDs is reached)
  int ledImage = clockDisplay[currLed];         // Get the image of the next digit from the fonts array.
  if (colonDisplayTime != 0) {                  // Work out whether the colon should be on or off.
    ledImage |= 0x80;                           // It should be on, so set bit 7 of the display image to 1
    colonDisplayTime--;                         // count this display time.
  }
  PORTA = ledImage;                             // Output the entire image to PORTA (DIO pins 22-29 on the Arduino Mega)
//    Serial.print("Clock LED "); Serial.print(currLed); Serial.print(": Writing: "); Serial.print(clockDisplay[currLed], HEX);
//    Serial.print(", pin: "); Serial.println(clockLedPin[currLed]);
  digitalWrite(clockLedPin[currLed], LOW);      // Finally turn on the digit that should display the image.
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





/*
 * Calculate the number of quarantine days remaining (including today) 
 * return the number of days remaining.
 */
int getQuarantineDaysRemaining(DateTime _now) {
                                                        // create a datetime for midnight on the date specified.
  DateTime _today = DateTime(_now.year(), _now.month(), _now.day(), 0, 0, 0);
  TimeSpan quarantineTimeRemaining = quarantineEndDate - _today;      // Work out the time remaining in days.
  
  int daysRemaining = quarantineTimeRemaining.days();
//  Serial.print(F("QDays remaining: ")); Serial.print(quarantineTimeRemaining.days());
//  Serial.print(F(", daysRemaining = ")); Serial.println(daysRemaining);
  
  if (daysRemaining < 0) {                              // if we have passed the quarantine end date, then
    daysRemaining = 0;                                  // simply return 0 days remaining (as opposed to a negative number).
  }
  return daysRemaining;
}



/*
 * Given a date+time, calculate the number of days remaining in quarantine.
 * Update the bar graph to show that number.
 */
void setDaysRemaining(DateTime _now) {
  int daysRemaining = getQuarantineDaysRemaining(_now);
  for (int i = 0; i < BAR_GRAPH_NUM_LEDS; i++) {
    digitalWrite(barGraphLed[i], i < daysRemaining);   // if i is within the number of remaining days, then turn the bar LED on otherwise turn it off
  }
  Serial.print("Setting days remaining to "); Serial.println(daysRemaining);

//  Serial.print("Quarantine days remaining: "); Serial.println(quarantineTimeRemaining.days() + 1);
//  DateTime rnd = DateTime(2020, 10, 8, 10, 0, 0);
//  TimeSpan rndTs = quarantineEndDate - rnd;
//  Serial.print("Random Date diff: "); Serial.println(rndTs.days());
  
}


/*
 * Output a formatted date/time.
 */
void printDate(DateTime dttm) {
  Serial.print("DateTime: ");
  Serial.print(dttm.year());  Serial.print("/"); Serial.print(dttm.month()); Serial.print("/"); Serial.print(dttm.day());
  Serial.print(" ");
  Serial.print(dttm.hour()); Serial.print(":");Serial.print(dttm.minute()); Serial.print(":");Serial.println(dttm.second());
}


// Variables used to track the previous second or day.
// These are used to detect if the second or day has changed and thus trigger an appropriate action (Defined below).
int prevSecond = 99;
int prevDay = 99;

/*
 * Checks the Real Time Clock (RTC) time and date.
 * If the time/date has changed, then update the relevant parameters including:
 * - the time to display on the clock LED.
 * - the quarantine days remaining
 * - whether the clock ':' should be on or off (to give the once per second blinking effect).
 */
void checkTime(unsigned long _now) {
  if (_now >= checkTimeTO) {              // Is it time to check the RTC?
    checkTimeTO = _now + CHECK_TIME_INTERVAL;   // Yep, work out when to next check it.
    
    DateTime dttm = rtc.now();            // Read the current date and time.
    //printDate(dttm);
    int day = dttm.day();                 // Extract the components of interest.
    int hour = dttm.hour();
    int minute = dttm.minute();
    int second = dttm.second();
                                          // Have we started a new day? If so, work out the remaining quarantine days.
    if (day != prevDay /*&& second >= 1*/) {  // NB: We need to wait to at least one second into the new day for the calculation to be correct.
      setDaysRemaining(dttm);
      prevDay = day;
    }

        // Work out what characters (digits) to put into the clock display.
    if (hour < 10) {                      // is the hour a signle digit?
      clockDisplay[0] = ledFont[11];      // Yes, output a blank (as opposed to a leading zero.
    } else {
      clockDisplay[0] = ledFont[hour / 10]; // No, work out what digit image to display for the 10's of the hour.
    }
    clockDisplay[1] = ledFont[hour % 10];   // The units digit for the hours.
    clockDisplay[2] = ledFont[minute / 10]; // The 10's digit for the minutes (this will display a leading zero if needed).
    clockDisplay[3] = ledFont[minute % 10]; // The units digit for the minutes.

    if (second != prevSecond) {           // Has the second changed?
      colonDisplayTime = CLOCK_COLON_DISPLAY_TIME;  // Yep, reset the colon blink time
      prevSecond = second;
      systemUpTime++;                     // Count 1 second for the system up time.
    }
  }
}


/**************************
 * The following set of functions and variables are used
 * to interact with the clock (e.g. set the quarantine date, interrogate the system status and others).
 * 
 */
/*
 * The list of valid commands.
 */
char *commands [] = {
  "date",       // Command: 0
  "time",       //  1
  "qdate",      //  2
  "status"      //  3
};

/*
 * A description of the commands. This is only used by the showCommands method.
 */
char *commandDesc [] = {
  "Set Date (yyyy-mm-dd)",
  "Set Time (hh:mm:ss)",
  "Set Quarantine End Date (yyyy-mm-dd).",
  "Print system status"
};


/*
 * Output a list of the commands along with there description and some generic information.
 */
void showCommands() {
  Serial.println(F("Available commands:"));
  for (int i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
    Serial.print("  ");
    Serial.print(commands[i]);
    Serial.print(": ");
    Serial.println(commandDesc[i]);
  }
  Serial.println(F("the format of any parameters is shown in parenthesis"));
  Serial.println(F("For example, use the following to set the time to 1:15 PM and 22 seconds:"));
  Serial.println(F("  time 13:15:22"));
  Serial.println(F("In most cases, you can omit the parameter and the corresponding value will be displayed."));
  Serial.println(F("For example, to show the current date, simply enter the date command with no parameter."));
  Serial.println();
  Serial.println(F("  ** NOTE ** there is no validation on the values entered. Make sure you enter a valid date or time."));
  Serial.println();
}


/*
 * Given some text, attempt to extract up to 3 integers from that text.
 * The integers can be delimited by any non numeric character (including '-' and '.'). Thus, negative
 * numbers will not be identified. By definition, fractional numbers will be treated as two seperate numbers
 * as this function only parses integers.
 * 
 * This function is used to extract the digits from three digit structures such as a date or a time.
 * It can also be used for shorter structures (e.g. a single or dual digit structure) if need be.
 * 
 * Return: The actual number of integers found in the text is returned.
 */
int parse3integers(char * pData, int parameterValues[]) {
  int pIndex = 0;     // the index where the integer will be stored.

  while (pIndex < 3 && *pData) {
          // find the start of the next/first number.
    while ((*pData < '0' || *pData > '9') && *pData) {
      pData++;                    // Skip characters until we get a digit or end of text.
    }
    if (! *pData) {               // If we are at the end of the text, just return what we have found so far.
      return pIndex;              // return the actual number of digits found.
    }
       // Great, we have found a digit, so convert it to a number.
    int acc = 0;                  // Initialise an accumulator to accumalate the digit values.
    while (*pData >= '0' && *pData <= '9') {
      acc = acc * 10 + *pData - '0';    // for each digit, multiply the accumulator by 10 and add in the current digit's value.
      pData++;                          // point to the next character.
    }
    //Serial.print("Storing "); Serial.print(acc); Serial.print(" at index "); Serial.println(pIndex);
    parameterValues[pIndex++] = acc;    // Store this integer and loop back for more.
  }
  return pIndex;                  // Finally return the number of digits captured.
}


// A buffer to accumulate characters received from Serial.
char buffer[80];
int bufferPtr = 0;      // A "pointer" into buffer defining where the next character shall be placed.

/*
 * A command has been supplied from Serial.
 * Identify the command entered and if valid, execute it.
 */
void processControlInput() {
  Serial.print("Command: '");           // To begin, simply echo what we have received.
  Serial.print(buffer);
  Serial.println("'");
    // check the command is valid.

  int cmdNo = -1;                       // Next try to identify the command from the list of available commands found in *commands[]
  for(int i = 0; i < sizeof (commands) / sizeof(commands[0]); i++) {
    char *res = strcasestr(&buffer[0], commands[i]);      // Does the buffer contain the command keyword?
//    Serial.print("Trying: "); Serial.print(commands[i]); Serial.print(", "); Serial.println(res == buffer ? "match" : "no match" );
    if (res == buffer) {                // if res is the same address as the buffer's address, then we have a command match.
      cmdNo = i;                        // identify the matched command
      break;                            // and terminate the loop.
    }
  }

  if (cmdNo == -1) {                    // Check that we have a valid command, if not, output the help and return.
    Serial.print(F("Invalid command entered: '")); Serial.print(buffer); Serial.println(F("'"));
    showCommands();
    return;
  }

    // Find where the command's parameter value starts
    // first, find the end of the command.
  char *pData = &buffer[0];             // Skip over the command text.
  while (*pData != ' ' && *pData != '\t' && *pData) {
    pData++;
  }

    // next find the end of the whitespace.
  while (*pData == ' ' || *pData == '\t') {
    pData++;
  }

  //Serial.print("Data Portion of message: "); Serial.println(pData);
  
  // Commands can accept up to 3 numeric values (e.g. year, month and day in a date).
  // So, prepare to receive those three values.
  int parameterValues[3];
  int parameterCount = parse3integers(pData, &parameterValues[0]);

    // Finally, try to execute the command.
  if (cmdNo == 0 || cmdNo == 1) { // date or time command
    if (parameterCount == 3) {    // did we get three numerics as parameters?
      DateTime dttm = rtc.now();  // Read the current Time.
      int year = dttm.year();
      int month = dttm.month();
      int day = dttm.day();
    
      int hour = dttm.hour();
      int minute = dttm.minute();
      int second = dttm.second();
      if (cmdNo == 0) {           // date command - override the current date with the supplied values.
        year = parameterValues[0];
        month = parameterValues[1];
        day = parameterValues[2];
      } else {                    // time command - override the current time with the supplied values.
        hour = parameterValues[0];
        minute = parameterValues[1];
        second = parameterValues[2];
      }
        // This line sets the RTC with an explicit date & time, for example to set
        // January 21, 2014 at 3am you would call:
        //rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
        // Set the date and time in accordance with the supplied values.
      rtc.adjust(DateTime(year, month, day, hour, minute, second));
      Serial.print(F("Setting date/time: "));
      Serial.print(year); Serial.print("/"); Serial.print(month); Serial.print("/"); Serial.print(day);
      Serial.print(" ");
      Serial.print(hour); Serial.print(":"); Serial.print(minute); Serial.print(":"); Serial.println(second);
    } else {                      // We did not receive three integers for the date / time commands.
      if (parameterCount > 0) {   // If we got any parameters, then there is a validation error. Show the help.
        Serial.println(F("** Failed to parse Date or Time"));
        showCommands();
      } else {                    // We got 0 parameters, so this is a request to show the current date/time.
        Serial.print(F("Current date/time: ")); printDate(rtc.now());
      }
    }
  } else if (cmdNo == 2) {        // qdate command
    if (parameterCount == 3) {    // Did we get three parameters? If so, set the new quarantine end date.
      quarantineEndDate = DateTime(parameterValues[0], parameterValues[1], parameterValues[2], 0, 0, 0);
      Serial.println(F("Quarantine end date set to: ")); printDate(quarantineEndDate);
      setDaysRemaining(rtc.now());  // Update the days remaining value and display.
    } else {                      // We did not get three parameters.
      if (parameterCount > 0) {   // Did we get any parameters? If so, there is an invalid input, so display error and help info.
        Serial.println(F("** Failed to parse Date or Time"));
        showCommands();
      } else {                    // We got zero parameters, so this is a request to show the quarantine end date information.
        Serial.print(F("Quarantine end date: "));
        printDate(quarantineEndDate);
        Serial.print(F("Days remaining: "));
        Serial.println(getQuarantineDaysRemaining(rtc.now()));
      }
    }
  } else if (cmdNo == 3) {        // Status command - takes no parameters, so just output the system status.
    DateTime _now = rtc.now();
    Serial.println(F("Current system status"));
    Serial.print(F("  Firmware Version: ")); Serial.println(F(VERSION));
    Serial.print(F("  Current Date/Time: ")); printDate(_now);
    Serial.print(F("  Quarantine end Date: ")); printDate(quarantineEndDate);
    Serial.print(F("  Quarantine days remaining: ")); Serial.println(getQuarantineDaysRemaining(_now));
    Serial.print(F("  System uptime: ")); Serial.print(systemUpTime); Serial.println(F("s"));
#if defined(USE_INTERRUPTS)
    Serial.println(F("  LED Clock refresh: Interrupt driven"));
#else
    Serial.println(F("  LED Clock refresh: Best effort polling"));
#endif
  } else {                        // We should not get here, but just in case, output an error.
      Serial.println(F("Unexpected command entered"));
      showCommands();
  }
}


/*
 * Check to see if there is any input on Serial.
 * 
 * If so, accumulate the input up to a Carriage Return (CR).
 * Once a CR has been received the input is processed.
 */
void checkControllerInput() {
  if (Serial.available() > 0) {
    char ch = Serial.read();
    if (ch == '\n') {       // We have a CR which marks the end of the input.
      buffer[bufferPtr++] = '\0';   // Null terminate the string.
      processControlInput();        // Process the input
      bufferPtr = 0;                // Reset the buffer Pointer for the next input.
    } else if (ch == '\r') {
      // ignore carriage returns.
    } else if (ch == '\b') {          // Just in case we are using a terminal,
      bufferPtr--;                     // process a backspace by removing a character from the input.
      if (bufferPtr < 0) {
        bufferPtr = 0;
      }
    } else {
                                    // Check for buffer overflow (and avoid it).
      if (bufferPtr < sizeof(buffer) - 1) {
        buffer[bufferPtr++] = ch;     // Not a CR and not a LF, so just accumulate the character.
      }
    }
  }
}



/*
 * setup routine.
 * 
 * Initialise everything including:
 *  - the Serial port for status messages and command input
 *  - set the various ports for output.
 *  - display a cutsie "test pattern" on the bar LED.
 *  - setup Timer2 to generate our interrupts (only for the Interrupt driven version of the program).
 *  - initialise the RTC (real time clock)
 *  - output some helpful information.
 */
void setup() {
  Serial.begin(115200);       // Initialise the Serial port.
  //Serial.begin(1200);       // Use this Serial.begin if you want to try the experiment described below
                              // in the interrupts section of setup().
  int to = 1000;
  while (! Serial && to > 0) {  // Wait for the Serial port to be ready, but no longer than 1 second.
    delay(1);
  }

  Serial.println();
  Serial.print(F("Dimming Quarantine Clock - version: "));
  Serial.println(F(VERSION));

  DDRA = 0xff;      // Set Port A to be output. Port A is used to output the image to display on one of the clock digits.
  PORTA = 0x00;     // turn the clock LED off.

        // Set the digital I/O pins for the clock display's common cathodes.
  for (int i = 0; i < NUM_LEDS; i++) {
    pinMode(clockLedPin[i], OUTPUT);
    digitalWrite(clockLedPin[i], HIGH);
  }

       // Set the digital I/O pins for the remaining days bar LED panel.
  for (int i = 0; i < BAR_GRAPH_NUM_LEDS; i++) {
    pinMode (barGraphLed[i], OUTPUT);
    digitalWrite(barGraphLed[i], LOW);
  }
  delay(500);

      // Output a cute "test pattern" on the bar LED.
  for (int i = 0; i < BAR_GRAPH_NUM_LEDS; i++) {
    digitalWrite(barGraphLed[i], HIGH);     // Turn each LED on one by one and pause after each one.
    delay(100);
  }
  delay(500);                               // Leave the LED's on for a short time.
  for (int i = BAR_GRAPH_NUM_LEDS - 1; i >= 0 ; i--) {
    digitalWrite(barGraphLed[i], LOW);      // Turn each LED off one by one and pase after each one.
    delay(50);
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

  Wire.begin();     // Initialise the interface to the RTC.
  rtc.begin();

  if (! rtc.isrunning()) {
    Serial.println(F("**** RTC is not running. Please set the time and date."));
  }
  Serial.println(); Serial.println(F("Use these commands to set the clock"));
  showCommands();
}


/*
 * The main loop.
 * 
 * Give the perdiodic activities a chance to run. These include:
 *  - Checking if there is any input on Serial.
 *  - Check whether the time has ticked over and format the new time for display.
 *  - strobe the clock LED display (only applies for the non-interrupt version of the program).
 */
void loop() {
  // put your main code here, to run repeatedly:
  unsigned long _now = millis();
  checkControllerInput();
  checkTime(_now);

  // If we are using interrupts to manage the clock display, we don't need this.
#if ! defined( USE_INTERRUPTS)
  strobeClockLed(_now);
#endif
}

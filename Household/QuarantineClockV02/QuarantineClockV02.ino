#include <EEPROM.h>
#include <Wire.h>
#include <RTClib.h>
#include "Brightness.h"
#include "ClockDisplay.h"

/*
 * Quarantine Clock.
 * ----------------
 * 
 * By gm310509
 *    2020-09-26
 * 
 * A simple program built for hotel quarantine.
 * 
 * This project is further documented on Instructables at: https://www.instructables.com/Event-Countdown-Clock-Covid-Clock-V20/
 * 
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
 * Version 2.01.00.00
 * ==================
 *  Moved clock display management into a source file of its own (ClockDisplay.c/h).
 *  Reorganised the commands and their descriptions into a single array (i.e. an array of structure elements).
 *  Upgraded the checkSerial to trigger on CR, LF or both CR/LF line terminators.
 *  Added BS support for terminals (e.g. putty) to checkSerial.
 *  
 * Version 2.00.00.00
 * ==================
 *  Added Brightness control for the LED's using a resistor ladder / digital potentiometer.
 *  Store qdate in EEPROM to preserve it across reboots.
 * 
 *
 * Version 1.00.00.00
 * ==================
 *  Initial Version.
 *  
 */
#define VERSION "2.01.00.00"


#define CHECK_TIME_INTERVAL 1000      /* Interval between RTC time checks = 1 second */
#define CHECK_LIGHT_INTERVAL 5000     /* Interval between checks of the ambient light levels */
#define LDR_PIN A0                    /* The pin that the LDR is connected to for measuring ambient light levels */
#define CLOCK_COLON_DISPLAY_TIME  500 /* How long the clock colon is turned on = 500 millisecond (or 1/2 a second) */

#define EN_ECHO 1                      /* Enable echo on Serial monitor */

/*
 * This is the quarantine end date.
 * Modify this to reflect your quarantine end date.
 * Mine was initially the 9th of October 2020.
 * Then another on the 26th of March 2022.
 * 
 * Note that this date will only be used if there isn't a valid date
 * stored in EEPROM. If EEPROM has a date in it, then the date used here
 * will be ingored in favour of using the EEPROM date.
 * The EEPROM date is set when you use the qdate command to set the
 * quarantine date.
 */
//DateTime quarantineEndDate (2020, 10, 9, 0, 0, 0);      // The date that quarantine will end.
DateTime quarantineEndDate (2022, 3, 20, 0, 0, 0);      // The date that quarantine will end.
//DateTime quarantineEndDate (2022, 3, 26, 0, 0, 0);      // The date that quarantine will end.


unsigned long checkLightLevelTO = 0;  // defines when to next check the ambiend light level.
unsigned long checkTimeTO = 0;      // Variable used to track the *next* time we check the RTC.
unsigned long systemUpTime = 0;     // Counts the number of seconds the system has been running.

/* 
 * The pins that are connected to the transistors in the digital potentiometer. 
 * In this version, it must contain exactly four elements (i.e. four transistors.
 * This equates to 5 brightness levels. There are 5 because there can be no resistor
 * enabled, the first resistor enabled and so on up to the fourth resistor (i.e.
 * four resistors + no resistor = 5 levels).
 */
const int varResistorPins[] = {     // These pins control a series of resistors.
  8, 9, 10, 11                      // The resistors are used to control the brightness of
};                                  // the LEDs.
Brightness ledBrightness (LDR_PIN, varResistorPins);


// Definition of the number of days remaining in quarantine bar graph LED panel.
const int barGraphLed[] = {         // The digital I/O pins used to control the 10 segment BAR LED.
    34, 35, 36, 37, 38,             // [0] is the least significant (rightmost in my design) LED used to represent 1 day left.
    39, 40, 41, 42, 43              // [9] is the most significant (leftmost in my design) LED used to represent 10 days left.
    };
const int BAR_GRAPH_NUM_LEDS = sizeof(barGraphLed) / sizeof (barGraphLed[0]);     // Number of bar LED's as defined by barGraphLed.
                                    // If you wanted to add more LED's (e.g. to show the full 14 days),
                                    // it should be a simple matter of extend barGraphLed with the extra DIO pin numbers.
                                    // NB: I haven't tested that, but the program is intended to adapt to the number
                                    // of elements in barGraphLed to define the number of quarantine day LEDs.


RTC_DS1307 rtc;                     // The realtime clock interface.


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

    if (day != prevDay) {                 // Have we started a new day?
      prevDay = day;                      // Yes, so work out the remaining quarantine days.
      setDaysRemaining(dttm);
    }

    setTime(hour, minute);                // Set the time on the clock display.

    if (second != prevSecond) {           // Has the second changed?
      prevSecond = second;                // Yes, so we need to reset the timing of the blinking colon.
      resetClockColonDisplayTime(CLOCK_COLON_DISPLAY_TIME);
      systemUpTime++;                     // Count 1 second for the system up time.
    }
  }
}

/**
 * Periodically check the ambient light and
 * adjust the brightness of the LEDs accordingly.
 */
void checkLightLevels(unsigned long _now) {
  if (_now >= checkLightLevelTO) {              // Is it time to check the RTC?
    checkLightLevelTO = _now + CHECK_LIGHT_INTERVAL;   // Yep, work out when to next check it.
    ledBrightness.checkLightLevels();
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
struct CmdStruct {
  char *command;
  char *description;
} commands [] = {
  { "date",   "Set Date (yyyy-mm-dd)"},                   // Command: 0
  { "time",   "Set Time (hh:mm:ss)"},                     //  1
  { "qdate",  "Set Quarantine End Date (yyyy-mm-dd)."},   //  2
  { "status", "Print system status"},                     //  3
  { "reset",  "Reset the light metrics"},                 //  4
  { "help",   "Display help (i.e. this message)"}         //  5
};
const int NUM_CMDS = sizeof(commands) / sizeof (commands[0]);

/*
 * Output a list of the commands along with there description and some generic information.
 */
void showCommands() {
  Serial.println(F("Available commands:"));
  for (int i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
    Serial.print("  ");
    Serial.print(commands[i].command);
    if (i < NUM_CMDS) {
      Serial.print(": ");
      Serial.println(commands[i].description);
    } else {
      Serial.println();
    }
  }
  Serial.println();
  Serial.println(F("The format of any parameters is shown in parenthesis."));
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
void processCommand() {

  if (! buffer[0]) {                    // Does the buffer contain any input?
    return;                             // No, so just return.
  }
  
  Serial.println();
  Serial.print(F("Command: '"));           // To begin, simply echo what we have received.
  Serial.print(buffer);
  Serial.println(F("'"));
    // check the command is valid.


  int cmdNo = -1;                       // Next try to identify the command from the list of available commands found in *commands[]
  for(int i = 0; i < NUM_CMDS; i++) {
    char *res = strcasestr(&buffer[0], commands[i].command);      // Does the buffer contain the command keyword?
//    Serial.print("Trying: "); Serial.print(commands[i].command); Serial.print(", "); Serial.println(res == buffer ? "match" : "no match" );
    if (res == buffer) {                // if res is the same address as the buffer's address, then we have a command match.
      cmdNo = i;                        // identify the matched command
      break;                            // and terminate the loop.
    }
  }

  if (cmdNo == -1) {                    // Check that we have a valid command, if not, output the help and return.
    Serial.print(F("Invalid command entered: '")); Serial.print(buffer); Serial.println(F("'"));
    Serial.println(F("Enter 'help' for help"));
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
      int year = parameterValues[0];
      int month = parameterValues[1];
      int day = parameterValues[2];
      quarantineEndDate = DateTime(year, month, day, 0, 0, 0);
      Serial.println(F("Quarantine end date set to: ")); printDate(quarantineEndDate);
      setDaysRemaining(rtc.now());  // Update the days remaining value and display.
      // Save the date into EEPROM to preserve it across resets.
      EEPROM.write(0, year - 2000);
      EEPROM.write(1, month);
      EEPROM.write(2, day);
      EEPROM.write(3, (year - 2000 + month + day) & 0xff);
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
#if defined(EN_ECHO)
    Serial.println(F("Serial terminal - echo enabled"));
#else
    Serial.println(F("Serial terminal - echo disabled"));
#endif
    Serial.println(F("Light Levels:"));
    Serial.print(F("  min: ")); Serial.println(ledBrightness.getMinLightLevel());
    Serial.print(F("  max: ")); Serial.println(ledBrightness.getMaxLightLevel());
    Serial.print(F("  now: ")); Serial.println(ledBrightness.getLightLevel());
    Serial.print(F("Dimming level: ")); Serial.println(ledBrightness.getDimmingLevel());
    ledBrightness.printDebugInfo();
  } else if (cmdNo == 4) {        // Reset command.
    ledBrightness.resetLightMetrics();
  } else if (cmdNo == 5) {        // Help command.
    showCommands();
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
#ifdef EN_ECHO
    Serial.print(ch);
#endif
    if (ch == '\n') {       // We have a CR which marks the end of the input.
      buffer[bufferPtr++] = '\0';   // Null terminate the string.
      processCommand();             // Process the input
      bufferPtr = 0;                // Reset the buffer Pointer for the next input.
    } else if (ch == '\r') {
      buffer[bufferPtr++] = '\0';   // Null terminate the string.
      processCommand();             // Process the input
      bufferPtr = 0;                // Reset the buffer Pointer for the next input.
    } else if (ch == '\b') {        // Just in case we are using a terminal,
      bufferPtr--;                  // process a backspace by removing a character from the input.
      if (bufferPtr < 0) {
        bufferPtr = 0;
      }
#ifdef EN_ECHO
      Serial.print(" \b");          // erase the character from the terminal. We already echoed the BS, so replace the character with a space and BS again.
#endif
    } else {
                                    // Check for buffer overflow (and avoid it).
      if (bufferPtr < sizeof(buffer) - 1) {
        buffer[bufferPtr++] = ch;   // Not a CR and not a LF, so just accumulate the character.
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
  Serial.println(F("Initialising, please wait"));

  initClockDisplay();


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

  Wire.begin();     // Initialise the interface to the RTC.
  rtc.begin();

  if (! rtc.isrunning()) {
    Serial.println(F("**** RTC is not running. Please set the time and date."));
  }

  int year = EEPROM.read(0);
  int month = EEPROM.read(1);
  int day = EEPROM.read(2);
  int checksum = EEPROM.read(3);

  Serial.print("y = "); Serial.print(year);
  Serial.print(",m = "); Serial.print(month);
  Serial.print(",d = "); Serial.print(day);
  Serial.print(",c = "); Serial.println(checksum);

  if ((year + month + day) & 0xff != checksum) {
    Serial.print(F("*** qdate not set."));
  } else {
    quarantineEndDate = DateTime(year+2000, month, day, 0, 0, 0);
    Serial.print(F("*** qdate: ")); printDate(quarantineEndDate);
    //setDaysRemaining(rtc.now());  // Update the days remaining value and display.
  }
  
  Serial.println(); Serial.println(F("Use these commands to set the clock"));
  showCommands();

  Serial.println(F("Ready."));
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
  checkLightLevels(_now);

  // If we are using interrupts to manage the clock display, we don't need this.
#if ! defined( USE_INTERRUPTS)
  strobeClockLed(_now);
#endif
}

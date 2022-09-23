/**
 * Reddit monitor (LCD).
 * ---------------
 * 
 * This program displays metrics for a subreddit.
 * 
 * The reddit API uses https (which is too complicated for Arduino
 * to decrypt), so a companion python script is used to obtain the
 * data, perform relevant calculations and pass the data to this
 * program for display.
 * 
 * Multiple display types are supported and more can be added.
 * 
 * Version history
 * ---------------
 * 2022-08-31 V1.2.0.0
 *   Added support for a TFT device.
 *   
 * 2022-08-25 V1.1.0.1
 *   Minor syntax corrections to address compilation warnings.
 *   
 * 2022-08-25 V1.1.0.0
 *   Added support for display of subReddit name in LCD class
 *   
 * 2022-08-24 V1.0.0.0
 *   Initial release.
 */

// Include our display devices
#include "SubredditStatsLCD.h"
#include "SubredditStatsLED.h"
#include "SubredditStatsTFT_8230.h"

/**** 
 *  Define the type of device we wish to use.
 * Uncomment one of these lines or add your own display
 * driver class and create your own instance of the
 * subredditStats variable.
 * Details can be found at:
 * http://gm310509.com/aaa/subredditMonitor/index.html
 */
//SubredditStatsLCD subredditStats = SubredditStatsLCD();         // 7 segment, 8 digit LED display.
//SubredditStatsLED subredditStats = SubredditStatsLED();           // 2 line 16 character LCD display.
SubredditStatsTFT subredditStats = SubredditStatsTFT();         // Simple 240x320 TFT display shield.

// Values used to process input received over the Serial connection.
#define EN_ECHO
#define BUF_SIZE 80
char inBuf[BUF_SIZE];
int buffPtr = 0;

// The version of the Arduino code.
const char * VERSION = "1.02.00.00";

/**
 * We have received some new data from the data source.
 * Process the data and update the display (if needed).
 */
void processCommand(char *cmd, int charCnt) {
  subredditStats.registerInputData(cmd);
  subredditStats.updateDisplay(true);
}

/*
 * Check to see if there is any input on Serial.
 * 
 * If so, accumulate the input up to a Carriage Return (CR)
 * or Line Feed (LF).
 * Once a CR or LF has been received the input is processed.
 */
void checkControllerInput() {
  if (Serial.available() > 0) {
    char ch = Serial.read();
#ifdef EN_ECHO
    Serial.print(ch);
#endif
    if (ch == '\n' || ch == '\r') { // Do we have a line terminator (LF || CR) which marks the end of the input.
      inBuf[buffPtr++] = '\0';        // Null terminate the string.
      processCommand(inBuf, buffPtr - 1);   // Process the input
      buffPtr = 0;                  // Reset the buffer Pointer for the next input.
    } else if (ch == '\b') {        // Just in case we are using a terminal,
      buffPtr--;                    // process a backspace by removing a character from the input.
      if (buffPtr < 0) {
        buffPtr = 0;
      }
#ifdef EN_ECHO
      Serial.print(" \b");          // erase the character from the terminal. We already echoed the BS, so replace the character with a space and BS again.
#endif
    } else {
                                    // Check for buffer overflow (and avoid it).
      if (buffPtr < sizeof(inBuf) - 1) {
        inBuf[buffPtr++] = ch;      // Not a CR and not a LF, so just accumulate the character.
      }
    }
  }
}


/**
 * Standard Arduino setup function.
 * Print some startup messages and invoke the initDisplay method.
 */
void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(1);
  }
  Serial.println("Sub-Reddit stats monitor");
  Serial.println("By: gm310509");
  Serial.println("    18-08-2022");
  Serial.print  ("Version: "); Serial.println(VERSION);

  // Notify the display to initialise itself if it needs to.
  subredditStats.initDisplay();

  /**
   * CRITICAL - do not change the following print statement in any way.
   * 
   * The word "Ready" is used by the python proxy script to determine
   * that the Arduino is online and ready to go.
   */
  Serial.println("Ready");
}

/**
 * Standard Arduino loop function.
 * Not much to do here, check for input and update the display as required.
 */
void loop() {
  checkControllerInput();
  subredditStats.updateDisplay(false);
}

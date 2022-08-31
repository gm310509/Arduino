#include <Arduino.h>
#include "SubredditStatsLED.h"

#include <LedControl.h>


/* Create a new LedControl variable.
 * We use pins 12,11 and 10 on the Arduino for the SPI interface
 * Pin 12 is connected to the DATA IN-pin of the first MAX7221
 * Pin 11 is connected to the CLK-pin of the first MAX7221
 * Pin 10 is connected to the LOAD(/CS)-pin of the first MAX7221
 * There will only be a single MAX7221 attached to the arduino 
 */
LedControl ledPanel = LedControl(12,11,10,1);

/**
 * Output a message at the current cursor position.
 */void SubredditStatsLED::output(const char * msg, int line, int col) {
  output(msg);
}

/**
 * Output a message at the specified position on the screen.
 */
void SubredditStatsLED::output(const char * msg) {
  if (!msg) {
    return;
  }
  int msgLen = strlen(msg);
  if (msgLen == 0) {
    return;
  }

  ledPanel.clearDisplay(0);

  int dispChar = msgLen;
  while (*msg) {
    ledPanel.setChar(0, --dispChar, *msg++, false);
  }
}

/**
 * Functon to show that the system is being
 * initialised. This displays dashes that
 * alternate from one digit to the other.
 */
void SubredditStatsLED::outputNoData() {
  static boolean left = true;
  if (left) {
    output("- - - - ");
  } else {
    output(" - - - -");
  }
  left = !left;
}

/**
 * Initialise the display.
 * For the LCD, output a banner and wait for 1 second
 * to allow people to see it.
 */
void SubredditStatsLED::initDisplay() {
    // turn on the LED panel.
  ledPanel.shutdown(0, false);
  ledPanel.setIntensity(0, 3);
  ledPanel.clearDisplay(0);
  outputNoData();  
}

/**
 * Update the display.
 * This function simply cycles through each of the available
 * values and displays them.
 * It does not attempt to display the sub-name.
 */
void SubredditStatsLED::updateDisplay(boolean newData) {
//  SubredditStats::updateDisplay(newData);
//  Serial.print("update display: LCD: newData = "); Serial.println(newData);
  unsigned long now = millis();
  if (now >= nextDisplayUpdateTime) {
    nextDisplayUpdateTime = now + displayUpdateInterval * (messageIndex == 1 ? 2 : 1);
    if (! hasValidData()) {
      outputNoData();
      return;
    }
//    Serial.print("Displaying: message[");
//      Serial.print(messageIndex);
//      Serial.print("]='");
//      Serial.print(messages[messageIndex]);
//      Serial.println("'");
    output(getValue(messageIndex));
//    messageIndex = (messageIndex + 1) % MAX_MSG_CNT;
    if (++messageIndex == MAX_MSG_CNT - 1) {
      messageIndex = 0;         // Don't attempt to display the subreddit name on the LED display panel.
    }
  }
}

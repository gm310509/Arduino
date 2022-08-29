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

void SubredditStatsLED::output(char * msg, int line, int col) {
  output(msg);
}

void SubredditStatsLED::output(char * msg) {
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


void SubredditStatsLED::outputNoData() {
  static boolean left = true;
  if (left) {
    output("- - - - ");
  } else {
    output(" - - - -");
  }
  left = !left;
}


void SubredditStatsLED::initDisplay() {
    // turn on the LED panel.
  ledPanel.shutdown(0, false);
  ledPanel.setIntensity(0, 3);
  ledPanel.clearDisplay(0);
  outputNoData();  
}


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

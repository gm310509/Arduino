#include <Arduino.h>
#include "SubredditStatsLCD.h"


#include <LiquidCrystal.h>
const int LCD_RS = 2;
const int LCD_RW = 3;
const int LCD_ENABLE = 4;

// 8 bit interface.
//LiquidCrystal lcd = LiquidCrystal(LCD_RS, LCD_RW, LCD_ENABLE, 5, 6, 7, 8,  9, 10, 11, 12);
// 4 bit interface (same wiring, but 5-8 (DB0-DB3) is unused.
LiquidCrystal lcd = LiquidCrystal(LCD_RS, LCD_RW, LCD_ENABLE, 9, 10, 11, 12);
// both of the above seem to work. However, it does need the variant that supplies the read/write signal.

void SubredditStatsLCD::initDisplay() {
  lcd.begin(16, 2);
  lcd.clear();
  lcd.noCursor();
  output("subreddit");
  output("monitor", 0, 1);
  delay(1000);
}

void SubredditStatsLCD::output(char * msg, int line, int col) {
  lcd.setCursor(line, col);
  output(msg);
}

void SubredditStatsLCD::output(char * msg) {
  if (!msg) {
    return;
  }
  int msgLen = strlen(msg);
  if (msgLen == 0) {
    return;
  }
//  if (rightJustify) {
//    Serial.print("Msg len: "); Serial.println(msgLen);
//    msgLen = min(16, msgLen);
//    lcd.setCursor(16 - msgLen, line);
//  }

  lcd.print(msg);
}


void SubredditStatsLCD::updateDisplay(boolean newData) {
//  SubredditStats::updateDisplay(newData);
//  Serial.print("update display: LCD: newData = "); Serial.println(newData);
  unsigned long now = millis();
  if (now >= nextDisplayUpdateTime) {
    nextDisplayUpdateTime = now + displayUpdateInterval;
    if (! hasValidData()) {
      lcd.clear();
      output("Awaiting data");
      return;
    }
        // Skip the subscribers number - which will always be displayed on the first line.
    if (messageIndex == 1) {
      messageIndex++;
    }
//    Serial.print("Displaying: message[");
//      Serial.print(messageIndex);
//      Serial.print("]='");
//      Serial.print(messages[messageIndex]);
//      Serial.println("'");
    lcd.clear();
    if (messageIndex < 6) { 
      output(getLabel(1));
      output(getValue(1));
    } else {
      output(getLabel(8));
      output(getValue(8));
    }
//    lcd.setCursor(0, 1);
    output(getLabel(messageIndex), 0, 1);
    output(getValue(messageIndex));
//    messageIndex = (messageIndex + 1) % MAX_MSG_CNT;
    if (++messageIndex == MAX_MSG_CNT - 1) {
      messageIndex = 0;         // Don't attempt to display the subreddit name on the LED display panel.
    }
  }
}

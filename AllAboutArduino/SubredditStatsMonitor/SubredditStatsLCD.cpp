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

/**
 * Initialise the display.
 * For the LCD, output a banner and wait for 1 second
 * to allow people to see it.
 */
void SubredditStatsLCD::initDisplay() {
  lcd.begin(16, 2);
  lcd.clear();
  lcd.noCursor();
  output("subreddit");
  output("monitor", 0, 1);
  delay(1000);
}

/**
 * Output a message at the specified position on the screen.
 */
void SubredditStatsLCD::output(const char * msg, int line, int col) {
  lcd.setCursor(line, col);
  output(msg);
}

/**
 * Output a message at the current cursor position.
 */
void SubredditStatsLCD::output(const char * msg) {
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

/**
 * Update the display.
 * 
 * This function cycles through a series of displays using
 * both of the lines on the LCD screen.
 * The cycles are:
 * Subscribers + active now
 * Subscribers + new today
 * Subscribers + new last 7 days
 * Subscribers + new last 30 days
 * Subscribers + new last 90 days
 * SubName + next target
 * SubName + estimated date to achieve that target.
 */
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
      output(getLabel(1));    // For the first 6 cycles, display the
      output(getValue(1));    // Current subscribers on line 1.
    } else {
      output(getLabel(8));    // For the last couple of cycles, display
      output(getValue(8));    // The sub-Name on line 1.
    }

    output(getLabel(messageIndex), 0, 1);
    output(getValue(messageIndex));

    if (++messageIndex >= MAX_MSG_CNT - 1) {
      messageIndex = 0;         // Loop back to the zero'th display line.
    }
  }
}

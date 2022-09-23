#include <Arduino.h>
#include "SubredditStatsTFT_8230.h"

#include <SPI.h>
//#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>

// Define the screen instance
// To make this work, the constants must be set in the library's MCUFRIEND_kbv.cpp so that 
// SUPPORT_8230 is defined. All other constants can be disabled.
// If you have a different type of supported display, then you will need to adjust the
// constant definitions accordingly.
//
// For my display (https://www.jaycar.com.au/240x320-lcd-touch-screen-for-arduino/p/XC4630)
// which is based on the 8230 driver chip the following connections are used:
//   Arduino    Shield   Description
//   D2-7       D2-7     Data bus
//   D8-9       D0-1     Data bus
//   A0         RD       Read from LCD
//   A1         WR       Write to LCD
//   A2         RS       Command/Data Select
//   A3         CS       Chip Select
//   A4         RST      Reset
//   SPI pins   SD Card  SD Card interface
//

MCUFRIEND_kbv tft;

// Human readable names for some basic colours.
// NB: Colours are 16 bit R(5) G(6) B(5).
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define LIGHT_GREY 0xC618
#define DARK_GREY 0x2084
#define GREY    0x8410


extern const char * VERSION;     // Link to the version of our program.

void SubredditStatsTFT::outputPoint(int16_t x, int16_t y, boolean newLine = false) {
  Serial.print(F("(")); Serial.print(x); Serial.print(F(", ")); Serial.print(y);
  if (newLine) {
    Serial.println(F(")"));
  } else {
    Serial.print(F(")"));
  }
}

void SubredditStatsTFT::drawBorder(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
  tft.drawLine(x,     y,     x + w, y,     GREY);   // Top Line
  tft.drawLine(x,     y + h, x + w, y + h, GREY);   // Bottom Line
  tft.drawLine(x,     y,     x,     y + h, GREY);   // Left Line
  tft.drawLine(x + w, y,     x + w, y + h, GREY);   // Right Line
}

void SubredditStatsTFT::drawBorder() {
  drawBorder(0, 0, displayWidth - 1, displayHeight - 1);
}

void SubredditStatsTFT::getWidthAndHeight(const char * msg, uint16_t *w, uint16_t *h) {
  int16_t x1, y1;
  tft.getTextBounds(msg, 0, 0, &x1, &y1, w, h);
}

void SubredditStatsTFT::output(const char * msg, int line, int col) {
  tft.setCursor(line, col);
  output(msg);
}

void SubredditStatsTFT::output(const char * msg) {
  if (!msg) {
    return;
  }
  int msgLen = strlen(msg);
  if (msgLen == 0) {
    return;
  }

  tft.print(msg);
}


void SubredditStatsTFT::center(const char * msg) {
  uint16_t w, h;
  getWidthAndHeight(msg, &w, &h);
  if (w > displayWidth) {
    Serial.print(F("**** Warning: following message wider than display"));
    Serial.print(msg);
    Serial.print(F("Display: ")); Serial.print(displayWidth);
    Serial.print(F(", Message: ")); Serial.print(w);
  }
  uint16_t x = (displayWidth - w) / 2;
  uint16_t y = (displayHeight - h) / 2;
//  Serial.print("Message (x, y) = "); outputPoint(x, y, true);
  output(msg, x, y);
}

void SubredditStatsTFT::initDisplay() {
  uint16_t w, h;               // Work variables for width and height values.

  uint16_t ID = tft.readID(); //
  Serial.print(F("TFT ID = 0x"));
  Serial.println(ID, HEX);
  if (ID == 0xD3D3) {
    ID = 0x9481; // write-only shield
  }
  tft.begin(ID);
  tft.setRotation(3);
  displayWidth = tft.width();
  displayHeight = tft.height();
  Serial.print(F("Dimensions (w,h) = ")); outputPoint(displayWidth, displayHeight, true);

  tft.fillScreen(BLACK);
  tft.setCursor(40, 20);
  tft.setTextColor(GREY, BLACK);
  tft.setTextSize(2);
  center("SubReddit Monitor");

  const char *msg = "Waiting for data...";
  getWidthAndHeight(msg, &w, &h);
  tft.setCursor((displayWidth - w) / 2, displayHeight * 5 / 8);
  tft.print(msg);

  tft.setTextSize(1);
  msg = "Version: ";
  getWidthAndHeight(msg, &w, &h);
  tft.setCursor(10, displayHeight - h - 8);
  tft.print(msg); tft.print(VERSION);
  drawBorder();
  Serial.println("Initialised TFT");

  delay(3000);
}


const int TITLE_BASE_X = 10;
const int TITLE_BASE_Y = 10;

void SubredditStatsTFT::updateDisplay(boolean newData) {
  uint16_t w, h;               // Work variables for width and height values.
  uint16_t y = TITLE_BASE_Y;   // Track our cursor position down the screen.
  SubredditStats::updateDisplay(newData);

  if (!newData) {
    return;
  }
  Serial.print("update display: TFT: newData = "); Serial.println(newData);
  tft.fillScreen(BLACK);
  drawBorder();
  
  // Display the banner.
  tft.setTextSize(3);                 // Display the banner
  getWidthAndHeight(getValue(8), &w, &h);
  y = TITLE_BASE_Y;
  tft.setCursor (TITLE_BASE_X, y);
  tft.print(getValue(8));             // Output the name and underline it
  y += h + 2;
  tft.drawLine (TITLE_BASE_X,     y, TITLE_BASE_X + w,     y, GREY);
  y += 2;
  tft.drawLine (TITLE_BASE_X + 3, y, TITLE_BASE_X + w - 3, y, GREY);
  // Display subscribers and active count
  y += 8;
  tft.setTextSize(2);                 // Display the banner
  getWidthAndHeight(getLabel(1), &w, &h);
  tft.setCursor(TITLE_BASE_X, y);
  tft.print(getLabel(1));
  tft.print(getValue(1));
  y += h + 4;
  tft.setCursor(TITLE_BASE_X, y);
  tft.print(getLabel(0));           // Active right now.
  tft.print(getValue(0));

  // Display the new member counts over the past few intervals.
  // And the estimate for the next target.
  for (int i = 2; i < 8; i++) {
    y += h + 4;
    getWidthAndHeight(getLabel(1), &w, &h);
    tft.setCursor(TITLE_BASE_X, y);
    tft.print(getLabel(i));
    tft.print(getValue(i));
  }

}

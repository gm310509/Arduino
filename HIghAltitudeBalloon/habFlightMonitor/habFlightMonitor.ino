/*****************
 * Program to monitor and log a High Altitude Balloon
 * flight.
 *
 */
// HAB stuff
#include "hab.h"


// OLED stuff.
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define VERSION "v1.00.01"
// Baud rate of Serial console.

#define OLED_LINE_SPACING 4 // Additional space between lines when positioning cursor.

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_SSD1306 display(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, & OLED_PORT, OLED_RESET);


/**
 * rightJustify
 * 
 * Right justify a long integer into a buffer that is size fieldSize + 1. 
 * The filler character is used to pad the number to the right.
 * 
 * @params
 *   buf - a buffer that is big enough to hold the converted number *and* a trailing null byte.
 *   fieldSize - the number of characters into which the number is formatted.
 *   x - the number to convert.
 *   filler - the character used to pad out the converted number.
 * 
 * @return
 *   a pointer to the buffer.
 * 
 * @note
 * The buffer must be at least fieldSize + 1 characters in size.
 * 
 * If the fieldSize is too small for the converted integer, then the buffer will not contain
 * all of the digits representing the number.
 * 
 */
char * rightJustify(char *buf, int fieldSize, long x, char filler = ' ', char seperator = '\0') {
  boolean negativeSign = false;

  // Check for a -ve sign and remember this for later.
  // ensure we have |x| for conversion.
  if ((negativeSign = x < 0)) {
    x = -x;
  }

  // NULL terminate the string.
  char *p = buf + fieldSize;
  *p-- = '\0';

  // Load up the characters representing the number.
  int digitCount = 0;
  do {
    *p-- = x % 10 + '0';
    x /= 10;
    digitCount++;
    if (seperator && digitCount % 3 == 0 && x && p >= buf) {
      *p-- = seperator;
    }
  } while (x && p >= buf);

  // Do we need a negative sign? If so, put it in.
  if (p >= buf && negativeSign) {
    *p-- = '-';
  }

  // Pad out the string.
  while (p >= buf) {
    *p-- = filler;
  }
  return buf;
}



char * rightJustifyF(char *buf, int fieldSize, double x, int dpCnt, char filler = ' ', char seperator = '\0') {

  long iPart = (long) x;

  double fPart = x - (double) iPart;

  rightJustify(buf, fieldSize - 1 - dpCnt, iPart, filler, seperator);
  char *p = buf + fieldSize - 1 - dpCnt;
  if (p < buf + fieldSize) {
    *p++ = '.';
  }

  // When outputing the fractional component, we need our number to be positive.
  if (fPart < 0) {
    fPart = -fPart;
  }

  while (p < buf + fieldSize && dpCnt--) {
    double tmp = fPart * 10.0;
    int digit = (int)tmp;
    *p++ = digit + '0';
    fPart = tmp - digit;
  }

  buf[fieldSize] = '\0';
  return buf;
}



char * rightJustifyF(char *buf, int fieldSize, double x, char filler = ' ', char seperator = '\0') {
  return rightJustifyF(buf, fieldSize, x, 2, filler, seperator);
}



void dumpStr(const char * lbl, const char *buf, int bufSize) {
  Serial.print(lbl);
  Serial.print(": Hex: ");
  for (int i = 0; i < bufSize; i++) {
    Serial.print((byte) buf[i], HEX);
    Serial.print(' ');
  }
  Serial.println();
}



void logData (int hour, int minute, int second, bool timeValid,
              double lat, double lon, bool locValid,
              double alt, bool altValid, bool recordBroken,
              double hdop, bool hdopValid,
              int satCnt, bool satCntValid,
              double tempC1, double tempC2,
              double battV) {
  // Serial.println(F("Log data called"));
}


void updateDisplay(int hour, int minute, int second, bool timeValid,
              double lat, double lon, bool locValid,
              double alt, bool altValid, bool recordBroken,
              double hdop, bool hdopValid,
              int satCnt, bool satCntValid,
              double tempC1, double tempC2,
              double battV) {
  char buf[20];  // Buffer for padding up to 9 characters including string terminator.

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  int16_t x, y, dx, dy;
  uint16_t w, h;
  // Location


  // Time
 #ifdef TEST_MODE
  Serial.print("time: ");
    Serial.print(rightJustify(buf, 2, hour));
    Serial.print(":");
    Serial.print(rightJustify(buf, 2, minute, '0'));
    Serial.print(":");
    Serial.print(rightJustify(buf, 2, second, '0'));
    Serial.print(F(" valid: ")); Serial.println(timeValid ? "T": "F");
 #endif

  display.setTextSize(2);
  if (timeValid) {
    display.print(rightJustify(buf, 4, hour));
    display.print(":");
    display.print(rightJustify(buf, 2, minute, '0'));
    display.print(":");
    display.println(rightJustify(buf, 2, second, '0'));
  } else {
    display.println("  --:--:--");
  }

  // Altitude

#if defined(TEST_MODE)
  Serial.print("altf: "); Serial.print(buf);
  Serial.print(F(" valid: ")); Serial.println(altValid ? "T": "F");
#endif

  display.setTextSize(2); // larger font for key metric.
  if (altValid) {
    display.print(rightJustifyF(buf, 10, alt, 2, ' ', ','));
    x = display.getCursorX();
    y = display.getCursorY();
    display.getTextBounds((const char *) buf, x, y, &dx, &dy, &w, &h);
    display.setTextSize(1);
    x = display.getCursorX();
    display.println("m");
    dy = display.getCursorY();
    display.setCursor(x, dy);
    if (recordBroken) {
      display.print("R");
    }
    // Leave a small amount of space below the large font messages.
    display.setCursor(0, y + h + OLED_LINE_SPACING);
  } else {
    display.println("Alt: --");
    display.setCursor(0, display.getCursorY() + OLED_LINE_SPACING);
    display.setTextSize(1);
  }


  // Location
  if (locValid) {
    double tmp = abs(lon);
    display.print(rightJustifyF(buf, 9, tmp, 5));
    display.print(lon < 0 ? "W" : "E");

    display.print(F(" "));

    tmp = abs(lat);
    display.print(rightJustifyF(buf, 8, tmp, 5));
    display.println(lat < 0 ? "S": "N");
  } else {
    display.println(F("Lon / lat"));
  }
  // display.setCursor(0, display.getCursorY() + OLED_LINE_SPACING / 2);
#if defined (TEST_MODE)
  Serial.print(F("lon,lat: "));
    Serial.print(lon, 6); 
    Serial.print(",");
    Serial.print(lat, 6);
    Serial.print(F(" valid: ")); Serial.print(locValid ? "T ": "F ");
    Serial.print(", alt: ");
    Serial.print(alt, 3);
    Serial.print(F(" valid: ")); Serial.println(altValid ? "T": "F");
#endif

  // Quality
  display.print(F("H:"));
  display.print(rightJustifyF(buf, 6, hdop, 2));
  // if (hdopValid) {
  //   display.print(rightJustifyF(buf, 6, hdop, 2));
  // } else {
  //   display.print(F(" --.--"));
  // }
  display.print(F(". #S:"));
  display.println(rightJustify(buf, 4, satCnt));
  // if (satCntValid) {
  //   display.println(rightJustify(buf, 4, satCnt));
  // } else {
  //   display.println(F("  --"));
  // }
  // display.setCursor(0, display.getCursorY() + OLED_LINE_SPACING / 2);
#if defined (TEST_MODE)
  Serial.print(F("HDOP: "));
  Serial.print(hdop, 2);
  Serial.print(F(". #Sat: "));
  Serial.println(satCnt);
#endif

  // Temperature
#if defined(TEST_MODE)
  for (unsigned int i = 0; i < sizeof(buf); i++) {
    buf[i] = '*';
  }

  Serial.print("temps: ");
    Serial.print(tempC1, 2);
    Serial.print(", ");
    Serial.print(tempC2, 2);
    Serial.print(F(", heat: "));
    Serial.print(F(isHeaterOn() ? "on" : "off"));
    Serial.println();
#endif

  display.print(rightJustifyF(buf, 5, tempC1, 1));
  // dumpStr("T1", buf, 20);
  display.print("/");
  display.print(rightJustifyF(buf, 5, tempC2, 1));
  // dumpStr("T2", buf, 20);
  display.print("C");
  display.print(isHeaterOn() ? "*" : " ");
  display.print(" ");
  display.print(battV, 2);
  display.println("V");

  display.display();
}


void setup() {

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(OLED_MODEL, OLED_SCREEN_ADDRESS)) {
    Serial.begin(CONSOLE_BAUD);
    Serial.println(F("SSD1306 allocation failed"));
    int interval = 200;
    pinMode(LED_BUILTIN, OUTPUT);
    while(true) {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      delay(interval);
      interval = 1000 - interval;
    }
  }
  display.display();
  delay(1000);

  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(2);
  display.setTextColor(WHITE);


#ifdef TEST_MODE
  display.println("TEST MODE");
#else
  display.println("HAB Log");
#endif
  display.setTextSize(1);
  display.println(VERSION);
  display.print(F("Serial: "));
  display.display();
  Serial.begin(CONSOLE_BAUD);
      // Teensy appears to require a positive connection before Serial is true.
      // So, include a timeout mechanism to allow the code to execute.
      // TODO: Rearrange this and put startup progress messages on the OLED.
  int cnt = 2000;
  while (!Serial && cnt--)
    ;
  if (cnt) {
    display.println(F("connected."));
  } else {
    display.println(F("not connected."));
  }
  Serial.println("HAB logger.");
  Serial.println("Init Hab");
  Serial.println(VERSION);
  display.print(F("init hab: "));
  display.display();
  initHab(display);
  display.println(F("Init complete."));
  delay(5000);
  display.clearDisplay();
}


void loop() {
static int hour = 0, minute = 0, second = 0, satCnt = 0;
static double lat = 0.0, lon = 0.0, alt = 0.0, tempInternal = 0.0, tempExternal = 0.0, batteryVoltage = 0.0, hdop = 0.0;
static double prevTemp = 9999.99;
static bool recordBroken = false;
static bool locValid = false, altValid = false, satCntValid = false, timeValid = false, hdopValid = false;
bool newData = false;

//***************************
// TODO: Use isValid() to determine whether to read new values or not.
//       Use a latch for has valid data every been received instead of the isValid() value.
//       Maybe make the latch expire if no valid data for some time? (probably not).
  if (checkGPSData()) {
    newData = true;
    digitalWrite(LED_BUILTIN, ! digitalRead(LED_BUILTIN));
    if (isTimeValid()) {
      timeValid = true;
      hour = getHour();
      hour += TZ_OFFSET;
      if (hour >= 24) {
        hour -= 24;
      } else if (hour < 0) {
        hour += 24;
      }
      minute = getMinutes();
      second = getSecond();
    }
    if (isLocValid()) {
      locValid = true;
      lat = getLat();
      lon = getLon();
    }
    
    if (isAltValid()) {
      altValid = true;
      alt = getAlt();
      if (alt > CURR_ALTITUDE_RECORD) {
        recordBroken = true;
      }

    }
    if (isHdopValid()) {
      hdopValid = true;
      hdop = getHdop();
    }
    if (isSatCntValid()) {
      satCntValid = true;
      satCnt = getSatCnt();
    }
  }
  if (checkTemperatureData()) {
    newData = true;

    batteryVoltage = getBatteryVoltage();

    tempInternal = getTemperature(INTERNAL_TEMP);
    tempExternal = getTemperature(EXTERNAL_TEMP);
    if (HEATER_TEMP_SENSOR == INTERNAL_TEMP) {
      if (checkHeater(tempInternal, prevTemp)) {
        prevTemp = tempInternal;
      }
    } else if (HEATER_TEMP_SENSOR == EXTERNAL_TEMP) {
      if (checkHeater(tempExternal, prevTemp)) {
        prevTemp = tempExternal;
      }
    }
  }


  if (newData) {
    updateDisplay(hour, minute, second, timeValid, lat, lon, locValid, alt, altValid, recordBroken, hdop, hdopValid, satCnt, satCntValid, tempInternal, tempExternal, batteryVoltage);
    logData(hour, minute, second, timeValid, lat, lon, locValid, alt, altValid, recordBroken, hdop, hdopValid, satCnt, satCntValid, tempInternal, tempExternal, batteryVoltage);
  }
}

/*****************
 * Program to monitor and log a High Altitude Balloon
 * flight.
 *
 *  V1.02.01.00 20-05-2024
 *    * Removed "slowest log time" metric from log.
 *
 *  V1.02.00.00 20-05-2024
 *    * Added logging of the NMEA GPS sentences processed by TinyGPS++.
 *    * Added column headers for all of the record types to the log file.
 *
 *  V1.01.00.00 19-05-2024
 *    * Added logging to SD card.
 *
 *  V1.00.02.00 19-05-2024
 *    * Added altitude record indicator.
 *
 *  V1.00.00.00 16-05-2024 (?)
 *    * Initial version.
 *  
 */
// HAB stuff
#include "hab.h"
#include "Utility.h"
#include "Logger.h"


// OLED stuff.
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>



#define VERSION "v1.02.01.00"
// Baud rate of Serial console.

#define OLED_LINE_SPACING 4 // Additional space between lines when positioning cursor.

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_SSD1306 display(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, & OLED_PORT, OLED_RESET);





void dumpStr(const char * lbl, const char *buf, int bufSize) {
  Serial.print(lbl);
  Serial.print(": Hex: ");
  for (int i = 0; i < bufSize; i++) {
    Serial.print((byte) buf[i], HEX);
    Serial.print(' ');
  }
  Serial.println();
}



void logHeader() {
  // logMessage(F("$HAB,UTCTime,lat,lon,alt,hdop,satCnt,T1,T2,battV,sumLogTimeMs,logCnt,worstLogTimeMs"));
  // logMessage(F("$GPRMC,time,status,lat,ns,lon,ew,spdKnot,cog,date,mv,mvEW,posMode,navStatus,chksum"));
  // logMessage(F("$GPGGA,time,lat,ns,lon,ew,quality,numSV,hdop,alt,altU,sep,sepU,diffAge,diffStation,chksum"));
  // logMessage(F("$GNRMC,time,status,lat,ns,lon,ew,spdKnot,cog,date,mv,mvEW,posMode,navStatus,chksum"));
  // logMessage(F("$GNGGA,time,lat,ns,lon,ew,quality,numSV,hdop,alt,altU,sep,sepU,diffAge,diffStation,chksum"));
  logMessage("$HAB,UTCTime,lat,lon,alt,hdop,satCnt,T1,T2,battV,sumLogTimeMs,logCnt");
  logMessage("$GPRMC,time,status,lat,ns,lon,ew,spdKnot,cog,date,mv,mvEW,posMode,navStatus,chksum");
  logMessage("$GPGGA,time,lat,ns,lon,ew,quality,numSV,hdop,alt,altU,sep,sepU,diffAge,diffStation,chksum");
  logMessage("$GNRMC,time,status,lat,ns,lon,ew,spdKnot,cog,date,mv,mvEW,posMode,navStatus,chksum");
  logMessage("$GNGGA,time,lat,ns,lon,ew,quality,numSV,hdop,alt,altU,sep,sepU,diffAge,diffStation,chksum");

}

void logData (int hour, int minute, int second, bool timeValid,
              double lat, double lon, bool locValid,
              double alt, bool altValid, bool recordBroken,
              double hdop, bool hdopValid,
              int satCnt, bool satCntValid,
              double tempC1, double tempC2,
              double battV) {

static uint32_t prevLogTime = 0;
static uint32_t logInterval = LOG_LOW_RATE_MS;
static unsigned int logCnt = 0;
static uint32_t logCumulativeTimeMs = 0;


  uint32_t _now = millis();
  char logRec [200];
  char wrkBuf[100];      // Workbuffer for any numeric conversions we need to do.
  
    // Is it too soon to log the next record?
  if (_now - prevLogTime < logInterval) {
    return;       // Yes, so just return.
  }

  // Setup the parameters for the next logging point.
  prevLogTime = _now;
  // Calculate the interval to the next log time.
  if (alt >= LOG_RATE_HIGH_THRESHOLD_ALT + LOG_THRESHOLD_ALT_TOL) {
    logInterval = LOG_HIGH_RATE_MS;
  } else if (alt <= LOG_RATE_HIGH_THRESHOLD_ALT - LOG_THRESHOLD_ALT_TOL) {
    logInterval = LOG_LOW_RATE_MS;
  }

  // Log the current data.
  // The record layout is "$HAB,time,lat,lon,alt,hdop,satCnt,C1,C2,battV"
  strcpy(logRec, "$HAB,");

  sprintf(wrkBuf, "%d:%02d:%02d,", hour, minute,second);
  strcat(logRec,wrkBuf);

  sprintf(wrkBuf, "%.6f,%.6f,%.2f,%.4f,%d,", lat, lon, alt, hdop, satCnt);
  strcat(logRec, wrkBuf);

  sprintf(wrkBuf, "%.2f,%.2f,%.2f,", tempC1, tempC2, battV);
  strcat(logRec, wrkBuf);

  sprintf(wrkBuf, "%lu,%u", logCumulativeTimeMs, logCnt);
  strcat(logRec, wrkBuf);

  if (!logMessage(logRec)) {
    Serial.print(F("*** Failed to log: ")); Serial.println(logRec);
  }

  uint32_t endTime = millis();

  uint32_t logTime = endTime - _now;
  logCumulativeTimeMs += logTime;
  logCnt++;
  Serial.print(F("Log msg: ")); Serial.print(logCnt); Serial.print(F(" ms=")); Serial.println(logTime);
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
  if (generateLogFileName(LOG_FILE_NAME_PREFIX, LOG_FILE_NAME_EXT)) {
    display.print(F("Log:"));
    display.println(getLogFileName());
    Serial.print(F("log filename: "));
    Serial.println(getLogFileName());
    logHeader();
  } else {
    display.println(F("NO SD CARD!!"));
    Serial.println(F("No SD Card"));
  }

  display.println(F("Init complete."));
  display.display();
  delay(5000);
  display.clearDisplay();
}


void loop() {
static int utcHour = 0, localHour = 0, minute = 0, second = 0, satCnt = 0;
static double lat = 0.0, lon = 0.0, alt = 0.0, tempInternal = 0.0, tempExternal = 0.0, batteryVoltage = 0.0, hdop = 0.0;
static double prevTemp = 9999.99;
static bool recordBroken = false;
static bool locValid = false, altValid = false, satCntValid = false, timeValid = false, hdopValid = false;
bool newData = false;

  checkAltitudeRecord(alt);       // Check the altitude and if appropriate, set or blink the record LED.

  if (checkGPSData()) {
    newData = true;
    digitalWrite(LED_BUILTIN, ! digitalRead(LED_BUILTIN));
    if (isTimeValid()) {
      timeValid = true;
      localHour = utcHour = getHour();
      localHour += TZ_OFFSET;
      if (localHour >= 24) {
        localHour -= 24;
      } else if (localHour < 0) {
        localHour += 24;
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
      if (alt > ALTITUDE_RECORD_LOW) {
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
    updateDisplay(localHour, minute, second, timeValid, lat, lon, locValid, alt, altValid, recordBroken, hdop, hdopValid, satCnt, satCntValid, tempInternal, tempExternal, batteryVoltage);
    logData(utcHour, minute, second, timeValid, lat, lon, locValid, alt, altValid, recordBroken, hdop, hdopValid, satCnt, satCntValid, tempInternal, tempExternal, batteryVoltage);
  }
}

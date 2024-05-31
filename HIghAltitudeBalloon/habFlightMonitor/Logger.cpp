#include "Logger.h"
#include "Utility.h"
#include "hab_config.h"

#include <Arduino.h>

char logFileName[MAX_LOG_FILE_NAME_SIZE];

// Set to true if we could successfully create a log file.
bool loggingEnabled = false;


// For Teensy 3.5 & 3.6 & 4.1 on-board: BUILTIN_SDCARD
#define SD_CD_PIN = BUILTIN_SDCARD;

#if SD_FAT_TYPE == 0
SdFat sd;
File file;
File root;
#elif SD_FAT_TYPE == 1
SdFat32 sd;
File32 file;
File32 root;
#elif SD_FAT_TYPE == 2
SdExFat sd;
ExFile file;
ExFile root;
#elif SD_FAT_TYPE == 3
SdFs sd;
FsFile file;
FsFile root;
#endif  // SD_FAT_TYPE


// Try to select the best SD card configuration.
#if HAS_SDIO_CLASS
#define SD_CONFIG SdioConfig(FIFO_SDIO)
#elif ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK)
#else  // HAS_SDIO_CLASS
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SPI_CLOCK)
#endif  // HAS_SDIO_CLASS




int logTimeHistory [MAX_LOG_HISTORY] = {};
int logTimeIndex = 0;

void recordLogTimeinHistory(int logTime) {
  // Record the log time in the history (if there is space).
  if (logTimeIndex < MAX_LOG_HISTORY) {
    logTimeHistory[logTimeIndex++] = logTime;
  }
}

void resetLogTimeHistory() {
  for (int i = 0; i < MAX_LOG_HISTORY; i++) {
    logTimeHistory[i] = -1;
  }
  logTimeIndex = 0;
}

int getLogTimeFromHistory(int index) {
  if (index < 0 || index > MAX_LOG_HISTORY) {
    return -1;
  }
  return logTimeHistory[index];
}



bool generateLogFileName(const char *, const char *) {
  char wrkBuf[80];
  if (sd.begin(SD_CONFIG)) {
    int fileCnt = 0;
    if (root.open("/")) {
      while (file.openNext(&root, O_RDONLY)) {
        if (file.isFile()) {
          file.getName(wrkBuf, sizeof(wrkBuf));
          Serial.print(wrkBuf);
          Serial.print(": (");
          Serial.print(file.size());
          Serial.println(" bytes)");
          fileCnt++;
        }
        file.close();
      }
      Serial.print("Found "); Serial.print(fileCnt); Serial.println(" files.");
      strcpy(logFileName, LOG_FILE_NAME_PREFIX);
      strcat(logFileName, rightJustify(wrkBuf, 4, fileCnt, '0'));
      strcat(logFileName, ".");
      strcat(logFileName, LOG_FILE_NAME_EXT);
      if (file.open(logFileName, O_RDWR | O_CREAT | O_AT_END)) {
        file.close();
        loggingEnabled = true;
      }
    }
  }
  return loggingEnabled;
}

const char * getLogFileName() {
  return logFileName;
}


bool isLoggingEnabled() {
  return loggingEnabled;
}


/**
 * Log a message to the SD Card.
 *
 * Return - the time (ms) required to log the record.
 */
int logMessage(const char * msg) {
  // Serial.print("Request to log: "); Serial.println(msg);
  uint32_t _startTime = millis();
  if (!isLoggingEnabled()) {
    return -1;
  }

  if (file.open(logFileName, O_RDWR | O_CREAT | O_AT_END)) {
    // Serial.println("File opened, writing message");
    file.println(msg);
    // file.flush();
    file.close();
    return millis() - _startTime;
  }
  return -1;
  
  // boolean result = file.println(msg);
  // file.flush();
  // return result;
}


// int logMessage(const __FlashStringHelper * msg) {
//   const int MaxMessageSize = 200;
//   char buf[MaxMessageSize];

//   char *dest = buf;
//   PGM_P p = reinterpret_cast<PGM_P>(msg);
//   unsigned char ch;
//   do {
//     ch = pgm_read_byte(p++);
//     *dest++ = ch;
//   } while (ch && (dest - buf) < MaxMessageSize);
//   if (dest - buf >= MaxMessageSize) {
//     buf[MaxMessageSize - 1] = '\0';
//   }

//   logMessage(buf);
// }


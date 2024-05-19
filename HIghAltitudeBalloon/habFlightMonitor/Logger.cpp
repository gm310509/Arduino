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
      if (file.open(logFileName, O_WRONLY | O_CREAT)) {
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



bool logMessage(const char * msg) {
  Serial.print("Request to log: "); Serial.println(msg);
  if (!isLoggingEnabled()) {
    return false;
  }

  if (file.open(logFileName, O_RDWR | O_CREAT | O_AT_END)) {
    Serial.println("File opened, writing message");
    file.println(msg);
    // file.flush();
    file.close();
    return true;
  }
  return false;
}


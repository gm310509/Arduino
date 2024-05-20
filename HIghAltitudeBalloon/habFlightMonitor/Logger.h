#ifndef _LOGGER_H
#define _LOGGER_H

#include <Arduino.h>

#include "hab_config.h"
#include <SdFat.h>
#include <sdios.h>

#define MAX_LOG_FILE_NAME_SIZE 12
// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 3


extern bool generateLogFileName(const char *, const char *);
extern const char * getLogFileName();

extern bool isLoggingEnabled();

extern bool logMessage(const char *);
// extern bool logMessage(const __FlashStringHelper *);

#endif
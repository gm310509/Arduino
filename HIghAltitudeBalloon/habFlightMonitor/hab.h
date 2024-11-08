#ifndef _HAB_H
#define _HAB_H

#include "hab_config.h"

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))


#ifdef __cplusplus
extern "C"{
#endif

enum AltitudeRecordState {
  NoRecord, LowRecord, HighRecord
};

enum AltitudeRecordState getAaltitudeRecordState();

extern void initHab(Adafruit_SSD1306 & display);
extern int checkGPSData();
extern int checkTemperatureData();

extern double getLat(void);
extern double getLon(void);
extern boolean isLocValid(void);

extern double getAlt(void);
extern boolean isAltValid(void);
extern void checkAltitudeRecord(double);

extern double getHdop(void);
extern boolean isHdopValid(void);

extern int getSatCnt(void);
extern boolean isSatCntValid(void);

extern int getHour(void);
extern int getMinutes(void);
extern int getSecond(void);
extern boolean isTimeValid(void);


extern double getTemperature(int);

extern int getTempUpdCnt(void);
extern uint32_t getSumTempUpdateTime(void);
extern uint32_t getSlowestTempUpdateTime(void);
extern void resetTempMetrics(void);


extern boolean checkHeater(double, double);
extern boolean heaterOn(boolean);
extern boolean isHeaterOn();

extern double getBatteryVoltage(void);

#ifdef __cplusplus
}
#endif

#endif

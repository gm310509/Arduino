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

extern void initHab(Adafruit_SSD1306 & display);
extern int checkGPSData();
extern int checkTemperatureData();

extern float getLat(void);
extern float getLon(void);
extern boolean isLocValid(void);

extern float getAlt(void);
extern boolean isAltValid(void);

extern float getHdop(void);
extern boolean isHdopValid(void);

extern int getSatCnt(void);
extern boolean isSatCntValid(void);

extern int getHour(void);
extern int getMinutes(void);
extern int getSecond(void);
extern boolean isTimeValid(void);


extern float getTemperature(int);


extern boolean checkHeater(float, float);
extern boolean heaterOn(boolean);
extern boolean isHeaterOn();

extern float getBatteryVoltage(void);

#ifdef __cplusplus
}
#endif

#endif

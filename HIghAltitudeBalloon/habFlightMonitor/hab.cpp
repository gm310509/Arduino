#include <Arduino.h>

/****************************************************
 * GPS Stuff
 ***************************************************/
#include <TinyGPS++.h>

TinyGPSPlus gps;

/****************************************************
 * Temperature Stuff
 ****************************************************/
// Include the Dallas 18B20 temperature sensor libraries.
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 10
#define TEMPERATURE_PRECISION 9

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device addresses
int temperatureSensorCnt = 0;
#define MAX_SENSOR_CNT 2
DeviceAddress tempSensorAddr[MAX_SENSOR_CNT];
double temperature[MAX_SENSOR_CNT];

// Include our library.

#include "hab.h"

#ifdef ARDUINO_AVR_UNO
#include <SoftwareSerial.h>
#define RX 2
#define TX 3
SoftwareSerial GPS_PORT(RX, TX);
#define SERIAL_TYPE "Software"
#else
#define SERIAL_TYPE "Hardware"
#endif



#define GPS_BAUD 9600



int checkGPSData() {

#if defined(TEST_MODE)
  while (GPS_PORT.available()) {
    char ch = GPS_PORT.read();
     Serial.print(ch);
    gps.encode(ch);
  }
  // TODO Remove this in favour of using the actual GPS data to determine "new data" status.
  static unsigned long lastUpdateTime;  
  unsigned long updateInterval = 5000;       // TODO remove this

  unsigned long _now = millis();
  if (_now - lastUpdateTime > updateInterval) {
    lastUpdateTime = _now;
    // TODO: Get GPS Data.
    return 1;
  }
  return 0;
#else
  while (GPS_PORT.available()) {
    char ch = GPS_PORT.read();
    Serial.print(ch);
    gps.encode(ch);
  }
  if (gps.location.isUpdated()
        || gps.altitude.isUpdated()
        || gps.time.isUpdated()
        // || gps.hdop.isUpdated()
        || gps.satellites.isUpdated()
      ) {
    return 1;
  }
  return 0;
#endif
}



int checkTemperatureData() {
static unsigned long lastUpdateTime;
unsigned long updateInterval = 1000;

  unsigned long _now = millis();
  if (_now - lastUpdateTime > updateInterval) {
    lastUpdateTime = _now;
    // Serial.println(F("Checking temperature"));
    sensors.requestTemperatures();
    for (unsigned int i = 0; i < ARRAY_SIZE(tempSensorAddr); i++) {
      temperature[i] = sensors.getTempC(tempSensorAddr[i]);
    }
    return 1;
  }
  return 0;
}





double getLat(void) {
#ifdef TEST_MODE
  return random(0,90L*60L*60L) / 3600.0;
#else
  return gps.location.lat();
#endif
}

double getLon(void) {
#ifdef TEST_MODE
  return random(0,180L*60L*60L) / 3600.0;
#else
  return gps.location.lng();
#endif
}

boolean isLocValid(void) {
#ifdef TEST_MODE
  return true;
#else
  return gps.location.isValid();
#endif
}



double getAlt(void) {
#ifdef TEST_MODE
  return random(0, 60000000L) / 1000.0;
#else
  return gps.altitude.meters();
#endif
}

boolean isAltValid(void) {
#ifdef TEST_MODE
  return true;
#else
  return gps.altitude.isValid();
#endif
}


double getHdop(void) {
#ifdef TEST_MODE
  return random(0, 5000L) / 1000.0;
#else
  return gps.hdop.value();
#endif
}


boolean isHdopValid(void) {
#ifdef TEST_MODE
  return true;
#else
  return gps.hdop.isValid();
#endif
}



int getSatCnt(void) {
#ifdef TEST_MODE
  return random(0, 20);
#else
  return gps.satellites.value();
#endif
}



boolean isSatCntValid(void) {
#ifdef TEST_MODE
  return true;
#else
  return gps.satellites.isValid();
#endif
}


int getHour(void) {
#ifdef TEST_MODE
  return random(0,24);
#else
  // Serial.print("h="); Serial.println(gps.time.hour());
  return gps.time.hour();
#endif
}

int getMinutes(void) {
#ifdef TEST_MODE
  return random(0,60);
#else
  // Serial.print("m="); Serial.println(gps.time.minute());
  return gps.time.minute();
#endif
}

int getSecond(void) {
#ifdef TEST_MODE
  return random(0,60);
#else
  // Serial.print("s="); Serial.println(gps.time.second());
  return gps.time.second();
#endif
}


boolean isTimeValid(void) {
#ifdef TEST_MODE
  return true;
#else
  return gps.time.isValid();
#endif
}




double getTemperature(int sensorId) {
#ifdef TEST_MODE
  return random(0,160)/2.0 - 40;
#else
  if (sensorId >= 0 && sensorId < ARRAY_SIZE(temperature)) {
    return temperature[sensorId];
  } else {
    return -99.99;
  }
#endif
}



static boolean heaterOnInd = false;

boolean heaterOn(boolean heatOn) {
  boolean prevState = isHeaterOn();
  digitalWrite(HEATER_CONTROL_PIN, heatOn ? HIGH : LOW);
  heaterOnInd = heatOn;
  return prevState;
}


boolean isHeaterOn() {
  return heaterOnInd;
}



boolean checkHeater(double currentTemp, double prevTemp) {
  boolean tempChangedInd = false;
  // Has the temperature changed?
  if ( (abs(currentTemp - prevTemp) >= TEMP_DIFF_THRESH)) {
    // Serial.println("Temperature changed.");
    // Serial.print("Heater: on: "); Serial.println(isHeaterOn() ? "Yes": "No");
    // Serial.print("Temp: "); Serial.println(currentTemp);
// Yes, it has changed.
    tempChangedInd = true;
    if (currentTemp <= HEATER_ON_TEMP && !isHeaterOn()) {
      heaterOn(true);
    } else if (currentTemp > HEATER_OFF_TEMP && isHeaterOn()) {
      heaterOn(false);
    }
  }

  return tempChangedInd;
}


double getBatteryVoltage() {
#ifdef TEST_MODE
  return random(300,650)/100.0;
#else
  unsigned int aValue = analogRead(VOLTAGE_MEASURE);
  // Serial.print(F("Analog Read Value: ")); Serial.println(aValue);
    // voltage = aValue / aRange * expected divider voltage (3V) * divider scale (0.5) ^ -1
  return aValue / 1023.0 * ((double)VREF) / ((double)DIVIDER_RATIO_GND);
#endif
}




/*********************************************
 * Initialisation
 ********************************************/


// function to print a device address
void printTempSensorAddress(DeviceAddress deviceAddress, bool newLine = false)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}



void initTemperatureSensors(Adafruit_SSD1306 & display) {
  Serial.print(F("Locating DS18B20 devices..."));
  sensors.begin();

  Serial.print(F("Found "));
  temperatureSensorCnt = sensors.getDeviceCount();
  Serial.print(temperatureSensorCnt);
  Serial.println(F(" devices."));
  display.print("Found ");
  display.print(temperatureSensorCnt);
  display.print(F(" ds18B20"));
  display.display();

  // report parasite power requirements
  Serial.print("Parasite power is: ");
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");

  for (unsigned int i = 0; i < ARRAY_SIZE(tempSensorAddr); i++) {
    if (sensors.getAddress(tempSensorAddr[i], i)) {
      sensors.setResolution(tempSensorAddr[i], TEMPERATURE_PRECISION);
      Serial.print(F("Found ds18b20 "));
      Serial.print(i);
      Serial.print(F(" at 0x"));
      printTempSensorAddress(tempSensorAddr[i]);
      Serial.print(F(". Resolution: "));
      Serial.println(sensors.getResolution(tempSensorAddr[i]));
    } else {
      Serial.print(F("Unable to find address for Device "));
      Serial.println(i);
    }
  }
}


void initHab(Adafruit_SSD1306 & display) {
  display.print(F("TZ: "));
  display.println(TZ_OFFSET);

#ifdef TEST_MODE
  randomSeed(analogRead(A0));
#else
  initTemperatureSensors(display);
#endif
  pinMode(HEATER_CONTROL_PIN, OUTPUT);
  heaterOn(true);
  delay(100);
  heaterOn(false);
  GPS_PORT.begin(GPS_BAUD);
}

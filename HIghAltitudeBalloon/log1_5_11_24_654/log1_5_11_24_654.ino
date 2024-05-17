#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <TinyGPS++.h>
#include <SD.h>
#include <SPI.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define ONE_WIRE_BUS 10
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

TinyGPSPlus gps;
File logFile;

unsigned long lastTempRequest = 0;
unsigned long delayInMillis = 750; // Conversion time for DS18B20
bool awaitingTemperature = false;

// Addresses of the DS18B20 sensors
DeviceAddress sensorTI = { 0x28, 0xA5, 0xFA, 0x75, 0xD0, 0x01, 0x3C, 0x5B };
DeviceAddress sensorTX = { 0x28, 0x28, 0xAE, 0x75, 0xD0, 0x01, 0x3C, 0x71 };

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600); // Start Serial1 for GPS

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for(;;); // Don't proceed, loop forever
  }
  display.display();
  delay(2000);
  display.clearDisplay();

  // Initialize DS18B20 sensors
  sensors.begin();

  // Check for each sensor's presence
  if (!sensors.getAddress(sensorTI, 0)) {
    Serial.println("Sensor TI not found");
    while(1);
  }
  if (!sensors.getAddress(sensorTX, 1)) {
    Serial.println("Sensor TX not found");
    while(1);
  }

  // Initialize SD Card
  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("SD Card initialization failed!");
    while(1);
  }
  logFile = SD.open("datalog.txt", FILE_WRITE);
  if (!logFile) {
    Serial.println("Failed to open datalog.txt for writing!");
    while(1);
  }
}

void loop() {
  while (Serial1.available() > 0) {
    gps.encode(Serial1.read());
  }

  if (millis() - lastTempRequest >= delayInMillis) {
    sensors.requestTemperatures(); // Send the command to get temperatures
    float tempC1 = sensors.getTempC(sensorTI);
    float tempC2 = sensors.getTempC(sensorTX);
    updateDisplayAndLog(tempC1, tempC2);
    lastTempRequest = millis();
  }
}

void updateDisplayAndLog(float tempC1, float tempC2) {
  display.clearDisplay();
  display.setTextSize(1); // Normal font size for better data visibility
  display.setTextColor(WHITE);
  display.setCursor(0,0);

  // Display altitude or no GPS if not available
  if (gps.location.isValid() && gps.altitude.isValid()) {
    display.print("Alt: ");
    display.print(gps.altitude.meters()); // Display altitude in meters
    display.println("m");
  } else {
    display.println("Waiting For GPS");
  }

  // Display temperatures
  display.print("IT: ");
  display.print(tempC1);
  display.println("C");

  display.print("XT: ");
  display.print(tempC2);
  display.println("C");

  // Display GPS time if available
  if (gps.time.isValid()) {
    int hour = gps.time.hour() - 6; // Convert UTC to CST
    if (hour < 0) hour += 24;
    display.print("Time: ");
    display.print(hour);
    display.print(":");
    display.print(gps.time.minute(), 2);
    display.print(":");
    display.println(gps.time.second(), 2);
  }

  display.display(); // Update display with all the new info

  // Log data to SD card
  logDataToSD(tempC1, tempC2);
}

void logDataToSD(float tempC1, float tempC2) {
  if (logFile) {
    if (gps.location.isValid()) {
      logFile.print("Lat: "); logFile.print(gps.location.lat(), 6);
      logFile.print(", Long: "); logFile.println(gps.location.lng(), 6);
    }
    if (gps.altitude.isValid()) {
      logFile.print("Altitude: "); logFile.print(gps.altitude.meters()); logFile.println(" m");
    }
    logFile.print("Internal Temp: "); logFile.print(tempC1); logFile.println(" C");
    logFile.print("External Temp: "); logFile.print(tempC2); logFile.println(" C");

    if (gps.date.isValid()) {
      logFile.print("Date: ");
      logFile.print(gps.date.month());
      logFile.print("/");
      logFile.print(gps.date.day());
      logFile.print("/");
      logFile.println(gps.date.year());
    }

    if (gps.time.isValid()) {
      int hour = gps.time.hour() - 6; // Adjust for CST
      if (hour < 0) hour += 24;
      logFile.print("Time: ");
      logFile.print(hour);
      logFile.print(":");
      logFile.print(gps.time.minute(), 2);
      logFile.print(":");
      logFile.println(gps.time.second(), 2);
    }

    logFile.flush(); // Ensure data is written to the file
  }
}

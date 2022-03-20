#include <Arduino.h>

#ifndef _BRIGHTNESS_LIB
#include "Brightness.h"
#define _BRIGHTNESS_LIB


void Brightness::printDebugInfo() {
  Serial.println(F("Brightness info:"));
  Serial.print(F("  LDR Pin: ")); Serial.println(_ldrPin);
  Serial.print(F("  digipot Pins: "));
  for (int i = 0; i < NUM_CONTROL_PINS; i++) {
    if (i) {
      Serial.print(F(", "));
    }
    Serial.print(_digiPotPins[i]);
  }
  Serial.println();
}

void Brightness::setBrightnessLevel(int level) {
  if (level < 0 || level > NUM_CONTROL_PINS) {
    return;
  }
  if (level == _currentBrightnessLevel) {
    return;
  }
  
//  Serial.print(F("Settting brightness to: ")); Serial.println(level);
//  Serial.print(F("            previously: ")); Serial.println(_currentBrightnessLevel);

  if (_currentBrightnessLevel < NUM_CONTROL_PINS) {
    digitalWrite(_digiPotPins[_currentBrightnessLevel], LOW);    // Turn off this level
  }
  if (level < NUM_CONTROL_PINS) {
    digitalWrite(_digiPotPins[level], HIGH);     // Turn on this level
  }
  _currentBrightnessLevel = level;
}


int Brightness::checkLightLevels() {
  _currentLightLevel = analogRead(_ldrPin);
  _minLightLevel = min(_minLightLevel, _currentLightLevel);
  _maxLightLevel = max(_maxLightLevel, _currentLightLevel);

  if (_currentLightLevel > 300) {
    setBrightnessLevel(0);
  } else if (_currentLightLevel > 200) {
    setBrightnessLevel(1);
  } else if (_currentLightLevel > 100) {
    setBrightnessLevel(2);
  } else if (_currentLightLevel > 50) {
    setBrightnessLevel(3);
  } else {
    setBrightnessLevel(4);
  }
}

#endif

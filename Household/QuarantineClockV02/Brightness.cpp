#include <Arduino.h>
#include "Brightness.h"


/**
 * Brightness Constructor.
 * Store the control pin specifications and
 * set the ports as needed.
 * 
 * Note: The digiPotPins must be an array of 4 integers in this version.
 */
Brightness::Brightness(int ldrPin, int digiPotPins[]) {
  _ldrPin = ldrPin;
  for (int i = 0; i < NUM_CONTROL_PINS; i++) {
    _digiPotPins[i] = digiPotPins[i];
  }

  for (int i = 0; i < NUM_CONTROL_PINS; i++) {
    pinMode(_digiPotPins[i], OUTPUT);
    digitalWrite (_digiPotPins[i], LOW);     // Turn the transistor (and thus its resistor off
  }                                          // Turn on the first transistor which means no resistors 
  digitalWrite(_digiPotPins[0], HIGH);       // in the variable resistance chain => Max brightness

  resetLightMetrics();                        // Reset the metrics of min/max brightness observed.
}

/**
 * Print debugging information.
 */
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


/**
 * Set the brightness level to a number between 0 and NUM_CONTROL_PINS
 * inclusive.
 * 
 * Thus if there are 4 control pins, there will be 5 brightness levels (0, 1, 2, 3 and 4).
 */
void Brightness::setBrightnessLevel(int level) {
  if (level < 0 || level > NUM_CONTROL_PINS) {
    return;
  }
  if (level == _currentBrightnessLevel) {       // Don't do anything if the level didn't change.
    return;
  }
  
//  Serial.print(F("Settting brightness to: ")); Serial.println(level);
//  Serial.print(F("            previously: ")); Serial.println(_currentBrightnessLevel);

  digitalWrite(_digiPotPins[_currentBrightnessLevel], LOW);    // Turn off this level
  digitalWrite(_digiPotPins[level], HIGH);      // Turn on this level
  _currentBrightnessLevel = level;              // Save this new level for the next check.
}

/**
 * Reads the LDR and adjusts the brightness according to a
 * hard coded table of values.
 */
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

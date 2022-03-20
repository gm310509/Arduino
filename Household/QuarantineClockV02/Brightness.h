/**
 * Control the brightness of the LED's using a (home made) digipot circuit
 * 
 * https://en.wikipedia.org/wiki/Digital_potentiometer
 * https://components101.com/articles/how-digital-potentiometer-works-and-how-to-use-it
 * 
 */

 class Brightness {
  public:
    Brightness(int ldrPin, int digiPotPins[]) {
      _ldrPin = ldrPin;
      for (int i = 0; i < NUM_CONTROL_PINS; i++) {
        _digiPotPins[i] = digiPotPins[i];
      }
      resetLightMetrics();
    }
    
    static const int NUM_CONTROL_PINS = 4;
    
    int checkLightLevels();

    int getLightLevel() {
      return _currentLightLevel;
    }
    
    int getMinLightLevel() {
      return _minLightLevel;
    }
    
    int getMaxLightLevel() {
      return _maxLightLevel;
    }

    void resetLightMetrics() {
      _minLightLevel = 1024;
      _maxLightLevel = 0;
    }
    
    int getBrightnessLevel() {
      return _currentBrightnessLevel;
    }

    void setBrightnessLevel(int level);


    void printDebugInfo();
    

  private:
    int _digiPotPins[NUM_CONTROL_PINS];
    int _currentBrightnessLevel = 0;
    int _currentLightLevel = 0;
    int _minLightLevel = 1024;
    int _maxLightLevel = 0;
    int _ldrPin;

};

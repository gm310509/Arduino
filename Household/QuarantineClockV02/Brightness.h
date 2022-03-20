#ifndef _BRIGHTNESS_LIB
#define _BRIGHTNESS_LIB

/**
 * Control the brightness of the LED's using a (home made) digipot circuit
 * 
 * https://en.wikipedia.org/wiki/Digital_potentiometer
 * https://components101.com/articles/how-digital-potentiometer-works-and-how-to-use-it
 * 
 */

 class Brightness {
  public:
    /**
     * Brightness Constructor.
     * Store the control pin specifications and
     * set the ports as needed.
     * 
     * Note: The digiPotPins must be an array of 4 integers in this version.
     */
    Brightness(int ldrPin, int digiPotPins[]);

    /**
     * The exact (i.e. not maximum, but exact) number of digiPot control pins
     * that must be specified to the constructor.
     */
    static const int NUM_CONTROL_PINS = 4;

    /**
     * Reads the LDR and adjusts the brightness according to a
     * hard coded table of values.
     */
    int checkLightLevels();

    /*
     * Return the most recently observed ambient light level.
     */
    int getLightLevel() {
      return _currentLightLevel;
    }
    
    /*
     * Return the lowest observed ambient light level.
     */
    int getMinLightLevel() {
      return _minLightLevel;
    }
    
    /*
     * Return the highest observed ambient light level.
     */
    int getMaxLightLevel() {
      return _maxLightLevel;
    }

    /*
     * Reset the light min and max metrics so a new range can be measured.
     */
    void resetLightMetrics() {
      _minLightLevel = 1024;
      _maxLightLevel = 0;
    }
    
    /*
     * Return the current brightness level - i.e. the current digiPot setting.
     */
    int getBrightnessLevel() {
      return _currentBrightnessLevel;
    }
    
    /**
     * Set the brightness level to a number between 0 and NUM_CONTROL_PINS
     * inclusive.
     * 
     * Thus if there are 4 control pins, there will be 5 brightness levels (0, 1, 2, 3 and 4).
     */
    void setBrightnessLevel(int level);

    /**
     * Print debugging information.
     */
    void printDebugInfo();
    

  private:
    int _digiPotPins[NUM_CONTROL_PINS];
    int _currentBrightnessLevel = 0;
    int _currentLightLevel = 0;
    int _minLightLevel = 1024;
    int _maxLightLevel = 0;
    int _ldrPin;

};

#endif

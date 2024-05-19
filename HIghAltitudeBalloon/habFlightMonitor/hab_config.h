/*
 * Configuration parameters for the hab logger program
 */

 /* Revision History
  *
  * 2024-05-18 Added Timezone offset and other configuration constants. 
  *
  * 2024-05-17 Initial checkin
  *
  */

/** IMPORTANT *** IMPORTANT *** IMPORTANT * IMPORTANT **
 ** The following #define should be commented out for an actual flight!
 ** IMPORTANT *** IMPORTANT *** IMPORTANT * IMPORTANT **/

// Uncomment to enable test mode - primarily generates random numbers
// in place of the sensor data.

// #define TEST_MODE


// The current altitude record.
#define ALTITUDE_RECORD_LOW 35043L
#define ALTITUDE_RECORD_HIGH 40000L

#define RECORD_LED_PIN  35

// 0 = UTC
// 8 = China, Singapore, Western Australia
// 10 - Eastern Australia (standard Time)
// 11 - Eastern Australia - daylight time (Not QLD)
#define TZ_OFFSET 0

// Temperature Sensors
#define INTERNAL_TEMP 0
#define EXTERNAL_TEMP 1

// Heater controlled by
#define HEATER_TEMP_SENSOR INTERNAL_TEMP


// The port the GPS is connected to.
#define GPS_PORT  Serial1
// Baud rate of the GPS.
#define GPS_BAUD 9600

// The port that the OLED is connected to.
#define OLED_PORT Wire

#define OLED_MODEL SSD1306_SWITCHCAPVCC
#define OLED_SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define OLED_SCREEN_WIDTH 128 // OLED display width, in pixels
#define OLED_SCREEN_HEIGHT 64 // OLED display height, in pixels



// DIO Pin Heater control is connected to.
#define HEATER_CONTROL_PIN  36

/* Temperature control parameters.
 * Used to determine if the temperature has materialy changed
 */
// Amount temperature must change before it is officially a change in value.
#define TEMP_DIFF_THRESH 0.1

// #define HEATER_ON_TEMP   27.0
// #define HEATER_OFF_TEMP  27.5
// Heater is turned on at this temperature or lower. Celsius.
#define HEATER_ON_TEMP   0.0
// Heater is turned off at this temperature or higher. Celsius.
#define HEATER_OFF_TEMP  0.5

// Pin from which voltages are measured.
#define VOLTAGE_MEASURE     A0
// Reference voltage as defined by the MCU. Typically the MCU's VDD as measured by a multimeter.
#define VREF 3.25

/* Ratio of the voltage divider fed into the VOLTAGE_MEASUER pin.
 * Example if the divider is two resistors of equal value, then the divider
 * produces 1/2 the voltage at the top of the divider.
 * If the divider consists of +V -> 2R -(VOLTAGE_MEASURE) -> R -> GND, then
 * the divider produces 1/3 of the voltage at VOLTAGE_MEASURE. Thus this would
 * be set to 1/3.
 */
#define DIVIDER_RATIO_GND 0.5


// Baud rate of the Serial (PC USB connection) device.
#define CONSOLE_BAUD 115200


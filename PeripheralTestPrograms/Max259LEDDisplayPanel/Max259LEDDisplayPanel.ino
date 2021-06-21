#include <LedControl.h>     // For MAX7219 LED Display Module.
/**
 * Tests the 8 digit LED display panel.
 * 
 * The test program outputs: 
 * * a display pattern of the digits 1 through 8 inclusive,
 *   then
 * * a positive and negative integer (one at a time)
 *   then
 * * an endless counter that increments in intervals of 1 second.
 * 
 * 
 */

int counter = 0;
unsigned long counterLastUpdateTime = 0;
const unsigned long COUNTER_UPDATE_INTERVAL = 1000;

/* Setup the ledDisplay
 *  LedControl(Data, Clock, ChipSelect, NumPanels)
 *    Data is 4
 *    Clock is 6
 *    ChipSelect is 5
 */
LedControl ledDisplay = LedControl(4,6,5,1);

/**
 * Display the digits 1 to 8 in positions 7 (Left most)
 * to 0 (right most).
 */
void displayOneToEight() {
  for (int i = 1; i <= 8; i++) {
    // 8 - i = digit position. Left Most is 7
    // and it should display a 1, therefore 8-1 = 7
    // Similarly for the 8 digit. The 8 goes in 
    // position 0. Thus 8 - 8 = 0.
    ledDisplay.setChar(0, 8 - i , i, false);
  }
}

void outputInteger (int value) {
  ledDisplay.clearDisplay(0);
  boolean negative = value < 0;
  if (negative) {
    value = -value;
  }

  if (value == 0) {
    ledDisplay.setChar(0, 0, 0, false);
    return;
  }

  int position = 0;
  while (position < 8 && value) {
    int digit = value % 10;
    value = value / 10;
    ledDisplay.setChar(0, position, digit, false);
    position++;
  }
  if (negative && position < 8) {
    ledDisplay.setChar(0, position, '-', false);
  }
}


void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(1);
  }

  Serial.println("Max259 7 segment, 8 digit LED display panel test program");
  Serial.println("Connections:");
  Serial.println("  Data (DIN): 4");
  Serial.println("  Chip Select (CS): 5");
  Serial.println("  Clock (CLK): 6");


  ledDisplay.shutdown(0, false);
  ledDisplay.setIntensity(0, 8);

  displayOneToEight();

  delay(1000);
  outputInteger(123);
  delay(1000);
  outputInteger(-456);
  delay(1000);
}

void loop() {
  unsigned long _now = millis();
  if (_now - counterLastUpdateTime >= COUNTER_UPDATE_INTERVAL) {
    outputInteger(counter++);
    counterLastUpdateTime = _now;
  }

}

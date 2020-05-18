#include "StatusLed.h"

StatusLed led2;
StatusLed led3 (11, 12, false);     // Third parameter is 3 pin LED (true) or 2 pin (false)

// Blinking green 3/4
int seq1Steps [] = { StatusLed::A, StatusLed::OFF };
int seq1Times [] = { 750, 250 };

// Long red, short red.
int seq2Steps [] = { StatusLed::B, StatusLed::OFF, StatusLed::B, StatusLed::OFF };
int seq2Times [] = { 1000, 150, 100, 150 };

// Fast red flash.
int seq3Steps [] = { StatusLed::B, StatusLed::OFF };
int seq3Times [] = { 100, 100 };

// Long green, short red.
int seq4Steps [] = { StatusLed::A, StatusLed::OFF, StatusLed::B, StatusLed::OFF };
int seq4Times [] = { 800, 200, 200, 200 };

// Traffic light
int seq5Steps [] = { StatusLed::A, StatusLed::BOTH, StatusLed::B };
int seq5Times [] = { 2000, 750, 1500 };

// Blinking red 10/11 and off 1/11 (duty cycle 10:1)
int seq6Steps [] = { StatusLed::B, StatusLed::OFF };
int seq6Times [] = { 1000, 100 };


void showLedConfig(StatusLed *led) {
  Serial.print(F("Pin A: ")); Serial.print(led->getA());
  Serial.print(F(" Pin B: ")); Serial.print(led->getB());
  Serial.print(F(" 3 pin mode: ")); Serial.println(led->isThreePinMode());
  
}

void setup() {

  Serial.begin(38400);

  while (!Serial) {
    delay(1);
  }

  Serial.println();
  Serial.print("LED2: "); showLedConfig(&led2);
  Serial.print("LED3: "); showLedConfig(&led3);
  
  led2.clearSequence();
  led3.clearSequence();

  
  Serial.println("Ready");
}


int duration = 1000;

void showDuration(int duration) {
  Serial.print(F("duration = ")); Serial.println(duration);
}

void showCmd(int cmd, int duration) {
  Serial.print(F("Step: ")); Serial.print(led2.getStepCount()); Serial.print(F("  "));
  if (cmd == StatusLed::OFF) {
    Serial.print(F("led.off("));
  } else {
    Serial.print(F("led.on("));
    switch (cmd) {
      case StatusLed::A:
        Serial.print(F("led.A"));
        break;
      case StatusLed::B:
        Serial.print(F("led.B"));
        break;
      case StatusLed::BOTH:
        Serial.print(F("led.BOTH"));
        break;
      default:
        Serial.print(F("led.UNEXPECTED:"));
        Serial.print(cmd);
        break;
    }
    Serial.print(F(", "));
  }
  Serial.print(duration);
  Serial.println(F(");"));
}


// Some interesting command sequences:
// zd2b0c2b0  - long red / short off / short red / short off
// zb12 - red / green
// za12 - short red / short green
//
void loop() {
  int chRead;
  int x;
  
  led2.timerEvent();
  led3.timerEvent();

  if ( (chRead = Serial.read()) != -1) {
    if (chRead > ' ') {
      switch (chRead) {
        case 'A':
        case 'a':
          duration = 100;
          showDuration(duration);
          break;
        case 'B':
        case 'b':
          duration = 250;
          showDuration(duration);
          break;
        case 'C':
        case 'c':
          duration = 500;
          showDuration(duration);
          break;
        case 'D':
        case 'd':
          duration = 1000;
          showDuration(duration);
          break;

 
        case 'X':
        case 'x':
          x = analogRead(A0);
          Serial.print(F("A0: ")); Serial.println(x);
          break;

        case 'Z':
        case 'z':
          led2.clearSequence();
          led3.clearSequence();
          Serial.println(F("Clear Steps"));
          break;

        case '!':       // Shift 1.
          led2.loadSequence(seq1Steps, seq1Times, sizeof (seq1Steps) / sizeof(seq1Steps[0]));
          led3.loadSequence(seq1Steps, seq1Times, sizeof (seq1Steps) / sizeof(seq1Steps[0]));
          break;
          
        case '@':       // Shift 2.
          led2.loadSequence(seq2Steps, seq2Times, sizeof (seq2Steps) / sizeof(seq2Steps[0]));
          led3.loadSequence(seq2Steps, seq2Times, sizeof (seq2Steps) / sizeof(seq2Steps[0]));
          break;
 
        case '#':       // Shift 3.
          led2.loadSequence(seq3Steps, seq3Times, sizeof (seq3Steps) / sizeof(seq3Steps[0]));
          led3.loadSequence(seq3Steps, seq3Times, sizeof (seq3Steps) / sizeof(seq3Steps[0]));
          break;
 
        case '$':       // Shift 4.
          led2.loadSequence(seq4Steps, seq4Times, sizeof (seq4Steps) / sizeof(seq4Steps[0]));
          led3.loadSequence(seq4Steps, seq4Times, sizeof (seq4Steps) / sizeof(seq4Steps[0]));
          break;
 
        case '%':       // Shift 5.
          led2.loadSequence(seq5Steps, seq5Times, sizeof (seq5Steps) / sizeof(seq5Steps[0]));
          led3.loadSequence(seq5Steps, seq5Times, sizeof (seq5Steps) / sizeof(seq5Steps[0]));
          break;
          
        case '^':       // Shift 6.
          led2.loadSequence(seq6Steps, seq6Times, sizeof (seq6Steps) / sizeof(seq6Steps[0]));
          led3.loadSequence(seq6Steps, seq6Times, sizeof (seq6Steps) / sizeof(seq6Steps[0]));
          break;

        case '0':
          led2.off(duration);
          led3.off(duration);
          showCmd(StatusLed::OFF, duration);
          break;
        case '1':
          led2.on(StatusLed::A, duration);
          led3.on(StatusLed::A, duration);
          showCmd(StatusLed::A, duration);
          break;
        case '2':
          led2.on(StatusLed::B, duration);
          led3.on(StatusLed::B, duration);
          showCmd(StatusLed::B, duration);
          break;
        case '3':
          led2.on(StatusLed::BOTH, duration);
          led3.on(StatusLed::BOTH, duration);
          showCmd(StatusLed::BOTH, duration);
          break;
        default:
          Serial.print(F("??? -> ")); Serial.print((char) chRead); Serial.print(F(" (")); Serial.print(chRead); Serial.println(F(")"));
          break;
      }
    }
  }
}


class StatusLed {
  public:

    StatusLed(const int a = 7, const int b = 8) {
      _a = a;
      _b = b;
      pinMode(_a, OUTPUT);
      pinMode(_b, OUTPUT);
    }

    int getA() {
      return _a;
    }

    int getB() {
      return _b;
    }

    void clearSequence() {
      _numSteps = 0;
      _off();
    }
 
    void off(int duration) {
      return _addStep(OFF, duration);
    }

    void on(int colour) {
      _colour = colour;
      timerEvent();
    }

    void timerEvent() {
      switch (_colour) {
        case OFF:
          _off();
          break;

        case A:
          _aOn();
          break;
          
        case B:
          _bOn();
          break;
          
        case BOTH:
          if (_curr == A) {
            _bOn();
            _currActiveLed = B;
          } else {
            _aOn();
            _currActiveLed = A;
          }
      }
    }

    static const int A = 0;
    static const int B = 1;
    static const int BOTH = 2;
    static const int OFF = -1;

  private:
    int _a;                 // Pin that one leg of the BiColour LED is connected to.
    int _b;                 // Pin that the other leg og the LED is connected to.
    int _currActiveLed;     // Only used when alternating between BOTH LED's
    int _colourIndex;       // The current index in the sequence of steps.
    int _numSteps;          // The number of steps in the sequence.
    int _colourSeq[MAX_STEPS];
    int _colourTime[MAX_STEPS];

    void _off() {
      digitalWrite(_a, LOW);
      digitalWrite(_b, LOW);    
    }

    void _aOn() {
      digitalWrite(_a, LOW);
      digitalWrite(_b, HIGH);    
    }

    void _bOn() {
      digitalWrite(_a, HIGH);
      digitalWrite(_b, LOW);    
    }

};

StatusLed led;

void setup() {

  Serial.begin(9600);

  while (!Serial) {
    delay(1);
  }

  led.off();

  Serial.print("LED Pin A: "); Serial.print(led.getA());
  Serial.print(" Pin B: "); Serial.println(led.getB());
  Serial.print("A = "); Serial.println(led.A);
  Serial.print("B = "); Serial.println(led.B);
  Serial.print("Both = "); Serial.println(led.BOTH);
  
  Serial.println("Ready");
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Colour 1");
  led.on(led.A);
  delay(500);
  Serial.println("Off");
  led.off();
  delay(500);
  Serial.println("Colour 2");
  led.on(led.B);
  delay(500);
  Serial.println("Off");
  led.off();
  delay(250);

  Serial.println("Both");
  led.on(led.BOTH);
  for (int i = 0; i< 500; i++) {
    led.timerEvent();
    delay(1);
  }
  led.off();
  delay(250);
}

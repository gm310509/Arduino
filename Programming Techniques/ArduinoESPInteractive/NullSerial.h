/*
 * NullSerial
 * ----------
 * 
 * An empty implementation of a Serial Interface.
 * 
 * The idea for this NullSerial class is to, via conditional compilation,
 * eliminate any Debug messages from released code.
 * 
 */

 #ifndef NullSerial_h
 #define NullSerial_h

class NullSerial {
  public:
    NullSerial(uint16_t receivePin, uint16_t transmitPin, bool inverse_logic = false) {}
    NullSerial() {}
    void begin (long speed) {}
    bool listen() { return false; }
    void end() {}
    bool isListening() { return false; }
    bool stopListening() { return false; }
    bool overflow() { return false; }
    int peek() { return -1; }

    void println() {}
    void print() {}
    void println(const void * msg) {}
    void print(const void * msg) {}
    void println(const int i) {}
    void print(const int i) {}
};

#endif   //NullSerial_h

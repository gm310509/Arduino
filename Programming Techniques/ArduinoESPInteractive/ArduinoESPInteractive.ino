/***********************************************
 * MegaESPInteractive
 * ------------------
 * 
 * This program is designed for use with an Arduino Mega + ESP 8266 combo board.
 * It allows you to: 
 * - interact with the ESP 8266 via the Arudino
 *   and
 * - upload programs to the Arduino
 * without constantly switching the DIP switches to connect the Arduino to the ESP
 * and/or the USB.
 * 
 * If you want to upload a program to the ESP, you will still need to set the DIP
 * switches.
 * 
 * The ability to have a development cycle that does not involve switching DIP
 * switches expedites the program/debug cycle of WiFi based apps because the very
 * tedious and fiddly step of constantly having to change them is eliminated. This
 * will also probably extend the life of the board as you don't have as much
 * mechanical wear and tear on them.
 * 
 * Hardware required:
 * - Arduino Mega + ESP 8266 wifi combo board such as the Jaycar XC4421: 
 *      https://www.jaycar.com.au/mega-with-wi-fi/p/XC4421.
 * - optionally an FTDI USB - Serial board such as the Jaycar XC4672:
 *      https://www.jaycar.com.au/isp-programmer-for-arduino-and-avr/p/XC4627)
 *
 * The development cycle works as follows:
 * 1) Develop your code and upload using the standard Arduino IDE upload mechanism.
 * 2) use Serial3 (via a preprocessor #define constant) for interaction with the
 *    ESP.
 * 2) Optionally interact with your Arduino program via Serial2 or Serial (also via
 *    a preprocessor constant)
 * 3) Output Debug messages to Serial (via a preprocessor constant).
 * 
 * Preprocessor symbols are used to represent the two Serial ports used as follows:
 *   ESP -> Serial3
 *   HOST-> Serial or Serial2
 *
 * Once your program is complete and can operate standalone, you can transfer to a
 * smaller device (e.g. the Arduino UNO + Wifi board). Simply redefine the constants
 * so that ESP is Serial and HOST is an instance of softwareSerial.
 * 
 * To use this program, two sets of switches on the Arduino Mega+ESP8266 Wifi must
 * be set as follows:
 * 
 * The large DPDT switch with labels RXD0 & TXD0 at one end and RXD3 & TXD3 must be
 * set to the RXD3 & TXD3 end. This enables the Mega Serial3 port for communications
 * with the ESP.
 *
 * The 8 position DIP switch must be set as follows:
 * 
 *    on      o  o  o  o
 *    off                 o  o  o  o
 * Number     1  2  3  4  5  6  7  8
 *
 * That is, switches 1, 2, 3 & 4 are be turned on and
 * switches 5, 6, 7 & 8 are turned off.
 * These switch configurations:
 * - Connect the Mega Serial3 to the ESP (for WiFi access)
 * - Connect the Mega Serial to the USB (for programming and debugging).
 
 */
#include "NullSerial.h"

#define VERSION "1.1.0.1"
/*
 * Revisions.
 * 1.1.0.1
 *    Corrected bug for LED on/off debug messages.
 *
 * 1.1.0.0
 *    Added concept of debug messages.
 *    Added initialisation of WiFi to station mode and connect to WiFi boiler
 *    plate code.
 * 
 */

#if defined(ARDUINO_AVR_MEGA2560)
  #define HOST_BAUD 115200
  #define HOST_RX  "USB(0)"
  #define HOST_TX  "USB(1)"
  #define HOST  Serial
  #define DEBUG Serial
  
//  NullSerial _debug(1, 2);
//  #define DEBUG _debug

  #define ESP_BAUD  115200
  #define ESP_RX  15
  #define ESP_TX  14
  #define ESP   Serial3

#else if defined(ARDUINO_AVR_UNO)
/*
 * On Uno, we only have one hardware serial device.
 * So, we will use SoftwareSerial for any interactions that may be required with
 * the host.
 */
  #include <SoftwareSerial.h>
  #define HOST_BAUD 115200
  #define HOST_RX  10
  #define HOST_TX  11
  SoftwareSerial _host(HOST_TX, HOST_RX);     /* (my_RX, my_TX) */
  #define HOST  _host
  
  NullSerial _debug(HOST_TX, HOST_RX);
  #define DEBUG _debug
//  #define DEBUG _host

  #define ESP_BAUD  115200
  #define ESP_RX  "USB(0)"
  #define ESP_TX  "USB(1)"
  #define ESP   Serial

#endif

/* A pair of macros that allows the value of symbols defined in #define
 * preprocessor directives (e.g. HOST) to be output as strings (e.g. in
 * print function calls).
 */
#define _STRING(x) (#x)
#define STRING(x) _STRING(x)



/*****************************
 * The hasEOLJustBeenSent is used to track whether an end of line has been sent to
 * the ESP.
 * The version of code shipped with the device requires a Carriage Return (CR)
 * followed by a Line Feed (LF) - in that order - to mark the end of a line (EOL).
 * That is, the ESP requires a CRLF end of line (EOL) sequence.
 * 
 * This program allows you considerable flexibility of windows terminal programs
 * (I use Putty, CoolTerm, the Arduino Serial monitor and others) in their default
 * configurations. These programs all generate various combinations of the common
 * line endings CR only, LF only and CRLF.
 * When it sees a CR ('\r') or LF ('\n'), the program will send an EOL to the ESP.
 * To allow for terminal programs that send both a CR and a LF, the
 * hasEOLJustBeenSent is used to track that an EOL has been sent when either the CR
 *  or LF is received from the HOST.
 * To prevent doubling up on EOL's, if we have just received a CR or LF, and the
 * next character is also a CR or LF, then it will simply be ignored.
 */
boolean hasEOLJustBeenSent = false;
char buf[100];
int bufPtr = 0;

boolean OKseen = false;
boolean ERRORseen = false;
boolean FAILseen = false;
boolean TIMEOUTseen = false;


/*
 * processInput(msg)
 * -----------------
 * 
 * Processes a message received from the ESP.
 * This includes processing responses to commands (e.g. OK, FAIL etc)
 * Processing input from a client.
 * Processing client connection/disconection messages.
 * And others as required.
 * 
 */
void processInput (const char * msg) {
  DEBUG.println();
//  DEBUG.print(F("Received: "));
//  DEBUG.println(msg);

  OKseen = false;
  ERRORseen = false;
  FAILseen = false;

  if (strncmp(msg, "+IPD", 4) == 0) {   // Client message?
    DEBUG.println(F("Received input from client"));
    char led = msg[9];                  // Extract the LED number
    char setting = msg[10];             // Extract the setting (on/off)
    if (led >= '1' && led <= '3') {     // Check the LED number for validity (i.e. 1, 2 or 3)
      led = led - '1';                  // if valid, convert to an integer.
    } else {                            // Otherwise print an error message to the USB.
      HOST.print(F("Invalid LED: ")); HOST.println(led);
      return;
    }

    DEBUG.print(F("Setting LED ")); DEBUG.print(led);
      DEBUG.print(F(" on DIO ")); DEBUG.print(led + 5);
      DEBUG.println(setting == '+' ? " on" : " off");
                                              // FInally, turn the requested LED on or off
    digitalWrite(led + 5, setting == '+');    // depending upon the value in "setting".
                                              // The led + 5 adjusts the LED value (0, 1 or 2)
                                              // to the corresponding Arduino digital I/O pins
                                              // which that have LED's connected whihc are pins 5, 6 & 7.
  }
  OKseen = strcmp(msg, "OK") == 0;
  ERRORseen = strcmp(msg, "ERROR") == 0;
  FAILseen = strcmp(msg, "FAIL") == 0;
}



/*  accumulateESPData
 *  =================
 *  
 *  Check if a character is available on the ESP Serial port.
 *  If it is, accumultate the character in a buffer.
 *  When a newline is observed, process the input.
 *
 *  Returns true when a newLine has been seen and the input processed.
 *          false otherwise
 */
boolean accumulateESPData() {
  boolean newLineSeen = false;

  if (ESP.available()) {
    char ch = ESP.read();
    HOST.write(ch);
    if (ch == '\r' || ch == '\n') {
      buf[bufPtr] = '\0';           // Null terminate the input;
      if (bufPtr > 0) {
        processInput(buf);
        newLineSeen = true;
      }
      bufPtr = 0;
    } else {
      if (bufPtr < sizeof(buf) - 1) {   // Append the character to
        buf[bufPtr++] = ch;         // the input buffer if space permits.
      }
    }
  }
  return newLineSeen;
}


/**
 * sendESP(msg)
 * ============
 * Send the supplied msg to the ESP along with a CRLF line ending.
 * Wait for a response of OK, ERROR, FAIL or a TIMEOUT.
 * The timeout period is specified by the toPeriod constant value.
 */
const unsigned long ESP_TO_PERIOD = 30000;    // millis to wait for reply (30 secs).
void sendESP(const char *msg) {
  ESP.print(msg);
  ESP.print("\r\n");
  OKseen = false;
  ERRORseen = false;
  FAILseen = false;
  TIMEOUTseen = false;
      // Establish the initial timout period.
  unsigned long timeout = millis() + ESP_TO_PERIOD;
      // Loop until we get one of the expected replies
      // or a timeout occurs (without getting one of the expected replies)
  while (!OKseen && !ERRORseen && !FAILseen && ! TIMEOUTseen) {
    if (accumulateESPData()) {
            // A response has been recieved.
            // Debug output the status of the expected replies
      DEBUG.print(F("O,E,F="));
        DEBUG.print(OKseen); DEBUG.print(ERRORseen);
        DEBUG.println(FAILseen);
      timeout = millis() + ESP_TO_PERIOD;    // message rcvd, reset the timer.
    }
    if (millis() >= timeout) {
      TIMEOUTseen = true;
      DEBUG.println(F("*** Timeout waiting for ESP reply"));
    }
  }
}


/**
 * AccumulateHOSTData
 * ==================
 * 
 * Check for data being available from the host. If there is data, simply pass it on
 * to the ESP.
 * If a CR or a LF is observed in the HOST data, send the ESP a CRLF.
 */
void accumulateHOSTData() {
  if (HOST.available()) {
    char ch = HOST.read();
//    HOST.write(ch);                   // Perform local echo to HOST (or not)
    if (ch == '\n' || ch == '\r') {   // Do we have a LF or CR?
      if (!hasEOLJustBeenSent) {      // Yep, did we just send one?
        ESP.write('\r');              // Nope, so send a CRLF to the ESP.
        ESP.write('\n');
        hasEOLJustBeenSent = true;    // Track that we've sent an EOL
      }
    } else {                          // Not a CR & not a LF, so
      ESP.write(ch);                  // Write the character to the ESP
      hasEOLJustBeenSent = false;     // Since not a CR & not a LF, track that
                                      // we didn't just send an EOL to ESP.
    }
  }
}


/**
 * setup
 * -----
 *
 * Initialise our Serial devices, output some configuration information
 * and set the BUILTIN_LED for output.
 */
void setup() {
  ESP.begin(ESP_BAUD);
  HOST.begin(HOST_BAUD);
  while (!HOST) {
    delay(1);
  }
  while(!ESP) {
    delay(1);
  }

  HOST.println(F("\x1b[2JESP WiFI Test Program"));

  HOST.print(F("Version: ")); HOST.println(F(VERSION));
  HOST.print(F("ESP  Ready on: ")); HOST.print(STRING(ESP));
    HOST.print(F(" @ ")); HOST.print(ESP_BAUD);
    HOST.print(F(" bps. RX, TX: "));
    HOST.print(ESP_RX); HOST.print(F(", ")); HOST.println(ESP_TX);
    
  HOST.print(F("HOST Ready on: ")); HOST.print(STRING(HOST));
    HOST.print(F(" @ ")); HOST.print(HOST_BAUD);
    HOST.print(F(" bps. RX, TX: "));
    HOST.print(HOST_RX); HOST.print(F(", ")); HOST.println(HOST_TX);

  DEBUG.println(F("**** Debug messages enabled ****"));
  HOST.println();

  pinMode(5, OUTPUT);             // Set some pins to output for LEDs.
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  
  DEBUG.println(F("Delaying in hopes the ESP will wake up."));
  delay(1000);
  DEBUG.println(F("Sending GetVersion"));
  sendESP("AT+GMR");              // Get Version Info.
  
    // Once off set mode and join the WiFi.
  DEBUG.println(F("Setting the mode"));
  sendESP("AT+CWMODE=1");         // Set operating mode to "station"
  
  DEBUG.println(F("Connect to my WiFi"));
//  sendESP("AT+CWJAP=\"YourWiFi\",\"yourpassword\"");
  sendESP("AT+CWJAP=\"XYZZY\",\"SyZyGy6969\"");

  delay(200);
//  DEBUG.println(F("Sending Max Connections=1"));
//  sendESP("AT+CIPMUX=1");         // Enable Server
//  DEBUG.println(F("Sending Start Server on Port 80"));
//  sendESP("AT+CIPSERVER=1,80");   // Open port 80

  HOST.println(F("Ready"));
  HOST.println();
}



/* Loop
 * ====
 * 
 * Accumulate characters from the HOST and ESP.
 * Process them as necessary.
 */
void loop() {
  accumulateHOSTData();
  accumulateESPData();
}

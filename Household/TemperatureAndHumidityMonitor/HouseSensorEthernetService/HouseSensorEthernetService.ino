/*
 * HomeSensor - Ethernet Web Server
 * --------------------------------
 * 
 * A project that monitors environmental conditions in our home.
 * Readings are extracted periodically via a REST API call.
 * 
 * Intended to be used with Arduino - EtherTen, but can also be used
 * with any Arduino and an Ethernet sheild.
 * 
 * Circuit:
 *  EtherTen (or Ethernet shield attached to pins 10, 11, 12, 13)
 *  DHT Temperature and Humidity sensor Pin 2.
 *  Optional LED, connected to PIN 3.
 *
 */


/*******************************************************************
 * IMPORTANT              IMPORTANT               IMPORTANT
 *
 *   Update the following NW_OFFSET to properly allocate a Mac address
 *   and an IP address.
 *   The first device should be 0, the next 1, the one after that 2
 *   and so on.
 *   Also, make sure your network (IP) address range is available on
 *   your network.
 *******************************************************************/
// A value added to the MAC and TCP/IP address to ensure uniqueness
// on the network.
#define NW_OFFSET       0
/*******************************************************************
 * IMPORTANT                                      IMPORTANT
 *
 *   Update the above NW_OFFSET to properly allocate a Mac address
 *   and an IP address.
 *******************************************************************/

#define DEBUG
//#define STARTUP_DEBUG
#ifdef DEBUG
  #define STARTUP_DEBUG
#endif

#define LED_ACTIVITY    3
#define SD_CARD         4


// See guide for details on sensor wiring and usage (DHT22):
//   https://learn.adafruit.com/dht/overview

#define DHTTYPE   DHT22     // DHT 22 (AM2302)
#define DHTPIN    2         // Pin which is connected to the DHT sensor.


#include <DHT.h>
#include <DHT_U.h>

//#include <SPI.h>
#include <Ethernet.h>


// Tracks the last recorded millisecond value.
// Used to track when a millisecond (or more) has passed.
unsigned long lastMillis;

#define SERVER_PORT 4000
/*******************************************************************
 * IMPORTANT              IMPORTANT               IMPORTANT
 * 
 * Enter a MAC address and IP address for your controller below.
 * The IP address will be dependent on your local network.
 * Ensure that the addresses you select are:
 * a) In your network as defined by the subnet mask and
 * b) Not used by another device.
 * 
 * For simplicity, try to identify a contiguous block of addresses
 * of sufficient size to have 1 address for each Arduino that you
 * wish to connect.
 */
byte mac[] = {    0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xA0 + NW_OFFSET   };
IPAddress ip(192, 168, 1, 160 + NW_OFFSET);
/*******************************************************************
 * IMPORTANT                                      IMPORTANT
 *
 *   Update the above IP Address to properly reflect your Local Area
 *   Network (LAN) address range.
 *******************************************************************/

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(SERVER_PORT);

String httpRequestText;         // string used to capture the HTTP Request Text.

DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMsDHTSensor;


/************************************************
 * Class TimedTask.
 * 
 * An abstract (incomplete) class that manages the scheduling of sub tasks.
 * 
 * This class tracks the elapsed amount of time on behalf of it's subclasses.
 * When the alotted time has passed, the "execute" method will be invoked to
 * allow the task to do whatever it needs to do.
 * NB: This TimedTask class would ideally be "factorised" out into a library.
 *     If we did this, then the entire functionality could be accessed by a single
 *     line of code (as opposed to the 70 odd lines here. The single line of code
 *     would be something like: #include <TimedTask.h>.
 */
class TimedTask {
  public:
    // Constructor - capture the time that has to pass until the task needs to be invoked.
    TimedTask(unsigned long nextEventTime) {
      this->nextEventTime = nextEventTime;
      if (nextEventTime == 0) {             // If we construct with a nextEventTime of zero,
        enabled = false;                    // disable the task for now.
      }
    }

    // Set the next event time.
    void setNextEventTime(unsigned long nextEventTime) {
      this->nextEventTime = nextEventTime;
    }

    // Abstract methods which must be implemented (defined) in a subclass.
    virtual unsigned long execute();        // Execute the task.
    virtual void outputTaskName();
    virtual void disableTask();             // Invoked when this task is being disabled.
    virtual void enableTask();              // Invoked when this task is being enabled.

    // Enable this task.
    void enable() {
      enabled = true;
      timeSinceLastEvent = 0;               // Reset the elapsed time counter.
      enableTask();                         // Notify the subclass that the task has been enabled.
    }

    // disable this task.
    void disable() {
      enabled = false;
      disableTask();                        // Notify the subclass that the task has been disabled.
    }


    // Return the enabled/disabled state of the task.
    boolean isEnabled() {
      return enabled;
    }

    // Record the fact that time has passed.
    void recordTime(unsigned long delta) {
      timeSinceLastEvent += delta;          // Record the time and check if this task is due to be
                                            // executed. NB: the task is only executed if it is enabled.
      if (timeSinceLastEvent >= nextEventTime && enabled) {
#ifdef DEBUG
//        Serial.print ("Executing task: ");
//        outputTaskName();
#endif
        unsigned long nev = execute();      // Notify the subclass to do it's thing.
        if (nev > 0) {                      // Record the next event time if it is non zero.
          nextEventTime = nev;
        }
        timeSinceLastEvent = 0;             // Reset the time counter.
      }
    }

  private:
    unsigned long nextEventTime;            // Time that must pass before we invoke the subtask.
    unsigned long timeSinceLastEvent = 0;   // The time has passed since the last invocation of the subtask.
    boolean enabled = true;
};



class ActivityLED : public TimedTask {
  public:
    ActivityLED(int ledPin)
      : TimedTask(HeartbeatLedOffTime) {
        this->ledPin = ledPin;            // remember the pin that the LED is connected to.
        pinMode(ledPin, OUTPUT);          // Set the pin as output.
        digitalWrite(ledPin, HIGH);       // turn the LED off.
        ledOnInd = false;
        mode = HeartbeatMode;             // set the LED to Heartbeat mode.
      }

    // Time to update the status of the LED.
    unsigned long execute() {
          // It's time to invert the LED status (i.e. On -> off and Off -> On).
      ledOnInd = !ledOnInd;                     // Invert the LED>
      digitalWrite(ledPin, ledOnInd ? LOW : HIGH);


      // If we are in Heartbeat mode (briefly flash the LED on once
      // every two seconds or so.
      if (mode == HeartbeatMode) {
        return ledOnInd ? HeartbeatLedOnTime : HeartbeatLedOffTime;
      }

      // Otherwise we are either in Response mode or Ident mode.
      // In these cases, Activate the LED in a pattern that includes
      // multiple repeats (multiple repeats can = 1).
      // If the LED is now on, count one of the repeats.
      // Otherwise the LED is now off, so
      //     check if we need to continue the pattern
      //     or revert to the heartbeat pattern.                          
      if (ledOnInd) {                     // Led is on. Subtract one from the cycle
        cycleCntr--;                      // counter. Each time the LED is turned on.
      } else if (cycleCntr == 0) {        // LED is off and we've reached the end of the cycleCntr
        mode = HeartbeatMode;             // so, revert to Heartbeat mode (which is intercepted above).
        return HeartbeatLedOffTime;       // and return the Heatbeat led off time.
      }
      return cycleTime;                   // Otherwise just return the base delay time as defined by the setmode function.
    }

      // Not used, but in case it is, just turn the LED off.
    void disableTask() {
      digitalWrite(ledPin, HIGH);        // turn the LED off.
      ledOnInd = false;
    }

    void enableTask() {
                                          // Nothing special to do.
    }

    void outputTaskName() {
      Serial.print ("Activity LED. Mode: ");
      Serial.println(mode);
    }

    // Constants for the modes.
    static const int TransmittingLedOnTime = 750;     // When transmiting - on for 750 ms.
    static const int IdentLedTime = 150;              // In IDENT mode, blink for 150 on, 150ms off.
    static const int DefaultLedTime = 500;            // if an unknown mode is activated, blink for 500ms on/off (should never happen)
    static const int HeartbeatLedOnTime = 200;        // In heartbeat mode, LED is on for 200ms and
    static const int HeartbeatLedOffTime = 2300;      //                    LED is off for 2300ms.
    
    static const int TransmittingDataMode = 0;        // Constant that identifies transmitting mode.
    static const int IdentMode = 1;                   // Constant that identifies ident mode.
    static const int HeartbeatMode = 2;               // Constant that identifies heartbeat (idle) mode.
    
    void setMode(int mode) {
      this->mode = mode;
      switch(mode) {
        case TransmittingDataMode:
          cycleCntr = 1;                              // Transmitting data is one cycle of LED ON
          cycleTime = TransmittingLedOnTime;
          break;
        case IdentMode:
          cycleCntr = 20;                             // Ident mode is 20 cycles of 150 on, 150ms off (total 3 seconds)
          cycleTime = IdentLedTime;
          break;
        case HeartbeatMode:
          break;                                      // No need to do anything special for heartbeat mode. This is
                                                      // handled entirely in the execute method.
        default:
          cycleCntr = 3;                              // Unknown mode is 3 cycles of 500ms on / off.
          cycleTime = DefaultLedTime;
          break;
      }
      execute();
    }


  private:
    int ledPin;                           // the led pin.
    int mode;                             // the current mode of operation.
    int cycleCntr;                        // the number of blink cycles.
    unsigned long cycleTime;              // the time to remain on and off.
    boolean ledOnInd = false;
    
};

// Define the Activity LED task.
ActivityLED activityLed(LED_ACTIVITY);

void setup() {

  // Open serial communications and wait for port to open (but not too long).
  Serial.begin(9600);
  int cnt = 0;
  while (!Serial && cnt < 100) {
    cnt++; // wait for serial port to connect. Needed for native USB port only
    delay(1);
  }
#ifdef STARTUP_DEBUG
  Serial.println("HouseSensorEthernetServer");
    Serial.print("LED: "); Serial.println(LED_ACTIVITY);
#endif

  delay(50);    // allow some time (50 ms) after powerup and sketch start,
                 // for the Wiznet W5100 Reset IC to release and come out of reset.

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    pinMode(LED_ACTIVITY, OUTPUT);
    while (true) {        // do nothing, no point running without Ethernet hardware
      digitalWrite(LED_ACTIVITY, LOW);
      delay(200);         // Fast blink the LED as an error indication in case Serial not enabled
      digitalWrite(LED_ACTIVITY, HIGH);
      delay(200);
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  // start the web server
  server.begin();

#ifdef STARTUP_DEBUG
  Serial.print("server is at ");
  Serial.print(Ethernet.localIP());
  Serial.print(":");
  Serial.println(SERVER_PORT);
  Serial.print("MAC addr: ");
  for (int i = 0; i < sizeof(mac); i++) {
    Serial.print(mac[i], HEX);
    if (i < sizeof(mac) - 1) {
      Serial.print(", ");
    } else {
      Serial.println();
    }
  }
  Serial.println("Initialising DHT...");
#endif

  // disable the SD card by switching pin 4 high
  // not using the SD card in this program, but if an SD card is left in the socket,
  // it may cause a problem with accessing the Ethernet chip, unless disabled
#ifdef STARTUP_DEBUG
  Serial.println("Disabling SD Card");
#endif
  pinMode(SD_CARD, OUTPUT);
  digitalWrite(SD_CARD, HIGH);

#ifdef STARTUP_DEBUG
  Serial.println("Initialising the DHT sensor");
#endif
  dht.begin();

  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  // Set delay between sensor readings based on sensor details.
  delayMsDHTSensor = sensor.min_delay / 1000;

#ifdef DEBUG
  Serial.println(" Done");
#endif
  
#ifdef DEBUG
  Serial.println("------------------------------------");
  Serial.println("Temperature");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" *C");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" *C");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" *C");  
  Serial.println("------------------------------------");
  delay(delayMsDHTSensor);
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Humidity");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println("%");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println("%");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println("%");  
  Serial.println("------------------------------------");
#endif

  lastMillis = millis();      // Initialise the "timer".
}


void loop() {
  // listen for incoming clients
  
  EthernetClient client = server.available();
  if (client) {
#ifdef DEBUG
    Serial.println("new client");
#endif
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
        // while the client is connected
    while (client.connected()) {
        // Check to see if some data is available.
      if (client.available()) {
        // If so, read the next character and assemble it into a string (the request)
        char c = client.read();
        httpRequestText += c;     // Capture the request one character at a time.
#ifdef DEBUG
        Serial.write(c);          // Display the character to the monitor if debugging.
#endif
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // Set the activity LED to "response mode".
          activityLed.setMode(ActivityLED::TransmittingDataMode);

          // Process the request and obtain the response.
          String msg = processRequest();

          // Send the reply message as is directly back to the requesting client.
          client.println(msg);
          httpRequestText = "";   // Clear the request text once done with the client.
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
#ifdef DEBUG
    Serial.println("client disconnected");
#endif
  }

// Check to see if a millisecond has passed.
// If it has, tick all of the timer handlers.
  unsigned long currTime = millis();
  if (lastMillis != currTime) {
    unsigned long deltaTime = currTime - lastMillis;
    lastMillis = currTime;
    activityLed.recordTime(deltaTime);
  }
}


/**
 * The process request method determines what request has been made
 * and executes it.
 * The possible actions include:
 * - Identify the device (return the MAC address and rapidly blink the LED)
 * - Read the sensor (return the sensor readings)
 * - Request help (return the help message)
 * - Unrecognised request (return a message telling the client how to request help).
 */
String processRequest() {
  String msg = "";
  // Is the request an "ident" request?
  if (httpRequestText.indexOf("ident ") > -1) {
    // formulate a response containing the MAC address.
    msg = "mac:";
    for (int i = 0; i < sizeof(mac); i++) {
      msg += " 0x";
      msg += String (mac[i], HEX);
    }
    // Blink the LED in IDENT mode.
    activityLed.setMode(ActivityLED::IdentMode);

    // Otherwise is the request for "help"?
  } else if (httpRequestText.indexOf("help") > -1) {
    // Formulate a "help" response.
    msg = "use: \n /help for help\n /ident to identify";
    // Otherwise is the request the "main request" - i.e. no modifier on the URL.
  } else if (httpRequestText.indexOf(" / HTTP") > -1) {
    // Return the sensor reading.
    msg = getSensorMsg();
    // Otherwise, the request is unrecognised, so return advice for help.
  } else {
    msg = "Invalid Request try /help";
  }
  return msg;
}


// for sensors_event_t field definitions:
//   https://github.com/adafruit/Adafruit_Sensor/blob/master/Adafruit_Sensor.h
// for DHT methods:
//   https://github.com/adafruit/DHT-sensor-library/blob/master/DHT.h
// for DHT_Unified methods:
//   https://github.com/adafruit/DHT-sensor-library/blob/master/DHT_U.h
/*
 * Read the sensor and formulate the message as follows:
 * - Reading Type e.g. T for temperature, H for Humidity or any other value you care to define
 *   for other sensors (e.g. L for LIGHT reading, P for pressure and so on).
 * - ID of the sensor (0 for the first one, 1, for the second and so on).
 * - The value for the reading (or 9999.9 for an invalid reading e.g. device unavailable).
 * 
 * - Each reading is seperated by a semi-colon. The reading type & sensor ID (2 characters) and value are separated by a comma.
 * For example,
 * T0,12.5;P0,1013.0;H0,87;T1,18.9;P1,9999.9;H1,50.2;
 * would be interpreted as follows:
 * - For sensor group 0:
 *   - Temperature (T0) is 12.5 degrees C
 *   - Humidity (H0) is 87%
 *   - Pressure (P0) is 1013.0 millibar
 * - For sensor group 1:
 *   - Temperature (T1) is 18.9 degrees C
 *   - Humidity (H1) is 50.2%
 *   - Pressure (P1) is unavailable (9999.9)
 *   
 * The following routine will return Temperature and Humidity values for a single
 * sensor group (T0 and H0).
 * 
 * The String returned from this function is returned to the client as is.
 */
String getSensorMsg() {
  String msg = "";
  String msg1 = "";

    sensors_event_t event;
      
    dht.temperature().getEvent(&event);
    msg = "T0,";
    msg += (isnan(event.temperature) ? 9999.9 : event.temperature);
    msg += ";";
    
    dht.humidity().getEvent(&event);
    msg1 = "H0,";
    msg1 += (isnan(event.relative_humidity) ? 9999.9 : event.relative_humidity);
    msg1 += ";";
    msg += msg1;

    return msg;
}

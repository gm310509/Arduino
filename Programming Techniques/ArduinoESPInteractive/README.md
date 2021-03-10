# ArduinoESPInteractive

This program should be read in conjunction with my instructable. The instructable
shows how to build an Arduino sketch that uses ESP8266 based WiFi without needing
to constantly adjust DIP switches during the development cycle.

The development should be conducted on an Arduino Mega+WiFi using Serial3 to 
communicate with the ESP (the instructable explains how to do this in detail).

By using Serial3 to communicate with the ESP, the Serial port is free to be used
as per normal for uploading sketches (to the Arduino Mega MCU) and debug via the
USB connection.

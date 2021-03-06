# Arduino Bootcamp
## Stage 1 - POST (Power on Self Test)

In this series of tutorials, we will look at how Integrated Circuits can
do some cool stuff for us. In this case, we will look at an 8 digit LED display
panel that is managed by the MAX7219 controller.

In a future project, we will dig a little bit deeper to see some of the things that
the module (or more precisely the MAX7219) does for us. This will involve using a
more basic LED Panel (only 4 digits) and a few different ways we can connect it to Arduino.

In this set of projects we will build a "Calculator" that receives its input from your connected PC
(via the USB connection).

There are lots of comments in each of the programs (or sketches) which should explain what to do and what to look
for to acheive the learnings.

Enjoy!


## Parts list

To complete this tutorial you will need:

* An Arduino Uno (you can use others, but you might need to adjust some of the code)
* Hookup wire.
* An 8 digit 7 Segment LED Display panel (module) and a MAX7219 (ideally the MAX7219 is builtin
  to the display module.


## Connecting it up
The schematics directory has a fritzing model and a Libre Draw circuit diagram.

To view the circuit in Fritzing, it will be necessary to install the "8 digit 7 segment LED Module with Max7219.fzpz" first.

## Projects
The projects are as follows:

|Project|Description|
|-------|-----------|
|0100-Max7219-Panel-Tester|A basic program to test installation of the libraries and connection of the LED Panel to Arduino|
|0101-Max7219-Panel-Timer|A basic timer program that introduces a function to output a signed integer onto the LED Panel|
|0102-Calcenstein-Slave|A basic calculator (with plent of TODO's for you to enjoy) that is driven by input from the connected PC|


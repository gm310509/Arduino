# Arduino Bootcamp
## Stage 0 - Power on Reset.

In this series of tutorials, we will use a pushbutton (or simply two wires shorted together) to turn on an LED.
But wait there is more, the pushbutton will also be used to turn the LED off again.

**More Importantly**, we will look at a phenomenan know as bounce - which is usually undesriable when it comes
to push buttons and switches.
We will then look at a way to deal with the "problem" in software.

Another outcome from this series will be to look at how the delay() function is not always desirable on Arduino
and a way to deal with that, also in software.

There are lots of comments in each of the programs (or sketches) which should explain what to do and what to look
for to acheive the learnings.

Enjoy!


## Parts list.

To complete this tutorial you will need:

* An Arduino Uno (you can use others, but you might need to adjust some of the code)
* A breadboard and hookup wire.
* An LED and a current limiting resistor (e.g. a 470 ohm resistor).
* A switch (or button, or simply touch two wires together)
* A 10K resistor.


## Connecting it up
The schematics directory has a fritzing model and a Libre Draw circuit diagram.

You will need to install the [LibreSymbols](https://extensions.libreoffice.org/extensions/libresymbols)
addon to see the circuit diagram in Libre Draw.


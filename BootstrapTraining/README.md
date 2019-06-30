# Arduino bootcamp files

## Overview

This directory contains a series of Arduino bootcamp projects.

The projects start with a basic Blink project and build up upon one another.
The series starts with the basics of dealing with hardware (an LED and a switch), next a couple
of modules are introduced (a 7 segment 8 digit Max7219 controlled LED display and a keypad).
Following that we look at how the LED display actually works (i.e. what the MAX7219 does for you)
by working with a similar LED display (this time common cathode 4 digit 7 segment display). We windup
by looking at how IC's can be used to help manage the number of Arduino digital I/O connections
needed to control all those LED's.

I've kind of used the Boot phases that a computer uses to label the modules. I hope you enjoy
my name selections.

## The projects

* Stage 0 - Power on reset - Learn about debouncing with an LED and a switch/button/shorted wire.
* Stage 1 - POST - Learn about how to control the 8 digit 7 segment Max7219 controlled LED display panel.
* Stage 2 - Find boot device - Add a keypad to the LED project and create a simple calculator I call "Calenstein".
You can use Calcenstein for some basic Frankulating!
* Stage 3 - Load OS - Look at what the Max7219 does for you and simplify digitial I/O connections using
supporting IC's.


## Parts required
Following is a list of the parts required for each project.

|Stage|Part|Description|
|----:|-----------|-------|
|0|LED|Any regular LED with an anode and cathode|
|0|Switch|Any Normally Open (NO) push button, switch - or simply just connect two wires together|
|1|LED Display Panel|A 7 segment, 8 digit LED Panel controlled by a Max 7219 chip. [Example](https://www.jaycar.com.au/8-digit-7-segment-display-module/p/XC3714 "Jaycar XC3714")|
|2|4 x 4 Keypad|Any 4 x 4 keypad. [Example](https://core-electronics.com.au/4x4-matrix-keypad.html "Core Electronics ADA3844")|
|3|SI register|I used a 74HC164 SI Parallel out register|
|3|Multiplexor/decoder|I used a 1 of 16 decoder 74HC154 (low when selected) but all that will be required is 1 of 4|
|3|LED Panel|A seven segment common cathode LED display panel. I used a TDCG1060M from Vishay|



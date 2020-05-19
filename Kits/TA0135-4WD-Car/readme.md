## TA0135 4WD Car

This set of folders contains enhancements to the set of programs that come with a 4WD kit car.

The main enhancements are to provide conditional compilation for these two primary goals:

1) Running various tests described in the manual using a "test wiring configuration".
2) Running those same tests but with the wiring used for the "final project's wiring configuration"

The reason this is needed is because, for reasons unknown, the manual uses two different wiring configurations (one for testing and a different one for the final project). I personally found it useful to be able to rerun some of the tests when the car had been fully assembled.

This was problematic so I created these enhanced versions of the tests that could easily be switched between the "final" and "test" wiring configurations by defining (or undefining) a symbol FINAL_PROJECT_CONFIGURATION. Details are included in the comments.

Finally, I will add more code here as time permits for various project ideas that I attempt over time.

The [TA0135](https://www.auselectronicsdirect.com.au/4-wheel-drive-with-ultrasonic-line-tracer-bluetoot) part number is the part
number used by Aus Electronics direct, but I've seen the same kit from many other suppliers.

The kit includes:

* 4 x DC motor and tires
* Motor driver module based upon L298N
* An Arduino Uno or clone (Iduino) and expansion shield
* Servo
* Ultrasonic sensor
* Bluetooth
* IR Receiver and remote
* LCD
* Wires
* Screws, nuts, bolts and spacers
* 2 x chassis

An [annotated version of the instruction manual](https://bit.ly/3g3GSO2) can be obtained from my google drive (https://bit.ly/3g3GSO2). The annotations are notes that I made while building the project. They cover two main areas:
1) Things that would have been helpful to know earlier in the development process.
2) Errors and inconsistencies.

![alt-text][carImage]


[carImage]: https://github.com/gm310509/Arduino/blob/master/Kits/TA0135-4WD-Car/TA0135_small.png

*************************
Project 1: Bar Code Scan
Author: Yue Zhang
*************************

In this laboratory experiment a UPC-A bar code scanner was designed, built, and tested.  It mainly emphasizes on the usage of GPIO pins on BeagleBone and the algorithm for reading a bar code.  

Hardware wiring. The HP Barcode Scanning Wand has 5 pins, and 3 of them are used in this lab. Pin 1 is connected to +5V, pin 4 is connected to ground, and pin 3 is connected to +3.3V through a 4.7kΩ pull-up resister. Pin 3 is the TTL-level signal wire. When the wand is held over a white space, the output of pin 3 is 0V; when momentarily over a black space it goes high(3.3V) for a short time (less than 0.3s). The output of the wand, e.g. pin 3, should be connected to a GPIO pin of BeagleBone. I connected the output signal to GPIO0_7 on the BeagleBone.

In the program, first, I asked the user to put an argument number together with the execution command to indicate which GPIO is used on the BeagleBone. If the user enters the input format wrong, I make the program display a prompt message on the screen. After setting up the GPIO as input, I make the program check the state of gpio contiguously until it goes high, which means black bar is encountered in the scanning. Then I set up a counter to count the relatively passed time while scanning, and saved them into an array. Then I calculated the relative width of each bar and rounded them into the closest number. So I got a 4-digit code for each bar code number. Then I translated them into real bar code with a lookup table. There are 12 digits in total and the 12th number is the checksum number. I calculated whether the result is right according to the checksum number, and print the bar code out if the result is correct.

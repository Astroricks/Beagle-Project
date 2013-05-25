*************************
Project 2: Ping Pong Game
Author: Yue Zhang
*************************

Hardware wiring. Since BeagleBone doesn’t have any onboard LEDs and switches, I have to wire up 6 LEDs and 3 switches for the project. In this project the LEDs is connected to the 11, 13, 15, 17, 19 and 21 pins on expansion header P9 separately. A protection resistor of 220Ω should be connected between the LEDs and ground to limit the current. Since only 1 LED will be lit up every time in this project, 1 resistor should be enough. The switches each have one end connected to 25, 27, 29 pins on expansion header P9 separately, and the other end to the ground.

Select the working mode. Most of the default working modes of these pins are not gpio. The gpio modes of these pins are all mode 7. So we should set up the working mode of these pins as 7 in order to use gpio. The pin mux resource is located in this folder on BeagleBone: 
/sys/kernel/debug/omap_mux/
We could look up in the SRM to find out which file we should write. After that, we would be able to set the mode. The intuitive way is to write hex numbers directly into the file. The muxing bits are shown below:
Bit 5: 1 - Input, 0 - Output
Bit 4: 1 - Pull up, 0 - Pull down
Bit 3: 1 - Pull disabled, 0 - Pull enabled
Bit 2 \
Bit 1 |- Mode
Bit 0 /

For example, if we want to set up the LEDs, we should use this command:
echo 7 > [corresponding file]	
(7 is %000111, e.g. output, pull down, pull enabled, mode 7)
If we want to set up the switches, we should use this command:
echo 37 > [corresponding file]	
(37 is %110111, e.g. input, pull up, pull enabled, mode 7)
After setting the pin mux mode, don’t forget to export the corresponding gpios and set direction for them.

Using the header file “gpio.h” would be easy to manipulate gpios. The central button serves as start button. The program keeps pulling the value of this switch until it goes low. Then the game begins. An LED will be lit up, then after a certain delay turned off and the next one is lit up. The delay time could be adjusted so we can have different speed in this game. When the light of one end is lit up, the program pulls the corresponding switch button. If it is not pressed during a fixed time, the game will end. Otherwise, the lights get lit up in the counter sequence to make it look like a ping pong game.


*******************
Mini Project 01
Author: Yue Zhang
*******************

The requirements:

It's up to you to decide just what your mini project does, however it must:

    use at least 1 interupt driven gpio input (it could read a switch)
    use one gpio output (it could blink an LED)
    use a PWM output (maybe blink or dim and LED)
    use an i2c device (read a temperature?)
    use at least one analog in (read a voltage?)
    handle a ^C interrupt 



A switch is used as an interupt driven gpio input.
An LED works as a gpio output. Every time the switch is pressed, the LED will be turned on/off.
Another LED is setted to blink at the frequency of 20 Hz as a PWM output.
TMP 101 is a i2c device witch catches the temperature. Every time the switch is pressed, it will read the value of temperature and display on the screen.
A variable resistor is used in the analog in. If we change the value of the resistor, the PWM driven LED's duty cycle will be changed too. Thus its brightness will be adjusted.
A ^C interrupt is introduced to close the file. 


The main code is in Mini_project_01.c. You will also need to download the header file i2c-dev.h. Compile and run the Mini_project_01.c or run the Mini_project_01 directly. 

Usage: <gpio-button> <gpio-led> <i2c-bus> <i2c-address> <i2c-daddress> <analog-in>

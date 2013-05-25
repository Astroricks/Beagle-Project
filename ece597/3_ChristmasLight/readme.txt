*********************************
Project 3: Christmas Light Show
Author: Yue Zhang
*********************************

In this project, BeagleBone is connected to a Christmas light show board through RS232 to drive the board playing a light show.

Hardware wiring: Since we only use Beagle Bone for sending out characters, two interface wires would be enough for serial communication: UART_TXD and GROUND. After wiring BeagleBone’s UART_TXD to Christmas Light’s RXD pinout, and connecting their ground signals together, they should be able to communicate.

Select the working mode: I chose to use UART1_TXD in this project. And we need to set the pin to Mode 0 to work in TXD mode.

To manipulate UARTs on embedded Linux operating system, we could use termios, a Unix API for terminal I/O. The anatomy of a program performing serial I/O with the help of termios is as follows:
1) Open serial device with standard Unix system call “open”
2) Configure communication parameters and other interface properties with the help of specific termios functions and data structures.
3) Use standard Unix system calls “read” and “write” for reading from, and writing to the serial interface. 
4) Close device with the standard Unix system call “close” when done.

There are Linux character devices assigned to the BeagleBone UARTs: UART1 is /dev/ttyO1.

Details about termios could be found here:
http://pubs.opengroup.org/onlinepubs/007908775/xsh/termios.h.html

In this project, I wrote a function to set serial attributes, e.g. setting the baud rate, and setting the interval time between each character. And two different functions are written to send out characters with odd parity or even parity.

To communicate with the Christmas light board, a group of data sent out through serial port should be in the following format:
sendCharEven(adr);// Board address, 0x01 in this project; Even parity.
sendCharOdd(dat1);// 3 bytes that control on/off of the lights; Odd parity.
sendCharOdd(dat2);
sendCharOdd(dat3);
	
So we could make a light show by sending out bytes correctly.

You can find the demo video on Youtube here:
https://www.youtube.com/watch?v=5R4uGQVnWuU


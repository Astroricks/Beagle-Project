#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h> // Defines signal-handling functions (i.e. trap Ctrl-C)
#include "gpio.h"

#define PIN_MUX_PATH "/sys/kernel/debug/omap_mux/"
#define MAX_BUF 64

/****************************************************************
* Global variables
****************************************************************/
int keepgoing = 1;	// Set to 0 when ctrl-c is pressed
unsigned int led[6] = {30, 31, 48, 5, 13, 3};	//numbers of gpios to be used for led
unsigned int sw[3] = {117, 115, 111};
unsigned int gamespeed = 300;	//Set the speed of game. Note: the smaller, the faster

/****************************************************************
* signal_handler
****************************************************************/
// Callback called when SIGINT is sent to the process (Ctrl-C)
void signal_handler(int sig)
{
	printf( "Ctrl-C pressed, cleaning up and exiting..\n" );
	keepgoing = 0;
}

/****************************************************************
* Set the mode to gpio output
****************************************************************/
int mode_gpio_out(char *pinMux)
{
	int fd, len;
	char buf[MAX_BUF];
 
	len = snprintf(buf, sizeof(buf), PIN_MUX_PATH "%s", pinMux);
 
	fd = open(buf, O_WRONLY);
	if (fd < 0) 
	{
		perror("mode/gpio");
		return fd;
	}
 
	write(fd, "7", 2);	//'7' means %000111, e.g. mode 7(gpio), pull enabled, pull down, output
 
	close(fd);
	return 0;
}

/****************************************************************
* Set the mode to gpio input
****************************************************************/
int mode_gpio_in(char *pinMux)
{
	int fd, len;
	char buf[MAX_BUF];
 
	len = snprintf(buf, sizeof(buf), PIN_MUX_PATH "%s", pinMux);
 
	fd = open(buf, O_WRONLY);
	if (fd < 0) 
	{
		perror("mode/gpio");
		return fd;
	}
 
	write(fd, "37", 3);	//'37' means %110111, e.g. mode 7(gpio), pull enabled, pull up, input
 
	close(fd);
	return 0;
}

/****************************************************************
* Initialize IO
****************************************************************/
void initIO(){
	char gpio30[] = "gpmc_wait0";
	char gpio31[] = "gpmc_wpn";
	char gpio48[] = "gpmc_a0";
	char gpio5[] = "spi0_cs0";
	char gpio13[] = "uart1_rtsn";
	char gpio3[] = "spi0_d0";
	char gpio117[] = "mcasp0_ahclkx";
	char gpio115[] = "mcasp0_fsr";
	char gpio111[] = "mcasp0_fsx";

	int i;

	//Set pin mux in gpio output mode for LEDs
	mode_gpio_out(gpio30);
	mode_gpio_out(gpio31);
	mode_gpio_out(gpio48);
	mode_gpio_out(gpio5);
	mode_gpio_out(gpio13);
	mode_gpio_out(gpio3);

	//Set pin mux in gpio input mode for switches
	mode_gpio_in(gpio117);
	mode_gpio_in(gpio115);
	mode_gpio_in(gpio111);

	//Export gpios and set up output direction for LEDs
	for (i = 0; i < 6; i++){
		gpio_export(led[i]);
		gpio_set_direction(led[i], 1);
	}
	
	//Export gpios and set up input direction for switches
	for (i = 0; i < 3; i++){
		gpio_export(sw[i]);
		gpio_set_direction(sw[i], 0);
	}	
}

/****************************************************************
* Delay Function
****************************************************************/
void delay (unsigned int loops)
{
	int i, j;
	for (i = 0; i < loops; i++)
		for (j = 0; j < 50000; j++);
}

/****************************************************************
* Main
****************************************************************/
int main(int argc, char *argv[]){
	// Set the signal callback for Ctrl-C
	signal(SIGINT, signal_handler);
	int swVal[3] = {1, 1, 1};
	int scoreL = 0, scoreR = 0;
	int inGame = 0, leftFirst = 1, backForward = 0;
	unsigned int i,j;

	initIO();
	
	while(keepgoing){
		printf("Left Score: %d\tRight Score: %d\n", scoreL, scoreR);
		printf("Press the central button to start next round!\n"
			"Or press Ctrl-C to exit.\n");
		gpio_get_value(sw[1], &swVal[1]);	//Read central switch
		while(swVal[1] && keepgoing){	//gpio high means sw not pushed yet
			gpio_get_value(sw[1], &swVal[1]);
		}
		printf("Let's Ping Pong!\n");
		inGame = 1;
		
		if(leftFirst)
			gpio_set_value(led[0], 1);	//Light up the leftmost LED
		else
			gpio_set_value(led[5], 1);	//Light up the rightmost LED
		delay(gamespeed);	//Interval between toggling LEDs
		while(inGame && keepgoing){
			//Left first and backforward case
			if((leftFirst || backForward) && inGame){	
				for(i = 0; i < 4; i++){	
					gpio_set_value(led[i], 0);
					gpio_set_value(led[i + 1], 1);
					delay(gamespeed);
				}
				gpio_set_value(led[4], 0);
				gpio_set_value(led[5], 1);
				//Check whether the paddle button is pushed
				gpio_get_value(sw[2], &swVal[2]);				
				i = 0;
				while(swVal[2] && i < gamespeed*15){
					gpio_get_value(sw[2], &swVal[2]);
					i++;
				}
				if(swVal[2]){	//If not pushed, game ends, left player win
					gpio_set_value(led[5], 0);
					inGame = 0;
					backForward = 0;
					printf("Left player win!\n");
					scoreL++;
				}
			}
			//Right first and backforward case
			if(inGame){
				for(i = 5; i > 1; i--){	
					gpio_set_value(led[i], 0);
					gpio_set_value(led[i - 1], 1);
					delay(gamespeed);
				}
				gpio_set_value(led[1], 0);
				gpio_set_value(led[0], 1);	
				//Check whether the paddle button is pushed
				gpio_get_value(sw[0], &swVal[0]);				
				i = 0;
				while(swVal[0] && i < gamespeed*15){
					gpio_get_value(sw[0], &swVal[0]);
					i++;
				}
				if(swVal[0]){	//If not pushed, game ends, right player win
					gpio_set_value(led[0], 0);
					inGame = 0;
					backForward = 0;
					printf("Right player win!\n");
					scoreR++;
				}
				else	//Make sure the LEDs react correctly when hitting back
					backForward = 1;	
			}
		}
		leftFirst = 1 - leftFirst; //Toggle the first ball direction
	}
	return 0;
} 

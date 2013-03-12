#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h> // Defines signal-handling functions (i.e. trap Ctrl-C)
#include "gpio.h"

#define BAR_MAX 59

/****************************************************************
* Global variables
****************************************************************/
int keepgoing = 1;	// Set to 0 when ctrl-c is pressed

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
* Delay Function
****************************************************************/
void delay (unsigned int loops)
{
	int i;
	for (i = 0; i < loops; i++);
}



/****************************************************************
* Main Function
****************************************************************/
int main(int argc, char *argv[]){
	unsigned int gpioState = 0;
	unsigned int *gpioStatePtr = &gpioState;
	unsigned int rawCount[BAR_MAX];	
	unsigned int gpioNum, i, j, sum, relTime[4];
	int code[12], errorFlag, oddSum, evenSum, checksum;
	float avg;
	char str[80]; 	

	// Deal with input instruction
	if (argc < 2) {
		printf("Usage: %s <gpio pin number (0 - 127)>\n", argv[0]);
		printf("The default gpio used for this project should be 7\n");
		exit(-1);
	}

	// Set the signal callback for Ctrl-C
	signal(SIGINT, signal_handler);
	
	// Get the gpio number from the input
	gpioNum = atoi(argv[1]);
	
	// Set the gpio pin to be input and set its working pattern
	gpio_export(gpioNum);
	gpio_set_direction(gpioNum, 0);
	
	printf("Ready to scan a bar code!\n");

	while(keepgoing)
	{
		errorFlag = 0;
		while(!gpioState && keepgoing)	//Wait until gpio value goes high
			gpio_get_value(gpioNum, gpioStatePtr);	
		printf("Bar code detected!\n");
		
		for(i = 0; i < BAR_MAX; i++)
		{	
			int count = 1;
			if(gpioState == 1)
			{	//Count in every cycle when the scanner is on the black bar
				while(gpioState && keepgoing)
				{
					count++;
					delay(50);
					gpio_get_value(gpioNum, gpioStatePtr);
				}
			}
			else if (gpioState == 0)
			{	//Count in every cycle when the scanner is on the white bar
				while(!gpioState && keepgoing)
				{
					count++;
					delay(50);
					gpio_get_value(gpioNum, gpioStatePtr);
				}
			}
			//Save the counted number to an array
			rawCount[i] = count;
			printf("%d  ", count);
		}
		printf("\nBar code scan finished!\n");
		
		for(i = 0; i < 6; i++)   //get the first 6 digits numbers
		{            
			sum = 0;
			code[i] = 0;
			for(j = 0; j < 4; j++)   //calculate average for every 4 elements
				sum += rawCount[4 * i + j + 3];
			avg = sum / 7.0;
			for(j = 0; j < 4; j++)
			{
				relTime[j] = (int) (rawCount[4 * i + j + 3] / avg + 0.5);  //rounding to relative time
				code[i] = code[i] * 10 + relTime[j];
			}

			printf("%d\n", code[i]);
		}

		for(i = 6; i < 12; i++)   //get the last 6 digits numbers
		{            
			sum = 0;
			code[i] = 0;
			for(j = 0; j < 4; j++)   //calculate average for every 4 elements
				sum += rawCount[4 * i + j + 8];
			avg = sum / 7.0;
			for(j = 0; j < 4; j++)
			{
				relTime[j] = (int) (rawCount[4 * i + j + 8] / avg + 0.5);  //rounding to relative time
				code[i] = code[i] * 10 + relTime[j];
			}

			printf("%d\n", code[i]);
		}
		
		for(i = 0; i < 12; i++)		//Use lookup table to get real bar code
		{       
    			switch(code[i]){      
    				case 3211:
      					code[i] = 0;
      					break;
    				case 2221:
					code[i] = 1;
      					break;
    				case 2122:
      					code[i] = 2;
      					break;
    				case 1411:
      					code[i] = 3;
      					break;
    				case 1132:
      					code[i] = 4;
      					break;
    				case 1231:
      					code[i] = 5;
      					break;
    				case 1114:
      					code[i] = 6;
      					break;
   				case 1312:
      					code[i] = 7;
      					break;
    				case 1213:
      					code[i] = 8;
      					break;
    				case 3112:
      					code[i] = 9;
      					break;
				default:
					code[i] = -1;	
					errorFlag = 1;
					break;
    			}
		}
		
		//Checksum calculation
		oddSum = 0;
		evenSum = 0;
		for(i = 0; i < 11; i++){
    			if(i % 2 == 0)
      				oddSum += code[i]; 
    			else 
      				evenSum += code[i];
  		}
		checksum = 10 - ((oddSum *3 + evenSum) % 10);
		if (code[11] != checksum)
			errorFlag = 1;

		//Print the detected bar code
		if (errorFlag == 1)
		{
			printf("Bar code scan failure: ");
			for(i = 0; i < 12; i++)
			{				
				if (code[i] != -1)
					printf("%d", code[i]);
				else 
					printf("*");
			}
			printf("\n");
		}
		else if (errorFlag == 0)
		{
			printf("Bar code scan success: ");
			for(i = 0; i < 12; i++)
			{				
				if (code[i] != -1)
					printf("%d", code[i]);
				else 
					printf("*");
			}
			printf("\n");
		}

		printf("Press Enter to start next scan!\n");
		printf("(Or press Ctrl-C then press Enter to exit.)\n");
		char enter = 0;
		while (enter != '\r' && enter != '\n' && keepgoing) { enter = getchar(); }
		if(keepgoing)
			printf("Ready to scan a bar code!\n");
	}
		
	
	return 0;		

}




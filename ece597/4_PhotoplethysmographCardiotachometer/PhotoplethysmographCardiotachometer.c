#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>	// Defines signal-handling functions (i.e. trap Ctrl-C)

/****************************************************************
* Constants
****************************************************************/
#define MAX_BUF 64

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
* analog input
****************************************************************/
int analogIn(char *ain)
{
	FILE *fp;
	char ainPath[MAX_BUF];
	char ainVal[MAX_BUF];
	
	snprintf(ainPath, sizeof ainPath, "/sys/devices/platform/omap/tsc/%s", ain);

	if((fp = fopen(ainPath, "r")) == NULL){
	printf("Can't open this pin, %s\n", ain);
	return 1;
	}

	fgets(ainVal, MAX_BUF, fp);

	fclose(fp);
	return atoi(ainVal);		
}

/****************************************************************
* Delay Function
****************************************************************/
void delay (unsigned int loops)
{
	int i, j;
	if(keepgoing)
		for (i = 0; i < loops; i++)
			for (j = 0; j < 50000; j++);
}

/****************************************************************
 * Main
 ****************************************************************/
int main(int argc, char **argv, char **envp)
{	
	int analog_value;
	char ain[] = "ain6";
	int heartbeat = 0;
	
	// Set the signal callback for Ctrl-C
	signal(SIGINT, signal_handler);

	while (keepgoing) {
		
		while(analog_value < 2100 && keepgoing)
			analog_value = analogIn(ain);
		while(analog_value > 1800 && keepgoing)
			analog_value = analogIn(ain);
	
		if(keepgoing)
			heartbeat++;

		printf("Heartbeat count: %d\n", heartbeat);
	}
}



#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h> 
#include <termios.h>    //Enables setting baud rate for RX/TX
#include <signal.h> 	// Defines signal-handling functions (i.e. trap Ctrl-C)

#define BAUDRATE B9600

int initUART();
int sendSerial(char *msg);
void signal_handler(int sig);

int keepgoing = 1;	// Set to 0 when ctrl-c is pressed

/****************************************************************
* Main
****************************************************************/
int main(){
	char msg = 'a';
	
	// Set the signal callback for Ctrl-C
	signal(SIGINT, signal_handler);

	initUART();
	while(keepgoing){
		sendSerial(&msg);
	}
	return 0;
}

/****************************************************************
* Initiate UART1_TXD
****************************************************************/
int initUART(){
	int fd, len;
	char *uart1_tx = "/sys/kernel/debug/omap_mux/uart1_txd";
	fd = open(uart1_tx, O_WRONLY);
	if (fd < 0) 
	{
		perror("Can't open uart1_txd\n");
		return fd;
	}
	
	write(fd, "0", 2);
	close(fd);
	return 0;
}

/****************************************************************
* Send character through serial
****************************************************************/
int sendSerial(char *msg) {
	struct termios Serial;
	int size = strlen(msg);
	int fd;
	if ((fd = open("/dev/ttyO1", O_RDWR | O_NOCTTY)) < 0){
		printf("Could not open ttyO1.\n");
		return -1;
	}
	if (cfsetospeed(&Serial, BAUDRATE) < 0){
		printf("Baud rate not successfully set.\n");
		return -1;
	}
	if (tcgetattr(fd, &Serial) != 0){ // Obtain current terminal device settings
		printf("Unable to retrieve port attributes.\n");
		return -1;
	}
	Serial.c_iflag = 0;
	Serial.c_oflag = 0;
	Serial.c_lflag = 0;
	Serial.c_cflag |= PARENB | PARODD;	//Enable odd parity
	Serial.c_cc[VMIN] = 0;
	Serial.c_cc[VTIME] = 1;	// Timeout after 0.1 seconds
	tcsetattr(fd, TCSANOW, &Serial); // Set the modified attributes
	write(fd, msg, size);
	close(fd);

	return 0;
}

/****************************************************************
* Signal_handler
****************************************************************/
// Callback called when SIGINT is sent to the process (Ctrl-C)
void signal_handler(int sig)
{
	printf( "Ctrl-C pressed, cleaning up and exiting..\n" );
	keepgoing = 0;
}

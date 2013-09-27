#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h> 
#include <termios.h>    //Enables setting baud rate for RX/TX
#include <signal.h> 	// Defines signal-handling functions (i.e. trap Ctrl-C)

#define BAUDRATE B9600

int initUART();
int setSerial();
int sendCharSet(unsigned char *msg);
int sendChar(unsigned char msg);
int sendCharOdd(unsigned char msg);
int sendCharEven(unsigned char msg);
int sendPattern(unsigned char *dat);
void signal_handler(int sig);
void delay (unsigned int loops);
void cwrotate(unsigned int time);
void ccwrotate(unsigned int time);
void burstout(unsigned int time);
void burstin(unsigned int time);
void starburstout(unsigned int time);
void starburstin(unsigned int time);
void strobe(unsigned int time);
void spiralin(unsigned int time);
void spiralout(unsigned int time);
void flashon_off(unsigned int time);
void snake(unsigned int time);

int keepgoing = 1;	// Set to 0 when ctrl-c is pressed


/****************************************************************
* Main
****************************************************************/
int main(){	
	unsigned char testMsg[]={0xFF, 0x00, 0x03, 0x1D, 0x0C};

	// Set the signal callback for Ctrl-C
	signal(SIGINT, signal_handler);

	initUART();
	setSerial();
	while(keepgoing){
		sendCharSet(testMsg);
		delay(1000);
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
* Set serial attributes
****************************************************************/
int setSerial() {
	struct termios Serial;
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
	Serial.c_iflag = 0;	//Clear all flags for input modes
	Serial.c_oflag = 0;	//Clear all flags for output modes
	Serial.c_lflag = 0;	//Clear all flags for local modes
	Serial.c_cflag &= ~PARENB;	//Enable parity check
	Serial.c_cc[VMIN] = 0;
	Serial.c_cc[VTIME] = 1;	// Timeout for 0.1 seconds
	close(fd);

	return 0;
}

/****************************************************************
* Send a set of 5 characters without parity check
****************************************************************/
int sendCharSet(unsigned char *msg) {
	struct termios Serial;
//	int size = strlen(&msg);	//Note: This may cause error when msg==0!
	char size = 5;
	int fd;
	if ((fd = open("/dev/ttyO1", O_RDWR | O_NOCTTY)) < 0){
		printf("Could not open ttyO1.\n");
		return -1;
	}	
	if (tcgetattr(fd, &Serial) != 0){ // Obtain current terminal device settings
		printf("Unable to retrieve port attributes.\n");
		return -1;
	}
	Serial.c_cflag &= ~PARENB;	//Disable parity check
	tcsetattr(fd, TCSANOW, &Serial); // Set the modified attributes
	write(fd, msg, size);
	close(fd);

	return 0;
}

/****************************************************************
* Send character without parity check
****************************************************************/
int sendChar(unsigned char msg) {
	struct termios Serial;
//	int size = strlen(&msg);	Note: This would cause error when msg==0!
	char size = 1;
	int fd;
	if ((fd = open("/dev/ttyO1", O_RDWR | O_NOCTTY)) < 0){
		printf("Could not open ttyO1.\n");
		return -1;
	}	
	if (tcgetattr(fd, &Serial) != 0){ // Obtain current terminal device settings
		printf("Unable to retrieve port attributes.\n");
		return -1;
	}
	Serial.c_cflag &= ~PARENB;	//Disable parity check
	tcsetattr(fd, TCSANOW, &Serial); // Set the modified attributes
	write(fd, &msg, size);
	close(fd);

	return 0;
}

/****************************************************************
* Send character with odd parity
****************************************************************/
int sendCharOdd(unsigned char msg) {
	struct termios Serial;
//	int size = strlen(&msg);	Note: This would cause error when msg==0!
	char size = 1;
	int fd;
	if ((fd = open("/dev/ttyO1", O_RDWR | O_NOCTTY)) < 0){
		printf("Could not open ttyO1.\n");
		return -1;
	}	
	if (tcgetattr(fd, &Serial) != 0){ // Obtain current terminal device settings
		printf("Unable to retrieve port attributes.\n");
		return -1;
	}
	Serial.c_cflag |= PARENB | PARODD;	//Enable parity check and odd parity
	tcsetattr(fd, TCSANOW, &Serial); // Set the modified attributes
	write(fd, &msg, size);
	close(fd);

	return 0;
}

/****************************************************************
* Send character with even parity
****************************************************************/
int sendCharEven(unsigned char msg) {
	struct termios Serial;
//	int size = strlen(&msg);	Note: This would cause error when msg==0!
	char size = 1;
	int fd;
	if ((fd = open("/dev/ttyO1", O_RDWR | O_NOCTTY)) < 0){
		printf("Could not open ttyO1.\n");
		return -1;
	}	
	if (tcgetattr(fd, &Serial) != 0){ // Obtain current terminal device settings
		printf("Unable to retrieve port attributes.\n");
		return -1;
	}
	Serial.c_cflag |= PARENB;	//Enable parity check
	Serial.c_cflag &= ~PARODD;	//Clear odd parity bit to enable even parity
	tcsetattr(fd, TCSANOW, &Serial); // Set the modified attributes
	write(fd, &msg, size);		
	close(fd);

	return 0;
}

/****************************************************************
* Send a certain pattern
****************************************************************/
int sendPattern(unsigned char *dat)
{
	int i;
	int arrayLength = sizeof(dat)/sizeof(unsigned char);
	for (i = 0; i<arrayLength; i++) {
		sendChar(dat[i]);
	}
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


#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h> 
#include <termios.h>    //Enables setting baud rate for RX/TX
#include <signal.h> 	// Defines signal-handling functions (i.e. trap Ctrl-C)
#include <pthread.h>

#define BAUDRATE B9600
#define ARRAY_SIZE 32

int initUART1_TXD();
int initUART1_RXD();
int setSerial();
int readCharSet();
int sendCharSet(unsigned char *msg);
void signal_handler(int sig);

int keepgoing = 1;	// Set to 0 when ctrl-c is pressed

pthread_t tid[1];


void* receiveThread(void *arg)
{
    unsigned long i = 0;
    pthread_t id = pthread_self();

    if(pthread_equal(id,tid[0]))
    {
        printf("\n First thread processing\n");
    }

	while(1) 
	{
    	printf("\n Trying to read something\n");
		if(readCharSet() != 0){}
			//break;
	}

    return NULL;
}

/****************************************************************
* Main
****************************************************************/
int main(){	
	unsigned char testMsg[]={0xFF, 0x00, 0x03, 0x1D, 0x0C};
	int err;

	// Set the signal callback for Ctrl-C
	signal(SIGINT, signal_handler);

	initUART1_TXD();
	initUART2_RXD();
	setSerial();
 	err = pthread_create(&(tid[0]), NULL, &receiveThread, NULL);
    if (err != 0)
        printf("\ncan't create thread :[%s]", strerror(err));
    else
        printf("\n Thread created successfully\n");

	while(keepgoing){
		sleep(1);
        printf("\n Sending start\n");
		sendCharSet(testMsg);
		sleep(1);
	}
	return 0;
}

/****************************************************************
* Initiate UART1_TXD
****************************************************************/
int initUART1_TXD(){
	int fd;
	char *uart1_tx = "/sys/kernel/debug/omap_mux/uart1_txd";
	fd = open(uart1_tx, O_WRONLY);
	if (fd < 0) 
	{
		perror("Can't open uart1_txd\n");
		return fd;
	}
	
	write(fd, "00", 2);
	close(fd);
	return 0;
}

/****************************************************************
* Initiate UART2_RXD
****************************************************************/
int initUART2_RXD(){
	int fd;
	char *uart2_rx = "/sys/kernel/debug/omap_mux/spi0_sclk";
	fd = open(uart2_rx, O_WRONLY);
	if (fd < 0) 
	{
		perror("Can't open uart2_rxd\n");
		return fd;
	}
	
	write(fd, "21", 2); //Enable receiver, set to mode 1 (uart2_rxd)
	close(fd);
	return 0;
}

/****************************************************************
* Set serial attributes
****************************************************************/
int setSerial() {
	struct termios Serial;
	int fd;
	// Set the sending port ttyO1
	if ((fd = open("/dev/ttyO1", O_RDWR | O_NOCTTY)) < 0){
		printf("Could not open ttyO1.\n");
		return -1;
	}
	if (tcgetattr(fd, &Serial) != 0){ // Obtain current terminal device settings
		printf("Unable to retrieve port attributes.\n");
		return -1;
	}
	if (cfsetospeed(&Serial, BAUDRATE) < 0){
		printf("Baud rate not successfully set.\n");
		return -1;
	}
	Serial.c_iflag = 0;	//Clear all flags for input modes
	Serial.c_oflag = 0;	//Clear all flags for output modes
	Serial.c_lflag = 0;	//Clear all flags for local modes
	Serial.c_cflag &= ~PARENB;	//Disable parity check
	Serial.c_cc[VMIN] = 0;
	Serial.c_cc[VTIME] = 2;	// Timeout for 0.1 * 2 seconds
	tcsetattr(fd, TCSANOW, &Serial); // Set the modified attributes
	close(fd);

	// Set the receiving port ttyO2
	if ((fd = open("/dev/ttyO2", O_RDWR | O_NOCTTY)) < 0){
		printf("Could not open ttyO1.\n");
		return -1;
	}
	if (tcgetattr(fd, &Serial) != 0){ // Obtain current terminal device settings
		printf("Unable to retrieve port attributes.\n");
		return -1;
	}
	if (cfsetispeed(&Serial, BAUDRATE) < 0){
		printf("Baud rate not successfully set.\n");
		return -1;
	}
	Serial.c_iflag = 0;	//Clear all flags for input modes
	Serial.c_oflag = 0;	//Clear all flags for output modes
	Serial.c_lflag = 0;	//Clear all flags for local modes
	Serial.c_cflag |= CREAD | CS8;	//Enable receiver; 8-bit, no parity, 1 stop bit
	Serial.c_cflag &= ~PARENB;	//Disable parity check
	Serial.c_cc[VMIN] = 2;
	Serial.c_cc[VTIME] = 10;	// Timeout for 0.1 * 2 seconds
	tcsetattr(fd, TCSANOW, &Serial); // Set the modified attributes
	close(fd);

	return 0;
}

/****************************************************************
* Read a set of characters
****************************************************************/
int readCharSet() {
	char byte_in[ARRAY_SIZE];
	struct termios Serial;
	int fd, len, i;
	for(i = 0; i < ARRAY_SIZE; i++) {
		byte_in[i] = 0;
	}
	if ((fd = open("/dev/ttyO2", O_RDWR | O_NOCTTY)) < 0){
		printf("Could not open ttyO2.\n");
		return -1;
	}	
	if (tcgetattr(fd, &Serial) != 0){ // Obtain current terminal device settings
		printf("Unable to retrieve port attributes.\n");
		return -1;
	}
	Serial.c_cflag &= ~PARENB;	//Disable parity check
	tcsetattr(fd, TCSANOW, &Serial); // Set the modified attributes
	usleep(50000);
	len = read(fd, byte_in, ARRAY_SIZE); //Read ttyO2 port, stores data into byte_in

//	for(i = 0; i< ARRAY_SIZE; ) {
//		len = read(fd, byte_in + i, ARRAY_SIZE - i);
//		if (0 < len) 
//			i += len;
//		else if (len == 0)
//			break;
//	}

	for (i = 0; i < ARRAY_SIZE; i++) {
		printf("%X ", byte_in[i]);
	}
	printf("\n%d\n", len);
	close(fd);

	if(len > 0) {
		return 1;
	}
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
* Signal_handler
****************************************************************/
// Callback called when SIGINT is sent to the process (Ctrl-C)
void signal_handler(int sig)
{
	printf( "Ctrl-C pressed, cleaning up and exiting..\n" );
	keepgoing = 0;
}


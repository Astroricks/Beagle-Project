#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h> 
#include <termios.h>    //Enables setting baud rate for RX/TX
#include <signal.h> 	// Defines signal-handling functions (i.e. trap Ctrl-C)

#define BAUDRATE B9600

int initUART();
int setSerial();
int sendCharOdd(unsigned char msg);
int sendCharEven(unsigned char msg);
int sendPattern(unsigned char adr, unsigned char dat1, unsigned char dat2, unsigned char dat3);
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
	// Set the signal callback for Ctrl-C
	signal(SIGINT, signal_handler);

	initUART();
	setSerial();
	while(keepgoing){
		cwrotate(800);
		ccwrotate(800);
		burstout(1000);
		burstin(1000);
		starburstout(600);
		starburstin(600);
		spiralin(400);
		spiralout(400);
		flashon_off(800);
		snake(200);
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
	Serial.c_cflag |= PARENB;	//Enable parity check
	Serial.c_cc[VMIN] = 0;
	Serial.c_cc[VTIME] = 1;	// Timeout for 0.1 seconds
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
int sendPattern(unsigned char adr, unsigned char dat1, unsigned char dat2, unsigned char dat3)
{
	sendCharEven(adr);
	sendCharOdd(dat1);
	sendCharOdd(dat2);
	sendCharOdd(dat3);
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

/****************************************************************
* Clockwise rotate
****************************************************************/
void cwrotate(unsigned int time)
{
	sendPattern(0x01,0x0f,0xff,0x0f);
	delay(time);
	sendPattern(0x01,0xff,0xf0,0x0f);
	delay(time);
	sendPattern(0x01,0xff,0x0f,0x0f);
	delay(time);
	sendPattern(0x01,0xff,0xff,0x00);
	delay(time);
	sendPattern(0x01,0xf0,0xff,0x0f);
	delay(time);
}

/****************************************************************
* Counter clockwise rotate
****************************************************************/
void ccwrotate(unsigned int time)
{
	sendPattern(0x01,0x0f,0xff,0x0f);
	delay(time);
	sendPattern(0x01,0xf0,0xff,0x0f);
	delay(time);
	sendPattern(0x01,0xff,0xff,0x00);
	delay(time);
	sendPattern(0x01,0xff,0x0f,0x0f);
	delay(time);
	sendPattern(0x01,0xff,0xf0,0x0f);
	delay(time);
}

/****************************************************************
* Swelling out
****************************************************************/
void burstout(unsigned int time)
{
	sendPattern(0x01,0xff,0xff,0xff);
	delay(time);
	sendPattern(0x01,0xff,0xff,0x0f);
	delay(time);
	sendPattern(0x01,0xee,0xee,0x0e);
	delay(time);
	sendPattern(0x01,0xcc,0xcc,0x0c);
	delay(time);
	sendPattern(0x01,0x88,0x88,0x08);
	delay(time);
	sendPattern(0x01,0x00,0x00,0x00);
	delay(time);
}

/****************************************************************
* Shrinking in
****************************************************************/
void burstin(unsigned int time)
{
	sendPattern(0x01,0x00,0x00,0x00);
	delay(time);
	sendPattern(0x01,0x88,0x88,0x08);
	delay(time);
	sendPattern(0x01,0xcc,0xcc,0x0c);
	delay(time);
	sendPattern(0x01,0xee,0xee,0x0e);
	delay(time);
	sendPattern(0x01,0xff,0xff,0x0f);
	delay(time);
	sendPattern(0x01,0xff,0xff,0xff);
	delay(time);
}

/****************************************************************
* Swelling concentric circles
****************************************************************/
void starburstout(unsigned int time)
{
	sendPattern(0x01,0xff,0xff,0xff);
	delay(time);
	sendPattern(0x01,0xff,0xff,0x0f);
	delay(time);
	sendPattern(0x01,0xee,0xee,0xfe);
	delay(time);
	sendPattern(0x01,0xdd,0xdd,0xfd);
	delay(time);
	sendPattern(0x01,0xbb,0xbb,0xfb);
	delay(time);
	sendPattern(0x01,0x77,0x77,0xf7);
	delay(time);
}

/****************************************************************
* Shrinking concentric circles
****************************************************************/
void starburstin(unsigned int time)
{
	sendPattern(0x01,0x77,0x77,0x77);
	delay(time);
	sendPattern(0x01,0xbb,0xbb,0xfb);
	delay(time);
	sendPattern(0x01,0xdd,0xdd,0xfd);
	delay(time);
	sendPattern(0x01,0xee,0xee,0xfe);
	delay(time);
	sendPattern(0x01,0xff,0xff,0x0f);
	delay(time);
	sendPattern(0x01,0xff,0xff,0xff);
	delay(time);
}

/****************************************************************
* All lights on and then all off
****************************************************************/
void strobe(unsigned int time)
{
	sendPattern(0x01,0x00,0x00,0x00);
	delay(time);
	sendPattern(0x01,0xff,0xff,0xff);
	delay(time);
}

/****************************************************************
* Spiral in
****************************************************************/
void spiralin(unsigned int time)
{
	sendPattern(0x01,0xff,0xff,0xff);
	delay(time);
	sendPattern(0x01,0x7f,0xff,0xff);
	delay(time);
	sendPattern(0x01,0xff,0xf3,0xff);
	delay(time);
	sendPattern(0x01,0xff,0x1f,0xff);
	delay(time);
	sendPattern(0x01,0xff,0xff,0xf0);
	delay(time);
	sendPattern(0x01,0xf0,0xff,0x0f);
	delay(time);
	sendPattern(0x01,0x8f,0xff,0x0f);
	delay(time);
	sendPattern(0x01,0xff,0xfc,0x0f);
	delay(time);
	sendPattern(0x01,0xff,0xef,0x0f);
	delay(time);
	sendPattern(0x01,0xff,0xff,0x0f);
	delay(time);
	sendPattern(0x01,0xff,0xff,0xff);
	delay(time);
}

/****************************************************************
* Spiral out
****************************************************************/
void spiralout(unsigned int time)
{
	sendPattern(0x01,0xff,0xff,0xff);
	delay(time);
	sendPattern(0x01,0xff,0xff,0x0f);
	delay(time);
	sendPattern(0x01,0xef,0xff,0x0f);
	delay(time);
	sendPattern(0x01,0xff,0xfc,0x0f);
	delay(time);
	sendPattern(0x01,0xff,0x8f,0x0f);
	delay(time);
	sendPattern(0x01,0xff,0xff,0x00);
	delay(time);
	sendPattern(0x01,0xf0,0xff,0xff);
	delay(time);
	sendPattern(0x01,0x1f,0xff,0xff);
	delay(time);
	sendPattern(0x01,0xff,0xf3,0xff);
	delay(time);
	sendPattern(0x01,0xff,0x7f,0xff);
	delay(time);
	sendPattern(0x01,0xff,0xff,0xff);
	delay(time);
}

/****************************************************************
* All on, all off several times
****************************************************************/
void flashon_off(unsigned int time)
{
	int i;
	for(i = 0; i < 5; i++)
		strobe(time);
}

/****************************************************************
* Snake effect
****************************************************************/
void snake(unsigned int time)
{
	sendPattern(0x01,0xff,0xff,0xff);
	delay(time);
	sendPattern(0x01,0x7f,0xff,0xff);
	delay(time);
	sendPattern(0x01,0x3f,0xff,0xff);
	delay(time);
	sendPattern(0x01,0x1f,0xff,0xff);
	delay(time);
	sendPattern(0x01,0x0f,0xff,0xff);
	delay(time);
	sendPattern(0x01,0x0f,0xff,0x0f);
	delay(time);
	sendPattern(0x01,0x8f,0xef,0x0f);
	delay(time);
	sendPattern(0x01,0xcf,0xcf,0x0f);
	delay(time);
	sendPattern(0x01,0xef,0x8f,0x0f);
	delay(time);
	sendPattern(0x01,0xff,0x0f,0x0f);
	delay(time);
	sendPattern(0x01,0xff,0x0f,0xff);
	delay(time);
	sendPattern(0x01,0xff,0x17,0xff);
	delay(time);
	sendPattern(0x01,0xff,0x33,0xff);
	delay(time);
	sendPattern(0x01,0xff,0x71,0xff);
	delay(time);
	sendPattern(0x01,0xff,0xf0,0xff);
	delay(time);
	sendPattern(0x01,0xff,0xf0,0x0f);
	delay(time);
	sendPattern(0x01,0xff,0xf8,0x0e);
	delay(time);
	sendPattern(0x01,0xff,0xfc,0x0c);
	delay(time);
	sendPattern(0x01,0xff,0xfe,0x08);
	delay(time);
	sendPattern(0x01,0xff,0xff,0x00);
	delay(time);
	sendPattern(0x01,0xff,0xff,0xf0);
	delay(time);
	sendPattern(0x01,0xff,0x7f,0xf1);
	delay(time);
	sendPattern(0x01,0xff,0x3f,0xf3);
	delay(time);
	sendPattern(0x01,0xff,0x1f,0xf7);
	delay(time);
	sendPattern(0x01,0xff,0x0f,0xff);
	delay(time);
	sendPattern(0x01,0xff,0x0f,0x0f);
	delay(time);
	sendPattern(0x01,0xfe,0x8f,0x0f);
	delay(time);
	sendPattern(0x01,0xfc,0xcf,0x0f);
	delay(time);
	sendPattern(0x01,0xf8,0xef,0x0f);
	delay(time);
	sendPattern(0x01,0xf0,0xff,0x0f);
	delay(time);
	sendPattern(0x01,0xf0,0xff,0xff);
	delay(time);
	sendPattern(0x01,0xf1,0xff,0xf7);
	delay(time);
	sendPattern(0x01,0xf3,0xff,0xf3);
	delay(time);
	sendPattern(0x01,0xf7,0xff,0xf1);
	delay(time);
	sendPattern(0x01,0xff,0xff,0xf0);
	delay(time);
	sendPattern(0x01,0xff,0xff,0x00);
	delay(time);
	sendPattern(0x01,0xff,0xff,0x08);
	delay(time);
	sendPattern(0x01,0xef,0xff,0x08);
	delay(time);
	sendPattern(0x01,0xcf,0xff,0x0c);
	delay(time);
	sendPattern(0x01,0x8f,0xff,0x0e);
	delay(time);
	sendPattern(0x01,0x0f,0xff,0x0f);
	delay(time);
	sendPattern(0x01,0x0f,0xff,0xff);
	delay(time);
	sendPattern(0x01,0x17,0xff,0xff);
	delay(time);
	sendPattern(0x01,0x33,0xff,0xff);
	delay(time);
	sendPattern(0x01,0x71,0xff,0xff);
	delay(time);
	sendPattern(0x01,0xf0,0xff,0xff);
	delay(time);
	sendPattern(0x01,0xf0,0xff,0x0f);
	delay(time);
	sendPattern(0x01,0xf8,0xfe,0x0f);
	delay(time);
	sendPattern(0x01,0xfc,0xfc,0x0f);
	delay(time);
	sendPattern(0x01,0xfe,0xf8,0x0f);
	delay(time);
	sendPattern(0x01,0xff,0xf0,0x0f);
	delay(time);
	sendPattern(0x01,0xff,0xf0,0xff);
	delay(time);
	sendPattern(0x01,0xff,0xf1,0xff);
	delay(time);
	sendPattern(0x01,0xff,0xf3,0xff);
	delay(time);
	sendPattern(0x01,0xff,0xf7,0xff);
	delay(time);
	sendPattern(0x01,0xff,0xff,0xff);
	delay(time);
}



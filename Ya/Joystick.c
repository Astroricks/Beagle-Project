/*****************************************************
Something interesting for Ya's 21's birthday!!!
*****************************************************/

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>	// Defines signal-handling functions (i.e. trap Ctrl-C)
#include "i2c-dev.h"
#include "i2cbusses.h"


/****************************************************************
 * Constants
****************************************************************/
 
#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define POLL_TIMEOUT (100) /* 0.1 seconds */
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
 * gpio_export
 ****************************************************************/
int gpio_export(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];
 
	fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}
 
	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);
 
	return 0;
}

/****************************************************************
 * gpio_unexport
 ****************************************************************/
int gpio_unexport(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];
 
	fd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}
 
	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);
	return 0;
}

/****************************************************************
 * gpio_set_dir
 ****************************************************************/
int gpio_set_dir(unsigned int gpio, unsigned int out_flag)
{
	int fd, len;
	char buf[MAX_BUF];
 
	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR  "/gpio%d/direction", gpio);
 
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/direction");
		return fd;
	}
 
	if (out_flag)
		write(fd, "out", 4);
	else
		write(fd, "in", 3);
 
	close(fd);
	return 0;
}

/****************************************************************
 * gpio_set_value
 ****************************************************************/
int gpio_set_value(unsigned int gpio, unsigned int value)
{
	int fd, len;
	char buf[MAX_BUF];
 
	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);
 
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/set-value");
		return fd;
	}
 
	if (value)
		write(fd, "1", 2);
	else
		write(fd, "0", 2);
 
	close(fd);
	return 0;
}

/****************************************************************
 * gpio_get_value
 ****************************************************************/
int gpio_get_value(unsigned int gpio, unsigned int *value)
{
	int fd, len;
	char buf[MAX_BUF];
	char ch;

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);
 
	fd = open(buf, O_RDONLY);
	if (fd < 0) {
		perror("gpio/get-value");
		return fd;
	}
 
	read(fd, &ch, 1);

	if (ch != '0') {
		*value = 1;
	} else {
		*value = 0;
	}
 
	close(fd);
	return 0;
}


/****************************************************************
 * gpio_set_edge
 ****************************************************************/

int gpio_set_edge(unsigned int gpio, char *edge)
{
	int fd, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/edge", gpio);
 
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/set-edge");
		return fd;
	}
 
	write(fd, edge, strlen(edge) + 1); 
	close(fd);
	return 0;
}

/****************************************************************
 * gpio_fd_open
 ****************************************************************/

int gpio_fd_open(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);
 
	fd = open(buf, O_RDONLY | O_NONBLOCK );
	if (fd < 0) {
		perror("gpio/fd_open");
	}
	return fd;
}

/****************************************************************
 * gpio_pullup(gpio0_7)
 ****************************************************************/
int gpio_pullup(void)
{
	FILE *fp;
	char pullup[] = "0x37";
	fp = fopen("/sys/kernel/debug/omap_mux/ecap0_in_pwm0_out", "w+");
	fprintf(fp, "%s", pullup);
	fclose(fp);
}
	
/****************************************************************
 * gpio_fd_close
 ****************************************************************/

int gpio_fd_close(int fd)
{
	return close(fd);
}

/****************************************************************
* analog_in
****************************************************************/
int analog_in(char *ain)
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
* Check
****************************************************************/
static void help(void) __attribute__ ((noreturn));

static void help(void)
{
	fprintf(stderr, "Usage: my2cset (hardwired to bus 3, address 0x70)\n");
	exit(1);
}

static int check_funcs(int file, int size)
{
	unsigned long funcs;

	/* check adapter functionality */
	if (ioctl(file, I2C_FUNCS, &funcs) < 0) {
		fprintf(stderr, "Error: Could not get the adapter "
			"functionality matrix: %s\n", strerror(errno));
		return -1;
	}

	switch (size) {
	case I2C_SMBUS_BYTE:
		if (!(funcs & I2C_FUNC_SMBUS_WRITE_BYTE)) {
			fprintf(stderr, MISSING_FUNC_FMT, "SMBus send byte");
			return -1;
		}
		break;

	case I2C_SMBUS_BYTE_DATA:
		if (!(funcs & I2C_FUNC_SMBUS_WRITE_BYTE_DATA)) {
			fprintf(stderr, MISSING_FUNC_FMT, "SMBus write byte");
			return -1;
		}
		break;

	case I2C_SMBUS_WORD_DATA:
		if (!(funcs & I2C_FUNC_SMBUS_WRITE_WORD_DATA)) {
			fprintf(stderr, MISSING_FUNC_FMT, "SMBus write word");
			return -1;
		}
		break;

	case I2C_SMBUS_BLOCK_DATA:
		if (!(funcs & I2C_FUNC_SMBUS_WRITE_BLOCK_DATA)) {
			fprintf(stderr, MISSING_FUNC_FMT, "SMBus block write");
			return -1;
		}
		break;
	case I2C_SMBUS_I2C_BLOCK_DATA:
		if (!(funcs & I2C_FUNC_SMBUS_WRITE_I2C_BLOCK)) {
			fprintf(stderr, MISSING_FUNC_FMT, "I2C block write");
			return -1;
		}
		break;
	}

	return 0;
}


/****************************************************************
* Main
****************************************************************/
int main(int argc, char *argv[])
{
	struct pollfd fdset[1];
	int nfds = 1;
	int gpio_button_fd, ain_pin_x, ain_pin_y, timeout, rc;
	char *buf[MAX_BUF];
	char ainx[MAX_BUF], ainy[MAX_BUF];
	unsigned int gpio_button;
	float analog_value_x = 0, analog_value_y = 0;
	int i,n = 0;

	int res, i2cbus, address, size, file;
	int value, daddress;
	char filename[20];
	int force = 0, readback = 1;
	__u16 block[I2C_SMBUS_BLOCK_MAX], hidden_bmp[I2C_SMBUS_BLOCK_MAX], bright_bmp[I2C_SMBUS_BLOCK_MAX], trace_bmp[I2C_SMBUS_BLOCK_MAX];
	int len;
	int row = 7, pos = 0x01;

	// Set the signal callback for Ctrl-C
	signal(SIGINT, signal_handler);
	
	// Get the pin values
	gpio_button = 7;
	ain_pin_x = 4;
	ain_pin_y = 6;

	// Set gpio0_7 to be pullup
	gpio_pullup();

	// Set the gpio_button to be input and set its working pattern	
	
	gpio_export(gpio_button);
	gpio_set_dir(gpio_button, 0);
	gpio_set_edge(gpio_button, "falling");  // Can be rising, falling or both
	gpio_button_fd = gpio_fd_open(gpio_button);

	// Set up analog-in
	snprintf(ainx, sizeof ainx, "ain%d", ain_pin_x);
	snprintf(ainy, sizeof ainy, "ain%d", ain_pin_y);
 
	timeout = POLL_TIMEOUT;
 
	// Set up i2c
	i2cbus = lookup_i2c_bus("3");
	printf("i2cbus = %d\n", i2cbus);
	if (i2cbus < 0)
		help();

	address = parse_i2c_address("0x70");
	printf("address = 0x%2x\n", address);
	if (address < 0)
		help();

	size = I2C_SMBUS_BYTE;

	daddress = 0x21;
	if (daddress < 0 || daddress > 0xff) {
		fprintf(stderr, "Error: Data address invalid!\n");
		help();
	}

	file = open_i2c_dev(i2cbus, filename, sizeof(filename), 0);
	printf("file = %d\n", file);
	if (file < 0
	 || check_funcs(file, size)
	 || set_slave_addr(file, address, force))
		exit(1);


		daddress = 0x21;	// Start oscillator (page 10)
		printf("writing: 0x%02x\n", daddress);
		res = i2c_smbus_write_byte(file, daddress);

		daddress = 0x81;	// Display on, blinking off (page 11)
		printf("writing: 0x%02x\n", daddress);
		res = i2c_smbus_write_byte(file, daddress);


		daddress = 0xe7;	// Full brightness (page 15)
		printf("writing: 0x%02x\n", daddress);
		res = i2c_smbus_write_byte(file, daddress);

		daddress = 0x00;	// Start writing to address 0 (page 13)
		printf("writing: 0x%02x\n", daddress);
		res = i2c_smbus_write_byte(file, daddress);

static __u16 dot_bmp[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
static __u16 smile_bmp[] = {0x3C, 0x42, 0x95, 0xA1, 0xA1, 0x95, 0x42, 0x3C};
static __u16 ya_bmp[] = {0xfc, 0x12, 0x12, 0xfd, 0x02, 0xfc, 0x02, 0x01};
//static __u16 neutral_bmp[] = {0x3C, 0x42, 0x95, 0x91, 0x91, 0x95, 0x42, 0x3C};


	while (keepgoing) {
		memset((void*)fdset, 0, sizeof(fdset));

		fdset[0].fd = gpio_button_fd;
		fdset[0].events = POLLPRI;

		analog_value_x = analog_in(ainx);
		analog_value_y = analog_in(ainy);

		
		poll(fdset, nfds, timeout);		

/*		rc = poll(fdset, nfds, timeout);      
		
		if (rc < 0) {
			printf("\npoll() failed!\n");
			return -1;
		}
      
		if (rc == 0) {
			printf(".");
		}
*/

		if(analog_value_x >= 4000 && row < 7) {
			row ++;
			dot_bmp[row] = pos;
			dot_bmp[row - 1] = 0x00;
		}
		if(analog_value_x <= 1000 && row > 0) {
			row --;
			dot_bmp[row] = pos;
			dot_bmp[row + 1] = 0x00;
		}
		if(analog_value_y >= 4000 && pos < 0x80) {
			pos = pos << 1;			
			dot_bmp[row] = pos;
		}
		if(analog_value_y <= 1000 && pos > 0x01) {
			pos = pos >> 1;			
			dot_bmp[row] = pos;
		}
		
		trace_bmp[row] = trace_bmp[row] | dot_bmp[row];
		for(i = 0; i < 8; i ++)		
			bright_bmp[i] = (bright_bmp[i] & ~trace_bmp[i]) | (hidden_bmp[i] & trace_bmp[i]) | dot_bmp[i];

/*
 * For some reason the display is rotated one column, so pre-unrotate the
 * data.
 */
		for(i = 0; i < 8; i ++)
			block[i] = (bright_bmp[i] & 0xfe) >>1 | (bright_bmp[i] & 0x01) << 7;

		res = i2c_smbus_write_i2c_block_data(file, daddress, 16, (__u8 *)block);


// button interrupt
		if (fdset[0].revents & POLLPRI) {
			lseek(fdset[0].fd, 0, SEEK_SET);  // Read from the start of the file
			len = read(fdset[0].fd, buf, MAX_BUF);						
			switch(n){
				case(0):
					for(i = 0; i < 8; i ++) {
						hidden_bmp[i] = 0x00;
						bright_bmp[i] = 0x00;
						trace_bmp[i] = 0x00;
					}	 
					break;		// Initialize in the initial interrupt.
				case(1):
					for(i = 0; i < 8; i ++) 
						block[i] = (smile_bmp[i]&0xfe) >>1 | (smile_bmp[i]&0x01) << 7;					
					res = i2c_smbus_write_i2c_block_data(file, daddress, 16, (__u8 *)block);
					sleep(1);
					for(i = 0; i < 8; i ++) {
						hidden_bmp[i] = ya_bmp[i];
						bright_bmp[i] = 0x00;
						trace_bmp[i] = 0x00;
					}	
					break;
				default:
					for(i = 0; i < 8; i ++) 
						block[i] = (smile_bmp[i]&0xfe) >>1 | (smile_bmp[i]&0x01) << 7;					
					res = i2c_smbus_write_i2c_block_data(file, daddress, 16, (__u8 *)block);
					sleep(1);
					for(i = 0; i < 8; i ++) {
						hidden_bmp[i] = ya_bmp[i];
						bright_bmp[i] = smile_bmp[i];
						trace_bmp[i] = 0x00;
					}	
					break;
			}			
			n++;
		}
		

		fflush(stdout);
	}

	gpio_fd_close(gpio_button_fd);
	return 0;


	exit(0);
}

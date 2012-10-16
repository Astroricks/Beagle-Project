

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>	// Defines signal-handling functions (i.e. trap Ctrl-C)
#include "i2c-dev.h"

 /****************************************************************
 * Constants
 ****************************************************************/
 
#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
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
 * gpio_fd_close
 ****************************************************************/

int gpio_fd_close(int fd)
{
	return close(fd);
}

/****************************************************************
* i2c_func
****************************************************************/

int i2c_func(int i2cbus, int address, int daddress)
{
	int res, size, file;
	char filename[20];

	size = I2C_SMBUS_BYTE;

	sprintf(filename, "/dev/i2c-%d", i2cbus);
	file = open(filename, O_RDWR);
	if (file<0) {
		if (errno == ENOENT) {
			fprintf(stderr, "Error: Could not open file "
				"/dev/i2c-%d: %s\n", i2cbus, strerror(ENOENT));
		} else {
			fprintf(stderr, "Error: Could not open file "
				"`%s': %s\n", filename, strerror(errno));
			if (errno == EACCES)
				fprintf(stderr, "Run as root?\n");
		}
		exit(1);
	}

	if (ioctl(file, I2C_SLAVE, address) < 0) {
		fprintf(stderr,
			"Error: Could not set address to 0x%02x: %s\n",
			address, strerror(errno));
		return -errno;
	}

	res = i2c_smbus_write_byte(file, daddress);
	if (res < 0) {
		fprintf(stderr, "Warning - write failed, filename=%s, daddress=%d\n",
			filename, daddress);
	}
	res = i2c_smbus_read_byte_data(file, daddress);
	close(file);

	if (res < 0) {
		fprintf(stderr, "Error: Read failed, res=%d\n", res);
		exit(2);
	}

	printf("0x%02x (%d)\n", res, res);

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
* set_pwm
****************************************************************/	
	int set_pwm(char* pwmNum, int periodFreq, int dutyPercent){
	FILE *fp;
	char pwmPath[MAX_BUF];

	snprintf(pwmPath, sizeof pwmPath, "/sys/class/pwm/%s/run", pwmNum);

	if((fp = fopen(pwmPath, "w")) == NULL){
	printf("Cannot open pwm run file, %s\n", pwmPath);
	return 1;
	}

	rewind(fp);	
	fprintf(fp, "1\n");
	fflush(fp);
	fclose(fp);

	snprintf(pwmPath, sizeof pwmPath, "/sys/class/pwm/%s/duty_ns", pwmNum);

	if((fp = fopen(pwmPath, "w")) == NULL){
	printf("Cannot open pwm duty_ns file, %s\n", pwmPath);
	}

	rewind(fp);
	fprintf(fp, "0\n");
	fflush(fp);	
	fclose(fp);

	snprintf(pwmPath, sizeof pwmPath, "/sys/class/pwm/%s/period_freq", pwmNum);
	
	if((fp = fopen(pwmPath, "w")) == NULL){
	printf("Cannot open pwm period_freq file, %s\n", pwmPath);
	}

	rewind(fp);
	fprintf(fp, "%d\n", periodFreq);
	fflush(fp);
	fclose(fp);

	snprintf(pwmPath, sizeof pwmPath, "/sys/class/pwm/%s/duty_percent", pwmNum);

	if((fp = fopen(pwmPath, "w")) == NULL){
	printf("Cannot open duty_percent file, %s\n", pwmPath);
	}

	rewind(fp);
	fprintf(fp, "%d\n", dutyPercent);
	fflush(fp);
	fclose(fp);
	}

/****************************************************************
* unset_pwm
****************************************************************/
	int unset_pwm(char* pwmPin){
	FILE *fp;
	char pwmPath[MAX_BUF];

	snprintf(pwmPath, sizeof pwmPath, "/sys/class/pwm/%s/run", pwmPin);

	if((fp = fopen(pwmPath, "w")) == NULL) {
	printf("Cannot open pwm run file, %s\n", pwmPath);
	return 1;
	}

	rewind(fp);
	fprintf(fp, "0\n");
	fflush(fp);
	fclose(fp);

	return 0;

	}
/****************************************************************
* set_mux_value
****************************************************************/
	int set_mux_value(char* muxPin, int value){
	FILE *fp;
	char muxPath[MAX_BUF];

	snprintf(muxPath, sizeof muxPath, "/sys/kernel/debug/omap_mux/%s", muxPin);

	if((fp = fopen(muxPath, "w")) == NULL){
	printf("Cannot open specified mux, %s\n", muxPath);
	return 1;
	}

	rewind(fp);
	fprintf(fp, "%d\n", value);
	fflush(fp);
	fclose(fp);

	}

/****************************************************************
 * Main
 ****************************************************************/
int main(int argc, char **argv, char **envp)
{
	struct pollfd fdset[1];
	int nfds = 1;
	int gpio_button_fd, gpio_led_fd, i2c_bus, i2c_address, i2c_daddress, ain_pin, timeout, rc;
	char *buf[MAX_BUF];
	char ain[MAX_BUF];
	unsigned int gpio_button, gpio_led;
	int len;
	char led_status = 0;
	float analog_value = 0, duty_cycle = 0;


	if (argc < 7) {
		printf("Usage: <gpio-button> <gpio-led> <i2c-bus> <i2c-address> <i2c-daddress> <analog-in> (Normally 7 60 3 73 0 6)\n\n");
		printf("Waits for a change\n");
		exit(-1);
	}

	// Set the signal callback for Ctrl-C
	signal(SIGINT, signal_handler);
	
	// Get the pin values
	gpio_button = atoi(argv[1]);
	gpio_led = atoi(argv[2]);
	i2c_bus = atoi(argv[3]);
	i2c_address = atoi(argv[4]);
	i2c_daddress = atoi(argv[5]);
	ain_pin = atoi(argv[6]);

	// Set the gpio_button to be input and set its working pattern	
	gpio_export(gpio_button);
	gpio_set_dir(gpio_button, 0);
	gpio_set_edge(gpio_button, "rising");  // Can be rising, falling or both
	gpio_button_fd = gpio_fd_open(gpio_button);

	// Set the gpio_led to be output
	gpio_export(gpio_led);
	gpio_set_dir(gpio_led, 1);
	gpio_led_fd = gpio_fd_open(gpio_led);
	
	// Set up analog-in
	snprintf(ain, sizeof ain, "ain%d", ain_pin);
 
	//Get the pwm up and running
	set_mux_value("gpmc_a2", 6);

	timeout = POLL_TIMEOUT;
 
	while (keepgoing) {
		memset((void*)fdset, 0, sizeof(fdset));

		fdset[0].fd = gpio_button_fd;
		fdset[0].events = POLLPRI;

		analog_value = analog_in(ain);
		
		duty_cycle = analog_value/4095 * 100;
		set_pwm("ehrpwm.1:0", 20, (int)duty_cycle);		

		rc = poll(fdset, nfds, timeout);

		if (rc < 0) {
			printf("\npoll() failed!\n");
			return -1;
		}
      
		if (rc == 0) {
			printf(".");
		}
            
		if (fdset[0].revents & POLLPRI) {
			lseek(fdset[0].fd, 0, SEEK_SET);  // Read from the start of the file
			len = read(fdset[0].fd, buf, MAX_BUF);			
			printf("\nGPIO %d interrupt occurred, value=%c, len=%d\n", gpio_button, buf[0], len);
			led_status = ~(led_status) & 1;   // Flip the led
			gpio_set_value(gpio_led, led_status);
			i2c_func(i2c_bus, i2c_address, i2c_daddress);

			printf("analog-in voltage value = %f\n", analog_value);

		}

		
		

		fflush(stdout);
	}

	gpio_fd_close(gpio_button_fd);
	gpio_fd_close(gpio_led_fd);
	unset_pwm("ehrpwm.1:0");
	set_mux_value("gpmc_a2", 7);
	return 0;
}


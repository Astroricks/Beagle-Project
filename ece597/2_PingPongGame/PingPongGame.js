var fs = require('fs');

var gpioPath = '/sys/class/gpio/';
var pinMuxPath = '/sys/kernel/debug/omap_mux/';

var led = new Array();
led[0] = 30;
led[1] = 31;
led[2] = 48;
led[3] = 5;
led[4] = 13;
led[5] = 2;


function initIO() {
	//Change the pin mux into gpio mode
	//'7' means %000111, e.g. mode 7, pull enabled, pull down, output
	fs.writeFile(pinMuxPath + 'gpmc_wait0', '7');
	fs.writeFile(pinMuxPath + 'gpmc_wpn', '7');
	fs.writeFile(pinMuxPath + 'gpmc_a0', '7');
	fs.writeFile(pinMuxPath + 'spi0_cs0', '7');
	fs.writeFile(pinMuxPath + 'uart1_rtsn', '7');
	fs.writeFile(pinMuxPath + 'spi0_d0', '7');
	
	//Export the gpios
	var i;
	for(i = 0; i < 6; i++){
		fs.writeFile(gpioPath + 'export', led[i]);
	}

	//Set the direction of the gpios as output
	for(i = 0; i < 6; i++){
		fs.writeFile(gpioPath + 'gpio' + led[i] + 'direction');
	}
}


function lightLed(ledPin){
	var ledPath = gpioPath + 'gpio' + ledPin + 'value';
	fs.writeFile(ledPath, 1);
}

function dieLed(ledPin){
	var ledPath = gpioPath + 'gpio' + ledPin + 'value';
	fs.writeFile(ledPath, 0);
}

InitIO();

lightLed(led[0]);
dieLed(led[1]);

var fs = require('fs'),
    exec = require('child_process').exec;

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
	exec("echo 7 > " + pinMuxPath + "gpmc_wait0");
	
	//Don't know why the wiretFile doesn't work	
/*	fs.writeFile(pinMuxPath + 'gpmc_wait0', '7');
	fs.writeFile(pinMuxPath + 'gpmc_wpn', '7');
	fs.writeFile(pinMuxPath + 'gpmc_a0', '7');
	fs.writeFile(pinMuxPath + 'spi0_cs0', '7');
	fs.writeFile(pinMuxPath + 'uart1_rtsn', '7');
	fs.writeFile(pinMuxPath + 'spi0_d0', '7');
*/	
	//Export the gpios
	var i;
	for(i = 0; i < 6; i++){
		fs.writeFile(gpioPath + 'export', led[i]);
	}

	//Set the direction of the gpios as output
	for(i = 0; i < 6; i++){
		fs.writeFile(gpioPath + 'gpio' + led[i] + '/direction', 'out');
	}

}


function lightLed(ledPin){
	var ledPath = gpioPath + 'gpio' + ledPin + '/value';
	fs.writeFile(ledPath, 1);
}

function dieLed(ledPin){
	var ledPath = gpioPath + 'gpio' + ledPin + '/value';
	fs.writeFile(ledPath, 0);
}

function delay(time){
	var i, j;
	for(i = 0; i < time; i++)
		for(j = 0; j < 50000; j++);
}

initIO();

lightLed(led[0]);
delay(5000);
dieLed(led[0]);
delay(5000);
lightLed(led[1]);


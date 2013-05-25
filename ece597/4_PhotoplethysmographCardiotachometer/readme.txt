**************************************************
Project 4: Photoplethysmograph Cardiotachometer
Author: Yue Zhang
**************************************************

In this laboratory experiment a heartbeat counting simulation program was designed, built, and tested on BeagleBone. Then it is connected to a PPG amplifier circuit to make a photoplethysmograph cardiotachometer.

Reading analog input: There are 8 analog input pins labeled as AIN in BeagleBone System Reference Manual. I used AIN5 in this lab. The analog input resources are located in this folder:
/sys/devices/platform/tsc
There are the various analog inputs here. But this interface starts numbering at 1 and the Table above starts at 0. So we must read ain6 to use AIN5.

Simulate with a potentiometer: I used a potentiometer to simulate the heart beat signal to verify the counting program works. Note that the analog input range on BeagleBone is 0-1.8V. Fortunately, there are two pins providing the voltages on BeagleBone. Pin34 is the analog ground, and pin32 is analog 1.8V. We could read the voltage on potentiometer through AIN5 which is pin 36. I set two different thresholds for ascending and descending to eliminate the effect of noises. The simulation worked well and the converted digital range goes from 0 to 4096.

Connecting to the circuit: I used 3.3V output on BeagleBone to drive the circuit because there would be a strong noise periodically if I use 5V dc.

Testing and debugging: When I put my finger between the LED and the phototransistor, waveforms could be observed on the oscilloscope. But it is very unstable. Sometimes the waveform vibrates severely, and sometimes the waveform vibrates in a small scale I can hardly read heartbeats, especially when my finger is cold.

When the waveform is steady, I ran the heartbeat counting program. I found the program worked better when I set the thresholds as 1800 and 2100. My heartbeat is about 80-90 beats per minute.

You can find the demo video on Youtube here:
https://www.youtube.com/watch?v=F-A7yeARWU8

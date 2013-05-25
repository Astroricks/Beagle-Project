**********************************
Project 6: Audio Effects (Failed)
Author: Yue Zhang
**********************************

In this project, a BeagleBone Audio Cape is used for audio processing. The BeagleBone Audio Cape features two standard 3.5mm audio jacks as audio input and output connectors. The audio input on cape could be connected to the audio output on computer with line-in. The audio output on cape could be connected to a speaker.

ALSA is used in this project. ALSA stands for the Advanced Linux Sound Architecture. It consists of a set of kernel drivers, an application programming interface (API) library and utility programs for supporting sound under Linux. So we don’t need to care about the hardware details by using ALSA. 
To use ALSA, on the Beagle you need to:
# opkg install alsa-dev
On the host:
$ sudo apt-get install libasound2-dev

The link below is quite useful.
http://elinux.org/EBC_Exercise_17_Using_ALSA_for_Audio_Processing
It is used for the audio processing on BeagleBoard xM. Since it is also using ALSA, we could run the program on BeagleBone with just a little modification:
1) In the file audio_thread.c, we should set the SOUND DEVICE as “plughw:0,0” instead of “plughw:1,0”, since BeagleBoard xM is using the microphone of  PS EYE as input, and we are using line-in.
2) In the file audio_input_output.c, we should comment out the code regarding snd_pcm_hw_params_set_periods. This function is used for setting periods of audio. It always gives me some errors while compiling if I don't comment it out.

After the modification, we could run the programs in lab06a_audio_record and lab06b_audio_playback. And after following the instructions on the Wiki page, we could write an audio loop through program by combining these two parts together.

To make a reverberation effect, the first thing we should do is to generate an audio delay. The method I used is used a number of buffers to record the input audio and then play them later on. 

The next thing we should do is adding the current buffer and the delayed buffer together. But unfortunately, I just got some noises. This is why the project is not succesful :(

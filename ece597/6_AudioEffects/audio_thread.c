/*
 *   audio_thread.c
 */

// Modfied for ALSA output 5-May-2011, Mark A. Yoder

// Based on Basic PCM audio (http://www.suse.de/~mana/alsa090_howto.html#sect02)http://www.suse.de/~mana/alsa090_howto.html#sect03

//* Standard Linux headers **
#include     <stdio.h>                          // Always include stdio.h
#include     <stdlib.h>                         // Always include stdlib.h
#include     <fcntl.h>                          // Defines open, read, write methods
#include     <unistd.h>                         // Defines close and sleep methods
//#include     <sys/ioctl.h>                      // Defines driver ioctl method
//#include     <linux/soundcard.h>                // Defines OSS driver functions
#include     <string.h>                         // Defines memcpy
#include     <stdint.h>				// Defines int16_t
#include     <alsa/asoundlib.h>			// ALSA includes

//* Application headers **
#include     "debug.h"                          // DBG and ERR macros
#include     "audio_thread.h"                   // Audio thread definitions
#include     "audio_input_output.h"             // Audio driver input and output functions

/* Input audio file */
#define     INPUTFILE        "/tmp/audio.raw"

// ALSA device
#define     SOUND_DEVICE     "plughw:0,0"
#define     SOUND_DEVICE_OUT     "plughw:0,0"

//* The sample rate of the audio codec **
#define     SAMPLE_RATE      48000

//* The gain (0-100) of the left channel **
#define     LEFT_GAIN        100

//* The gain (0-100) of the right channel **
#define     RIGHT_GAIN       100

//*  Parameters for audio thread execution **
#define     BLOCKSIZE        48000

//* Number of buffers used
#define	    BUFFERNUM	     50


//*******************************************************************************
//*  audio_thread_fxn                                                          **
//*******************************************************************************
//*  Input Parameters:                                                         **
//*      void *envByRef    --  a pointer to an audio_thread_env structure      **
//*                            as defined in audio_thread.h                    **
//*                                                                            **
//*          envByRef.quit -- when quit != 0, thread will cleanup and exit     **
//*                                                                            **
//*  Return Value:                                                             **
//*      void *            --  AUDIO_THREAD_SUCCESS or AUDIO_THREAD_FAILURE as **
//*                            defined in audio_thread.h                       **
//*******************************************************************************
void *audio_thread_fxn( void *envByRef )
{

// Variables and definitions
// *************************

    // Thread parameters and return value
    audio_thread_env * envPtr = envByRef;                  // < see above >
    void             * status = AUDIO_THREAD_SUCCESS;      // < see above >

    // The levels of initialization for initMask
    #define     INPUT_FILE_OPENED           0x1
    #define     OUTPUT_ALSA_INITIALIZED     0x4
    #define     OUTPUT_BUFFER_ALLOCATED     0x8
    #define     INPUT_ALSA_INITIALIZED      0x10

            unsigned  int   initMask =  0x0;	// Used to only cleanup items that were init'd

    // Input and output driver variables
    snd_pcm_t	*pcm_output_handle;		// Handle for the PCM device
    snd_pcm_t   *pcm_capture_handle;
    snd_pcm_uframes_t exact_bufsize;		// bufsize is in frames.  Each frame is 4 bytes

    int   blksize = BLOCKSIZE;			// Raw input or output frame size in bytes
//    char *outputBuffer = NULL;			// Output buffer for driver to read from
    char *outputBuffer[BUFFERNUM];
    char *voidBuffer = NULL;
    int k;


// Thread Create Phase -- secure and initialize resources
// ******************************************************


for(k=0;k<BUFFERNUM;k++)
{
	outputBuffer[k] = NULL;
}	
    
    // Initialize the output ALSA device
    DBG( "pcm_output_handle before audio_output_setup = %d\n", (int) pcm_output_handle);
    exact_bufsize = blksize/BYTESPERFRAME;
    DBG( "Requesting bufsize = %d\n", (int) exact_bufsize);
    if( audio_io_setup( &pcm_output_handle, SOUND_DEVICE_OUT, SAMPLE_RATE, 
			SND_PCM_STREAM_PLAYBACK, &exact_bufsize) == AUDIO_FAILURE )
    {
        ERR( "audio_output_setup failed in audio_thread_fxn\n" );
        status = AUDIO_THREAD_FAILURE;
        goto  cleanup ;
    }
	

    //Initialize for the input ALSA device
    if(audio_io_setup( &pcm_capture_handle, SOUND_DEVICE, SAMPLE_RATE,
                       SND_PCM_STREAM_CAPTURE, &exact_bufsize) == AUDIO_FAILURE)
    {
        ERR( "audio_input_setup failed in audio_thread_fxn\n" );
        status = AUDIO_THREAD_FAILURE;
        goto  cleanup ;
    }
    DBG( "pcm_output_handle after audio_output_setup = %d\n", (int) pcm_output_handle);
    DBG( "blksize = %d, exact_bufsize = %d\n", blksize, (int) exact_bufsize);

    blksize = exact_bufsize;
    // Record that input ALSA device was opened in initialization bitmask
    initMask |= OUTPUT_ALSA_INITIALIZED;
    initMask |= INPUT_ALSA_INITIALIZED;

for(k=0;k<BUFFERNUM;k++)
{
    // Create output buffer to write from into ALSA output device
    if( ( outputBuffer[k] = malloc( blksize ) ) == NULL )
    {
        ERR( "Failed to allocate memory for output block (%d)\n", blksize );
        status = AUDIO_THREAD_FAILURE;
        goto  cleanup ;
    }

    DBG( "Allocated output audio buffer of size %d to address %p\n", blksize, outputBuffer[k] );
}

    if( ( voidBuffer = malloc( blksize ) ) == NULL )
    {
        ERR( "Failed to allocate memory for void buffer (%d)\n", blksize );
        status = AUDIO_THREAD_FAILURE;
        goto  cleanup ;
    }

    // Record that the output buffer was allocated in initialization bitmask
    initMask |= OUTPUT_BUFFER_ALLOCATED;


// Thread Execute Phase -- perform I/O and processing
// **************************************************
    // Get things started by sending some silent buffers out.
    int i;
    for(k=0;k<BUFFERNUM;k++)
    {
        memset(outputBuffer[k], 0, blksize);		// Clear the buffer
    }
    memset(voidBuffer, 0, blksize);
    for(i=0; i<2; i++) {
	if ((snd_pcm_writei(pcm_output_handle, outputBuffer[0],
		exact_bufsize)) < 0) {
	    snd_pcm_prepare(pcm_output_handle);
	    ERR( "<<<Pre Buffer Underrun >>> \n");
	}
    }


//
// Processing loop
//
    DBG( "Entering audio_thread_fxn processing loop\n" );

    int count = 0;
    int d = 0;
//    while( !envPtr->quit )

    //Record the audio into the buffers for playing out later
    for(k=1;k<BUFFERNUM;k++)
    {
        if (snd_pcm_readi(pcm_capture_handle, outputBuffer[k], blksize/BYTESPERFRAME) < 0)
	{
	    snd_pcm_prepare(pcm_capture_handle);
            ERR( "<<<<<<<<<<<<<<< Buffer Overrun1 >>>>>>>>>>>>>>>\n");
            status = AUDIO_THREAD_FAILURE;
            goto cleanup;
	}
	//Send void buffers to output to avoid buffer underrun
	if ((snd_pcm_writei(pcm_output_handle, voidBuffer,blksize/BYTESPERFRAME)) < 0) {
	    snd_pcm_prepare(pcm_output_handle);
	    ERR( "<<<Void Buffer Underrun >>> \n");
	}

    }

    //Output the processed audio
    for(k=0;k<BUFFERNUM;k++)
    {
	//Read the playing audio into buffer
	if (snd_pcm_readi(pcm_capture_handle, outputBuffer[k], blksize/BYTESPERFRAME) < 0)
	{
	    snd_pcm_prepare(pcm_capture_handle);
            ERR( "<<<<<<<<<<<<<<< Buffer Overrun2 >>>>>>>>>>>>>>>\n");
            status = AUDIO_THREAD_FAILURE;
            goto cleanup;
	}

	//outputBuffer[k+1] is the delayed buffer
	d = k+1;
	if(d>=BUFFERNUM)	
		d = 0;

	//Add the current buffer and delayed buffer together to make a reverbration effect. Just got some noise though.
	for(i=0;i<blksize;i=i+2){
		int16_t s1 = outputBuffer[k][i]<<8 | (unsigned char)outputBuffer[k][i+1];
		int16_t s2 = outputBuffer[d][i]<<8 | (unsigned char)outputBuffer[d][i+1];
		s1 = s1+s2*3/4;
		char r1 = (s1>>8) & 0xff;
		char r2 = s1 & 0xff;
		outputBuffer[k][i] = r1;
		outputBuffer[k][i+1] = r2;
	}

	DBG("%X ", outputBuffer[k][0]);

        // Write output buffer into ALSA output device
        while (snd_pcm_writei(pcm_output_handle, outputBuffer[k], blksize/BYTESPERFRAME) < 0) 
        {
            snd_pcm_prepare(pcm_output_handle);
            ERR( "<<<<<<<<<<<<<<< Buffer Underrun %d >>>>>>>>>>>>>>>\n", k);
            status = AUDIO_THREAD_FAILURE;
            goto cleanup;
        }

	DBG("%d, ", count++);
//	DBG("%X%X %X%X, %X%X %X%X\n", outputBuffer[k][0], outputBuffer[k][1], outputBuffer[k][2], outputBuffer[k][3], outputBuffer[k][4], outputBuffer[k][5], outputBuffer[k][6], outputBuffer[k][7]);
//	DBG("\n");
    }
    DBG("\n");

    DBG( "Exited audio_thread_fxn processing loop\n" );


// Thread Delete Phase -- free up resources allocated by this file
// ***************************************************************

cleanup:

    DBG( "Starting audio thread cleanup to return resources to system\n" );

    // Close the audio drivers
    // ***********************
    //  - Uses the initMask to only free resources that were allocated.
    //  - Nothing to be done for mixer device, as it was closed after init.

    // Close output ALSA device
    if( initMask & OUTPUT_ALSA_INITIALIZED )
        if( audio_io_cleanup( pcm_output_handle ) != AUDIO_SUCCESS )
        {
            ERR( "audio_output_cleanup() failed for file descriptor %d\n", (int)pcm_output_handle );
            status = AUDIO_THREAD_FAILURE;
        }

    // Free allocated buffers
    // **********************

    // Free output buffer
    if( initMask & OUTPUT_BUFFER_ALLOCATED )
    {
	for(k=0;k<BUFFERNUM;k++)
   	{
        	free( outputBuffer[k] );
        	DBG( "Freed audio output buffer at location %p\n", outputBuffer[k] );
    	}
    }

    // Return from audio_thread_fxn function
    // *************************************
	
    // Return the status at exit of the thread's execution
    DBG( "Audio thread cleanup complete. Exiting audio_thread_fxn\n" );
    return status;
}


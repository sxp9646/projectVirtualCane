#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "openal_gyro.h"
//#include "efx.h"
//#include "xram.h"

#define	TEST_WAVE_FILE		"water.wav"
#define NUM_BUFFERS (1)
#define PI (3.14)

#define TRUE 1
#define FALSE 0

void loadWAVFile(char *filename,  ALenum *format, ALvoid **data, ALsizei *size, ALsizei *freq);

// WAVE file header format
struct HEADER
{
	unsigned char riff[4];						// RIFF string
	unsigned int overall_size;				    // overall size of file in bytes
	unsigned char wave[4];						// WAVE string
	unsigned char fmt_chunk_marker[4];			// fmt string with trailing null char
	unsigned int length_of_fmt;					// length of the format data
	unsigned int format_type;					// format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
	unsigned int channels;						// no.of channels
	unsigned int sample_rate;					// sampling rate (blocks per second)
	unsigned int byterate;						// SampleRate * NumChannels * BitsPerSample/8
	unsigned int block_align;					// NumChannels * BitsPerSample/8
	unsigned int bits_per_sample;				// bits per sample, 8- 8bits, 16- 16 bits etc
	unsigned char data_chunk_header [4];		// DATA string or FLLR string
	unsigned int data_size;						// NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read
};

ALCdevice   *Device; 
ALCcontext  *Context;
ALenum error;
ALboolean g_bEAX;
ALuint g_Buffers[NUM_BUFFERS];
   
ALenum format;

// Pointer to audio data    
ALvoid *data;
    
// size of audio data
ALsizei size;
    
// Frequency of audio data
ALsizei freq;
ALuint source;
ALint state;
/*
int main()
{
    char buff[1000];
    init();
    printf("press something\n");
    scanf("%s",buff );
    turn(0.0);
    printf("press something\n");
    scanf("%s",buff );
    end();

}
*/
int init()
{
// Initialization
    char fname[1024] = TEST_WAVE_FILE;

    //printf("Name of .wav file?\n");
    //scanf("%s", &fname);

    Device = alcOpenDevice(NULL); // select the "preferred device"
    if (Device) {
        Context = alcCreateContext(Device,NULL);
        alcMakeContextCurrent(Context);
    }
    // Check for EAX 2.0 support
    g_bEAX = alIsExtensionPresent("EAX2.0");
    // Generate Buffers
    alGetError(); // clear error code
    alGenBuffers(NUM_BUFFERS, g_Buffers);
    if ((error = alGetError()) != AL_NO_ERROR)
    {
        //DisplayALError("alGenBuffers :", error);
        return -1;
    }
    // Load test.wav
    // YOU GOTTA PROGRAM THIS YOURSELF YOU IDIOT
    loadWAVFile(fname,&format,&data,&size,&freq);

    if ((error = alGetError()) != AL_NO_ERROR)
    {
        //DisplayALError("alutLoadWAVFile test.wav : ", error);
        alDeleteBuffers(NUM_BUFFERS, g_Buffers);
        return -1;
    }
    
    // Copy test.wav data into AL Buffer 0
    // How big should this stupid thing be? g_Buffers[0]? 
    // Should it be big enough to hold the whole .wav file?
    alBufferData(g_Buffers[0],format,data,size,freq);
    if ((error = alGetError()) != AL_NO_ERROR)
    {
        //DisplayALError("alBufferData buffer 0 : ", error);
        alDeleteBuffers(NUM_BUFFERS, g_Buffers);
        return -1;
    }
    
    // Unload test.wav
    // WHY?
    //unloadWAV(format,data,size,freq);
    if ((error = alGetError()) != AL_NO_ERROR)
    {
        //DisplayALError("alutUnloadWAV : ", error);
        alDeleteBuffers(NUM_BUFFERS, g_Buffers);
        return -1;
    }
    // Generate Sources
    alGenSources(1, &source);
    if ((error = alGetError()) != AL_NO_ERROR)
    {
        //DisplayALError("alGenSources 1 : ", error);
        return -1;
    }
    // Attach buffer 0 to source
    alSourcei(source, AL_BUFFER, g_Buffers[0]);
    if ((error = alGetError()) != AL_NO_ERROR)
    {
        //DisplayALError("alSourcei AL_BUFFER 0 : ", error);
    }
    
    // Wait for the song to complete
    //printf("playing sound?\n");
    ALfloat pos[3] = {0};
    pos[2] = 2.0;
    ALfloat orient[6] = {0,0, 1, 0,1,0}; // AT, AT, AT, UP, UP, UP ?

    alSourcefv(source, AL_POSITION, pos);
    pos[2] = 0.0;
    alListenerfv(AL_POSITION,pos);

    return 0;
}

void play()
{
    alSourcei(source,AL_LOOPING,AL_TRUE); 
    alSourcePlay(source);
}
void stop()
{
	alSourceStop(source);
	alGetSourcei(source, AL_SOURCE_STATE, &state);
	while(state == AL_PLAYING)
	{
		alGetSourcei(source, AL_SOURCE_STATE, &state);
	}
}

void turn(double degree)
{
    Device = alcGetContextsDevice(Context);
    if (Device == NULL) {
    	Device = alcOpenDevice(NULL); // select the "preferred device"
        Context = alcCreateContext(Device,NULL);
        alcMakeContextCurrent(Context);
    }
    ALfloat orient[6] = { 0, 0, 1, 0,1,0}; // AT, AT, AT, UP, UP, UP ?

	// X rotation
    orient[0] = -1*sin(degree * (PI / 180.0));

	// Z rotation
    orient[2] = cos(degree * (PI / 180.0));


    //printf("Degree: %.2f\n", degree);
    printf("at x: %.2f\tat y: %.2f\tat z: %.2f\n\n", orient[0], orient[1], orient[2]);

    alListenerfv(AL_ORIENTATION,orient); 
}

void end()
{
    printf("done?\n");

    // Exit
    Context=alcGetCurrentContext();
    Device=alcGetContextsDevice(Context);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(Context);
    alcCloseDevice(Device); 

}

void loadWAVFile(char *filename,  ALenum *format, ALvoid **data, ALsizei *size, ALsizei *freq)
{
    unsigned char buffer4[4];
    unsigned char buffer2[2];
    FILE *f_ptr;
    struct HEADER header;


    // open file
    //printf("Opening  file..\n");
    f_ptr = fopen(filename, "rb");
    if (f_ptr == NULL) 
    {
        printf("Error opening file\n");
        exit(1);
    }

    int read = 0;

    // read header parts

    read = fread(header.riff, sizeof(header.riff), 1, f_ptr);
    printf("(1-4): %s \n", header.riff); 

    read = fread(buffer4, sizeof(buffer4), 1, f_ptr);
    //printf("%u %u %u %u\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]);

    // convert little endian to big endian 4 byte int
    header.overall_size  = buffer4[0] | 
                        (buffer4[1]<<8) | 
                        (buffer4[2]<<16) | 
                        (buffer4[3]<<24);

    printf("(5-8) Overall size: bytes:%u, Kb:%u \n", header.overall_size, header.overall_size/1024);

    read = fread(header.wave, sizeof(header.wave), 1, f_ptr);
    printf("(9-12) Wave marker: %s\n", header.wave);

    read = fread(header.fmt_chunk_marker, sizeof(header.fmt_chunk_marker), 1, f_ptr);
    printf("(13-16) Fmt marker: %s\n", header.fmt_chunk_marker);

    read = fread(buffer4, sizeof(buffer4), 1, f_ptr);
    //printf("%u %u %u %u\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]);

    // convert little endian to big endian 4 byte integer
    header.length_of_fmt = buffer4[0] |
                            (buffer4[1] << 8) |
                            (buffer4[2] << 16) |
                            (buffer4[3] << 24);
    //printf("(17-20) Length of Fmt header: %u \n", header.length_of_fmt);

    read = fread(buffer2, sizeof(buffer2), 1, f_ptr); 
    //printf("%u %u \n", buffer2[0], buffer2[1]);

    header.format_type = buffer2[0] | (buffer2[1] << 8);
    char format_name[10] = "";
    if (header.format_type == 1)
    {
        strcpy(format_name,"PCM"); 
    }
    else if (header.format_type == 6)
    {
        strcpy(format_name, "A-law");
    }
    else if (header.format_type == 7)
    {
        strcpy(format_name, "Mu-law");
    }

    printf("(21-22) Format type: %u %s \n", header.format_type, format_name);

    read = fread(buffer2, sizeof(buffer2), 1, f_ptr);
    //printf("%u %u \n", buffer2[0], buffer2[1]);

    header.channels = buffer2[0] | (buffer2[1] << 8);
    printf("(23-24) Channels: %u \n", header.channels);

    read = fread(buffer4, sizeof(buffer4), 1, f_ptr);
    //printf("%u %u %u %u\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]);

    header.sample_rate = buffer4[0] |
                        (buffer4[1] << 8) |
                        (buffer4[2] << 16) |
                        (buffer4[3] << 24);

    // ASSIGN THE FREQUENCY HERE:
    printf("(25-28) Sample rate: %u\n", header.sample_rate);

    read = fread(buffer4, sizeof(buffer4), 1, f_ptr);
    //printf("%u %u %u %u\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]);

    header.byterate  = buffer4[0] |
                        (buffer4[1] << 8) |
                        (buffer4[2] << 16) |
                        (buffer4[3] << 24);
    //printf("(29-32) Byte Rate: %u , Bit Rate:%u\n", header.byterate, header.byterate*8);
    *freq = header.sample_rate;

    read = fread(buffer2, sizeof(buffer2), 1, f_ptr);
    //printf("%u %u \n", buffer2[0], buffer2[1]);

    header.block_align = buffer2[0] |
                    (buffer2[1] << 8);
    printf("(33-34) Block Alignment: %u \n", header.block_align);

    read = fread(buffer2, sizeof(buffer2), 1, f_ptr);
    //printf("%u %u \n", buffer2[0], buffer2[1]);

    header.bits_per_sample = buffer2[0] |
                    (buffer2[1] << 8);
    printf("(35-36) Bits per sample: %u \n", header.bits_per_sample);

    read = fread(header.data_chunk_header, sizeof(header.data_chunk_header), 1, f_ptr);
    printf("(37-40) Data Marker: %s \n", header.data_chunk_header);

    read = fread(buffer4, sizeof(buffer4), 1, f_ptr);
    //printf("%u %u %u %u\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]);

    header.data_size = buffer4[0] |
                (buffer4[1] << 8) |
                (buffer4[2] << 16) | 
                (buffer4[3] << 24 );
    printf("(41-44) Size of data chunk: %u \n", header.data_size);


    // calculate no.of samples
    long num_samples = (8 * header.data_size) / (header.channels * header.bits_per_sample);
    printf("Number of samples:%lu \n", num_samples);

    long size_of_each_sample = (header.channels * header.bits_per_sample) / 8;
    printf("Size of each sample:%ld bytes\n", size_of_each_sample);
    switch(size_of_each_sample)
    {
        case 1:
            if(header.channels == 1)
            {
                *format = AL_FORMAT_MONO8;
            }
            else
            {
                *format = AL_FORMAT_STEREO8;
            }
            break;
        case 2:
            if(header.channels == 1)
            {
                *format = AL_FORMAT_MONO16;
            }
            else
            {
                *format = AL_FORMAT_STEREO16;
            }
            break;
    }

    printf("Mallocing %ld bytes for sound\n", size_of_each_sample * num_samples);
    char *sound_data = malloc(size_of_each_sample * num_samples);
    *size = size_of_each_sample * num_samples;
    if(sound_data == NULL)
    {
        printf("Error - could not allocate memory i guess\n");
        exit(1);
    }
    *data = (ALvoid *) sound_data;
    // read each sample from data chunk if PCM
    if (header.format_type == 1)
    { // PCM
        long i =0;
        char data_buffer[size_of_each_sample ];
        int  size_is_correct = TRUE;

        // make sure that the bytes-per-sample is completely divisible by num.of channels
        long bytes_in_each_channel = (size_of_each_sample / header.channels);
        if ((bytes_in_each_channel  * header.channels) != size_of_each_sample) 
        {
            printf("Error: %ld x %ud <> %ld\n", bytes_in_each_channel, header.channels, size_of_each_sample);
            size_is_correct = FALSE;
        }

        if (size_is_correct)
        { 
            // the valid amplitude range for values based on the bits per sample
            long low_limit = 0l;
            long high_limit = 0l;

            switch (header.bits_per_sample) 
            {
                case 8:
                    low_limit = -128;
                    high_limit = 127;
                    break;
                case 16:
                    low_limit = -32768;
                    high_limit = 32767;
                    break;
                case 32:
                    low_limit = -2147483648;
                    high_limit = 2147483647;
                    break;
            }					

            //printf("\n\n.Valid range for data values : %ld to %ld \n", low_limit, high_limit);
            for (i =0; i < num_samples; i++) 
            {
                //printf("==========Sample %ld / %ld=============\n", i, num_samples);
                read = fread(data_buffer, sizeof(data_buffer), 1, f_ptr);
                if (read == 1)
                {
                    for(int k = sizeof(data_buffer); k >= 0 ; k--)
                    //for(int k = 0; k < sizeof(data_buffer); k++)
                    {
                        //sound_data[i*sizeof(data_buffer) + sizeof(data_buffer) - k] = data_buffer[k];
                        sound_data[i*sizeof(data_buffer) + k] = data_buffer[k];
                    }
                    /*
                    // dump the data read
                    unsigned int  xchannels = 0;
                    int data_in_channel = 0;

                    // STB:  THIS FUNCTION IS JUST STRAIGHT UP WRONG.  SERIOUSLY
                    for (xchannels = 0; xchannels < header.channels; xchannels ++ ) 
                    {
                        printf("Channel#%d : ", (xchannels+1));
                        // convert data from little endian to big endian based on bytes in each channel sample
                        if (bytes_in_each_channel == 4) 
                        {
                            data_in_channel =	data_buffer[0] | 
                                                (data_buffer[1]<<8) | 
                                                (data_buffer[2]<<16) | 
                                                (data_buffer[3]<<24);
                        }
                        else if (bytes_in_each_channel == 2) 
                        {
                            data_in_channel = data_buffer[0] |
                                                (data_buffer[1] << 8);
                        }
                        else if (bytes_in_each_channel == 1) 
                        {
                            data_in_channel = data_buffer[0];
                        }

                        //printf("%d ", data_in_channel);

                        // check if value was in range
                        if (data_in_channel < low_limit || data_in_channel > high_limit)
                            printf("**value out of range\n");

                        //printf(" | ");
                    }
                    */
                    //printf("\n");
                }
                else 
                {
                    char *temp = realloc(sound_data, (i) * size_of_each_sample);
                    if(temp != NULL)
                    {
                        printf("Rearranging sound data?\n");
                        sound_data = temp;
                        *data = (ALvoid *) sound_data;
                        *size = (i) * size_of_each_sample;
                    }
                    printf("Error reading file. %d bytes\n", read);
                    printf("%d bytes read, %d bytes remained\n", (i+1), (num_samples - i));
                    break;
                }

            } // 	for (i =1; i <= num_samples; i++) {

        } // 	if (size_is_correct) { 
    } //  if (header.format_type == 1) { 

    //printf("Closing file..\n");
    fclose(f_ptr);
    return;
}

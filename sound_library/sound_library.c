#include <string.h>
#include <stdio.h>
#include <stdlib.h>
//#include <math.h>
#include "sound_library.h"

// This tells the program WHERE TO LOOK FOR THE SOUNDS. Currently its' the current directory
// Currently not used, but will be soonTM
#define BASE_SOUND_PATH		"/home/pi/projectDaredevil/sound/"

#define DEBUG 0

// General OPENAL stuff:
ALCdevice   *Device  = NULL; 
ALCcontext  *Context = NULL;

// REVERB stuff:
ALuint *uiEffectSlot = NULL;
ALuint *uiEffect = NULL;

// Helper function to load a ".wav" file out into memory
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

//Constructor functions:
// Initializes OPENAL global variables
void SL_Init()
{
	ALenum error;
    Device = alcOpenDevice(NULL); // select the "preferred device"
    if (Device) 
	{
        Context = alcCreateContext(Device, NULL);
		error = alGetError();
		if(error != AL_NO_ERROR)
		{
			printf("there was problem generating a context\n");
		}
		
        alcMakeContextCurrent(Context);
		error = alGetError();
		if(error != AL_NO_ERROR)
		{
			printf("there was problem |:\n");
		}
		// By default, make sure the user is at '0,0,0'
		SL_MoveUser(0.0, 0.0, 0.0);

		// By default, make the user face down the -Z axis
		// with the top of their head pointing up the Y axis
		SL_TurnUser(0.0, 0.0, -1.0, 
					0.0, 1.0, 0.0);
    }
	else
	{
		printf("CRITICAL ERROR: COULD NOT FIND DEVICE\n");
		exit(-1);
	}
}

void SL_InitSource(SL_Sound *src)
{
	ALenum error;
	// This should never happen.  If it does, nuke everything
	if(src == NULL)
	{
		printf("ERROR: Sound_Source pointer is uninitialized!\n");
		exit(-1);
	}
	
    // Generate Sources
    alGenSources(1, &src->sourceID);
    if ((error = alGetError()) != AL_NO_ERROR)
    {
        //DisplayALError("alGenSources 1 : ", error);
        return;
    }
    if(uiEffectSlot != NULL && uiEffect != NULL)
    {
        alSource3i(src->sourceID, AL_AUXILIARY_SEND_FILTER, *uiEffectSlot, 0, AL_FILTER_NULL);
        if ((error = alGetError()) != AL_NO_ERROR)
        {
            printf("Could not attach effects to sound\n");
        }
    }
}

// Loads a sound into a source
void SL_LoadSound(SL_Sound *src, char *soundfile)
{
	// Used for sound data extraction
    ALenum format;
    ALvoid *data;
    ALsizei size;
    ALsizei freq;

    int full_path_size = strlen(soundfile) + strlen(BASE_SOUND_PATH);
    char* full_path = (char *) malloc(full_path_size + 1);
    strcpy(full_path, BASE_SOUND_PATH);
    strcat(full_path, soundfile);
	
	// For retrieving errors
	ALenum error;
	
    alGenBuffers(1, &src->bufferID);
    if ((error = alGetError()) != AL_NO_ERROR)
    {
        printf("there was a problem :'(\n");
		return;
    }
    // Load test.wav
    loadWAVFile(full_path,&format,&data,&size,&freq);
    free(full_path);

    if ((error = alGetError()) != AL_NO_ERROR)
    {
        alDeleteBuffers(1, &src->bufferID);
        printf("there was a problem :'(\n");
		return;
    }
    
    // Copy test.wav data into OpenAL Buffer
    // Should it be big enough to hold the whole .wav file
    alBufferData(src->bufferID,format,data,size,freq);
	
	// IS THIS VALID??
	free(data);
    if ((error = alGetError()) != AL_NO_ERROR)
    {
        alDeleteBuffers(1, &src->bufferID);
        printf("there was a problem :'(\n");
		return;
    }
    
    // Attach buffer to source
    alSourcei(src->sourceID, AL_BUFFER, src->bufferID);
    if ((error = alGetError()) != AL_NO_ERROR)
    {
        printf("there was a problem :'(\n");
		return;
    }
}

// General Access Functions
// These should be the only functions necessary for the project?

// Move the source to its' currently stored x,y,z location
// Call this after updating the source's internal location
void SL_PlaceSound(SL_Sound *src)
{
    ALfloat pos[3] = {0.0, 0.0, 0.0};
	ALenum error;

	pos[0] = src->x;
	pos[1] = src->y;
	pos[2] = src->z;
	alSourcefv(src->sourceID, AL_POSITION, pos);
    if ((error = alGetError()) != AL_NO_ERROR)
    {
		printf("there was a problem :'(\n");
    }
}
void SL_PlaySound(SL_Sound *src, ALint play, ALint loop)
{
	ALenum error;
	ALint state;
	if(play == AL_TRUE)
	{
		alSourcei(src->sourceID, AL_LOOPING,loop);
        alGetSourcei(src->sourceID, AL_SOURCE_STATE, &state);
		if(state != AL_PLAYING)
		{
			alSourcePlay(src->sourceID);
		}
	}
	else if(play == AL_FALSE)
	{
		alSourceStop(src->sourceID);
	}
    if ((error = alGetError()) != AL_NO_ERROR)
    {
		printf("there was a problem :'(\n");
    }
}

void SL_TurnUser(ALfloat at_x, ALfloat at_y, ALfloat at_z,
				 ALfloat up_x, ALfloat up_y, ALfloat up_z)
{
    ALfloat orient[6] = {0.0}; // AT, AT, AT, UP, UP, UP
	ALenum error;
	orient[0] = at_x;
	orient[1] = at_y;
	orient[2] = at_z;
	orient[3] = up_x;
	orient[4] = up_y;
	orient[5] = up_z;
	alListenerfv(AL_ORIENTATION,orient);
    if ((error = alGetError()) != AL_NO_ERROR)
    {
		printf("there was a problem :'(\n");
    }
}
void SL_MoveUser(ALfloat pos_x, ALfloat pos_y, ALfloat pos_z)
{
    ALfloat pos[3] = {0.0, 0.0, 0.0};
	ALenum error;

	pos[0] = pos_x;
	pos[1] = pos_y;
	pos[2] = pos_z;
	alListenerfv(AL_POSITION,pos);
    if ((error = alGetError()) != AL_NO_ERROR)
    {
		printf("there was a problem :'(\n");
    }
}

// Updates the user's position and orientation
// based on the information stored in the SL_Listener structure
void SL_UpdateUser(SL_Listener *user)
{
    ALfloat orient[6] = {0.0}; // AT, AT, AT, UP, UP, UP
    ALfloat pos[3] = {0.0};
	ALenum error;
	
	pos[0] = user->pos_x;
	pos[1] = user->pos_y;
	pos[2] = user->pos_z;
	
	orient[0] = user->at_x;
	orient[1] = user->at_y;
	orient[2] = user->at_z;
	orient[3] = user->up_x;
	orient[4] = user->up_y;
	orient[5] = user->up_z;

	alListenerfv(AL_POSITION,pos);
	alListenerfv(AL_ORIENTATION,orient);
    if ((error = alGetError()) != AL_NO_ERROR)
    {
		printf("there was a problem :'(\n");
    }
}

// Two destructor functions
void SL_FreeSound(SL_Sound *src)
{
	ALenum error;

	// Detach buffer from source
	alSourcei(src->sourceID, AL_BUFFER, 0);
	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		printf("Error detaching buffer from sound source\n");
	}

	alDeleteBuffers(1, &src->bufferID);
	error = alGetError();
    if (error != AL_NO_ERROR)
    {
		if(error == AL_INVALID_OPERATION)
		{
			printf("The buffer is still in use and can not be deleted");
		}
		else if(error == AL_INVALID_NAME)
		{
			printf("Buffer ID is invalid\n");
		}
		else if(error = AL_INVALID_VALUE)
		{
			printf("Requested number of buffers cannot be deleted\n");
		}
		else
		{
			printf("Could not successfully delete buffer\n");
		}
    }

    // Remove effects from sound source
    alSource3i(src->sourceID, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, 0, AL_FILTER_NULL);

	alDeleteSources(1, &src->sourceID);
	error = alGetError();
    if (error != AL_NO_ERROR)
    {
		if(error == AL_INVALID_NAME)
		{
			printf("Source is not valid\n");
		}
		else if(error == AL_INVALID_OPERATION)
		{
			printf("No current context is specified\n");
		}
		else
		{
			printf("Could not successfully delete source\n");
		}
    }
}
void SL_Uninit()
{
    Context=alcGetCurrentContext();
    Device=alcGetContextsDevice(Context);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(Context);
    alcCloseDevice(Device); 
}

/* 
*  This code is partially lifted from the internet and repurposed to suit our needs.
*  It was somewhere on stack exchange, as you could probably imagine.
*  It originally printed out the contents of a sound wave file.  I made it put the stuff into
*  the appropriate variables for OpenAL.
*/
void loadWAVFile(char *filename,  ALenum *format, ALvoid **data, ALsizei *size, ALsizei *freq)
{
    unsigned char buffer4[4];
    unsigned char buffer2[2];
    FILE *f_ptr;
    struct HEADER header;

    // open file
    f_ptr = fopen(filename, "rb");
    if (f_ptr == NULL) 
    {
        printf("Error opening file\n");
        exit(1);
    }

    int read = 0;

    // read header parts
    read = fread(header.riff, sizeof(header.riff), 1, f_ptr);
    read = fread(buffer4, sizeof(buffer4), 1, f_ptr);

    // convert little endian to big endian 4 byte int
    header.overall_size  = buffer4[0] | 
                        (buffer4[1]<<8) | 
                        (buffer4[2]<<16) | 
                        (buffer4[3]<<24);

    read = fread(header.wave, sizeof(header.wave), 1, f_ptr);
    read = fread(header.fmt_chunk_marker, sizeof(header.fmt_chunk_marker), 1, f_ptr);
    read = fread(buffer4, sizeof(buffer4), 1, f_ptr);

    // convert little endian to big endian 4 byte integer
    header.length_of_fmt = buffer4[0] |
                            (buffer4[1] << 8) |
                            (buffer4[2] << 16) |
                            (buffer4[3] << 24);

    read = fread(buffer2, sizeof(buffer2), 1, f_ptr); 

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

    read = fread(buffer2, sizeof(buffer2), 1, f_ptr);

    header.channels = buffer2[0] | (buffer2[1] << 8);
    read = fread(buffer4, sizeof(buffer4), 1, f_ptr);

    header.sample_rate = buffer4[0] |
                        (buffer4[1] << 8) |
                        (buffer4[2] << 16) |
                        (buffer4[3] << 24);

    read = fread(buffer4, sizeof(buffer4), 1, f_ptr);

    header.byterate  = buffer4[0] |
                        (buffer4[1] << 8) |
                        (buffer4[2] << 16) |
                        (buffer4[3] << 24);
    *freq = header.sample_rate;

    read = fread(buffer2, sizeof(buffer2), 1, f_ptr);

    header.block_align = buffer2[0] |
                    (buffer2[1] << 8);
    read = fread(buffer2, sizeof(buffer2), 1, f_ptr);

    header.bits_per_sample = buffer2[0] |
                    (buffer2[1] << 8);

    read = fread(header.data_chunk_header, sizeof(header.data_chunk_header), 1, f_ptr);
    read = fread(buffer4, sizeof(buffer4), 1, f_ptr);

    header.data_size = buffer4[0] |
                (buffer4[1] << 8) |
                (buffer4[2] << 16) | 
                (buffer4[3] << 24 );


    // calculate no.of samples
    long num_samples = (8 * header.data_size) / (header.channels * header.bits_per_sample);
    long size_of_each_sample = (header.channels * header.bits_per_sample) / 8;

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
	
	if(DEBUG)
	{
		printf("(1-4): %s \n", header.riff); 
		printf("(5-8) Overall size: bytes:%u, Kb:%u \n", header.overall_size, header.overall_size/1024);
		printf("(9-12) Wave marker: %s\n", header.wave);
		printf("(13-16) Fmt marker: %s\n", header.fmt_chunk_marker);
		printf("(17-20) Length of Fmt header: %u \n", header.length_of_fmt);
		printf("(21-22) Format type: %u %s \n", header.format_type, format_name);
		printf("(23-24) Channels: %u \n", header.channels);
		printf("(25-28) Sample rate: %u\n", header.sample_rate);
		// Seems to be a gap between 28 and 33... I forget which that is 
		printf("(33-34) Block Alignment: %u \n", header.block_align);
		printf("(35-36) Bits per sample: %u \n", header.bits_per_sample);
		printf("(37-40) Data Marker: %s \n", header.data_chunk_header);
		printf("(41-44) Size of data chunk: %u \n", header.data_size);
		printf("Number of samples:%lu \n", num_samples);
		printf("Size of each sample:%ld bytes\n", size_of_each_sample);
		printf("Mallocing %ld bytes for sound\n", size_of_each_sample * num_samples);
	}

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
    {
        long i = 0;
        char data_buffer[size_of_each_sample ];
        int  size_is_correct = AL_TRUE;

        // make sure that the bytes-per-sample is completely divisible by num.of channels
        long bytes_in_each_channel = (size_of_each_sample / header.channels);
        if ((bytes_in_each_channel  * header.channels) != size_of_each_sample) 
        {
            printf("Error: %ld x %ud <> %ld\n", bytes_in_each_channel, header.channels, size_of_each_sample);
            size_is_correct = AL_FALSE;
        }

        if (size_is_correct)
        { 		
            for (i =0; i < num_samples; i++) 
            {
                read = fread(data_buffer, sizeof(data_buffer), 1, f_ptr);
                if (read == 1)
                {
                    for(int k = sizeof(data_buffer); k >= 0 ; k--)
                    {
                        sound_data[i*sizeof(data_buffer) + k] = data_buffer[k];
                    }
					
					// This might work for reverse endianness.  I haven't looked at this code in months.
                    //for(int k = 0; k < sizeof(data_buffer); k++)
                        //sound_data[i*sizeof(data_buffer) + sizeof(data_buffer) - k] = data_buffer[k];
                }
                else 
                {
					// Resize the data - IDK why I put this here.
                    char *temp = realloc(sound_data, (i) * size_of_each_sample);
                    if(temp != NULL)
                    {
                        printf("Rearranging sound data\n");
                        sound_data = temp;
                        *data = (ALvoid *) sound_data;
                        *size = (i) * size_of_each_sample;
                    }
                    printf("Error reading file. %d bytes\n", read);
                    printf("%d bytes read, %d bytes remained\n", (i+1), (num_samples - i));
                    break;
                }
            }
        }
    }
    fclose(f_ptr);
    return;
}


void SL_InitReverb()
{
    EFXEAXREVERBPROPERTIES efxReverb;
    EAXREVERBPROPERTIES eaxRoom = EFX_REVERB_PRESET_ROOM;
    
    alGenAuxiliaryEffectSlots(1, uiEffectSlot);
	if (alGetError() != AL_NO_ERROR)
    {
		printf("CANT DO THE REVERBS (1) :'\\n");
        return;
    }
    
    // Clear AL Error State
    alGetError();

    // Generate an Effect
    alGenEffects(1, uiEffect);
    if (alGetError() != AL_NO_ERROR)
    {
        printf("CANT DO THE REVERBS (2) :'\\n");
        return;
    }
    // Set the Effect Type
    alEffecti(*uiEffect, AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);
    if (alGetError() != AL_NO_ERROR)
    {
        alDeleteEffects(1, puiEffect);
        printf("CANT DO THE REVERBS (3) :'\\n");
        return;
    }
    
    alEffectf(*uiEffect, AL_EAXREVERB_DENSITY, eaxRoom.flDensity);
    alEffectf(*uiEffect, AL_EAXREVERB_DIFFUSION, eaxRoom.flDiffusion);
    alEffectf(*uiEffect, AL_EAXREVERB_GAIN, eaxRoom.flGain);
    alEffectf(*uiEffect, AL_EAXREVERB_GAINHF, eaxRoom.flGainHF);
    alEffectf(*uiEffect, AL_EAXREVERB_GAINLF, eaxRoom.flGainLF);
    alEffectf(*uiEffect, AL_EAXREVERB_DECAY_TIME, eaxRoom.flDecayTime);
    alEffectf(*uiEffect, AL_EAXREVERB_DECAY_HFRATIO, eaxRoom.flDecayHFRatio);
    alEffectf(*uiEffect, AL_EAXREVERB_DECAY_LFRATIO, eaxRoom.flDecayLFRatio);
    alEffectf(*uiEffect, AL_EAXREVERB_REFLECTIONS_GAIN, eaxRoom.flReflectionsGain);
    alEffectf(*uiEffect, AL_EAXREVERB_REFLECTIONS_DELAY, eaxRoom.flReflectionsDelay);
    alEffectfv(*uiEffect, AL_EAXREVERB_REFLECTIONS_PAN, eaxRoom.flReflectionsPan);
    alEffectf(*uiEffect, AL_EAXREVERB_LATE_REVERB_GAIN, eaxRoom.flLateReverbGain);
    alEffectf(*uiEffect, AL_EAXREVERB_LATE_REVERB_DELAY, eaxRoom.flLateReverbDelay);
    alEffectfv(*uiEffect, AL_EAXREVERB_LATE_REVERB_PAN, eaxRoom.flLateReverbPan);
    alEffectf(*uiEffect, AL_EAXREVERB_ECHO_TIME, eaxRoom.flEchoTime);
    alEffectf(*uiEffect, AL_EAXREVERB_ECHO_DEPTH, eaxRoom.flEchoDepth);
    alEffectf(*uiEffect, AL_EAXREVERB_MODULATION_TIME, eaxRoom.flModulationTime);
    alEffectf(*uiEffect, AL_EAXREVERB_MODULATION_DEPTH, eaxRoom.flModulationDepth);
    alEffectf(*uiEffect, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, eaxRoom.flAirAbsorptionGainHF);
    alEffectf(*uiEffect, AL_EAXREVERB_HFREFERENCE, eaxRoom.flHFReference);
    alEffectf(*uiEffect, AL_EAXREVERB_LFREFERENCE, eaxRoom.flLFReference);
    alEffectf(*uiEffect, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR, eaxRoom.flRoomRolloffFactor);
    alEffecti(*uiEffect, AL_EAXREVERB_DECAY_HFLIMIT, eaxRoom.iDecayHFLimit);

    if (alGetError() != AL_NO_ERROR)
    {
        printf("CANT DO THE REVERBS (4) :'\\n");
        return;
    }

    // Load Effect into Auxiliary Effect Slot
    alAuxiliaryEffectSloti(*uiEffectSlot, AL_EFFECTSLOT_EFFECT, *uiEffect);

    // Enable (non-filtered) Send from Source to Auxiliary Effect Slot
    alSource3i(uiSource, AL_AUXILIARY_SEND_FILTER, uiEffectSlot, 0, AL_FILTER_NULL);

}
void SL_UninitReverb()
{
    if(uiEffectSlot != NULL)
    {
        // Load NULL Effect into Effect Slot
        alAuxiliaryEffectSloti(*uiEffectSlot, AL_EFFECTSLOT_EFFECT, AL_EFFECT_NULL);
    }
    if(uiEffect != NULL)
    {
        // Delete Effect
        alDeleteEffects(1, uiEffect);
    }
    if(uiEffectSlot != NULL)
    {
        // Delete Auxiliary Effect Slot
        alDeleteAuxiliaryEffectSlots(1, uiEffectSlot);
    }
}
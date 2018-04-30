/*
*	This sound library was made with the hopes of "generalizing" the sound portion
*	of the project, while still maintaining all the necessary power of the underlying sound library.
*	The hope is to make utilizing the sound library less bazaar by condensing the overhead into functions
*	and removing capabilities that are not required.
*/
#ifndef _OPENAL_ACCESS_H_
#define _OPENAL_ACCESS_H_

#include "/usr/local/include/AL/al.h"
#include "/usr/local/include/AL/alc.h"
#include "/usr/local/include/AL/alext.h"

//#include <efx.h>
//#include <efx-presets.h>
#include "/usr/local/include/AL/efx.h"
#include "/usr/local/include/AL/efx-presets.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
*	Some vocabulary definition:
*		"Source" is a place where sound will emit from
*		"Listener" is the place where sound is heard from
*
*	Some background info:
*		OpenAL Operates under the context of a 3D World environment
*		1: This means that sound sources have a coordinate location in your 3D world
*		2: This means that YOU, the LISTENER, has a coordinate in your 3D world as well
*		3: This also means that YOU, as a LISTENER, have a facing within the 3D World
*			a: This facing is described with two vectors: "AT" and "UP"
*				i: the AT vector describes where your face is pointing.   This means an
*					AT vector value of <x,y,z> = <0,0,-1> means you're facing down the -Z axis.
*					to play a sound in front of that listener, set the coordinate to <0,0,-offset>,
*					where "offset" is how far away the sound should be.  To play a sound to the right-hand
*					side of the user, play a sound at the coordinate <offset,0,0>.
*
*				ii: the UP vector describes where the top of your head is.  it MUST BE ORTHOGONAL
*					to the AT vector.  To keep things simple, best just leave the UP vector as 
*					<x,y,z> = <0,1,0>, and completely ignore the vertical aspect of the 3D world.
*
*			b: For the current scope of this project, setting the user's facing will likely be
*				a one-and-done activity.  If a more advanced approach is taken, facing will need to
*				be considered.
*
*/

/*
*
*
*	Here's a basic how-to use this library:
*
*	EVERY SOUND YOU PLAY MUST BE ASSOCIATED WTIH A Sound_Source VARIABLE.
*	Thus, to play a sound on "the chair" you'll want: to have a something like this:
*	"Sound_Source chair;".  You can have as many sound sources as you'd like.
*	Each sound source is limited to having one sound buffer at a time.  This is 
*	because I'm a lazy programmer and it is not a limitation of the library.
*	I don't know why the hell you'd even want more than one sound played at a source
*	so just lay off the salt, OK?
*
*	The reason sounds are kept inside their own structure is simply because
*	it makes it so that the program can be free of any and all memory leaks that
*	may occur if the BufferID's are lost when the buffer is attached to the source.
*	The user must update the Sound_Source's x,y,z position manually.  This also has
*	the added benefit of keeping track of where each sound is currently placed, though
*	there most likely access-calls for that anyway.
*
*	1: Initialize everything:
* 		Call SL_Init() to initialize all the peripheral library stuff
* 		Then for each Sound_Source, call SL_InitSource(&src) - this will
* 		call the necessary OpenAL initialization stuff
* 
*	2: Load in a sound wave before using a Sound_Source
*		Call SL_LoadSound on an initialized Sound_Source, with the name
*		of the sound file that should be played at the source name.
*		
*		It is not necessary to load in sound-waves to unused sound-sources
*		if memory is a constraint.  I don't know how necessary this will ever be.
* 
*	3: Place the sound
*		Designate an X,Y,Z position of the sound source by modifying the internal variables
*
*	4: Play the sound
*		You decide whether or not the sound will loop, or play once.  If told to play once
*		the function will ignore subsequent "play" commands till the sound finishes.  If told
*		to stop, it will cease playing the sound (as you'd probably expect).
*
*	5 (OPTIONAL): Move / Rotate the listener
*		
*	When finished, do the following:
*
*	1: Loop through every initialized sound source, and uninitialize them
*	2: Uninitialize the OpenAL global variables.
*		 
*/
typedef struct SL_Sound {
	ALuint sourceID;		// Keeps track of the internal OpenAL Sound ID
	ALuint bufferID;	// Keeps track of the OpenAL associated BufferID
						// Because I'm lazy, only 1 buffer per sound.

	// Maintain absolute sound position
	ALfloat x;
	ALfloat y;
	ALfloat z;
} SL_Sound;

// Listener is just for keeping track of their position and orientation
typedef struct SL_Listener {
	// Contains the absolute position of the listener
	ALfloat pos_x, pos_y, pos_z;
	
	// Contains the absolute facing of the listener, in vector format
	ALfloat at_x, at_y, at_z;
	
	// Contains the absolute facing OF THE TOP OF THE LISTENER'S HEAD,
	// in vector format
	ALfloat up_x, up_y, up_z;
} SL_Listener;
//Constructor functions:

// Call this function before you do anything else with this library.
// Or else.
void SL_Init();

void SL_InitReverb();

// Call this function before you even try to use a Sound_Source.
// Please don't pass in a NULL POINTER for the structure.  If you do, it will
// politely nuke the rest of your program.
void SL_InitSource(SL_Sound *src);

// Load a sound into 
void SL_LoadSound(SL_Sound *src, char *soundfile);		// Loads a sound into a source

// General Access Functions
// These should be the only functions necessary for the project?
void SL_PlaceSound(SL_Sound *src);
void SL_PlaySound(SL_Sound *src, ALint play, ALint loop);
void SL_TurnUser(ALfloat at_x, ALfloat at_y, ALfloat at_z,
				 ALfloat up_x, ALfloat up_y, ALfloat up_z);
				 
void SL_MoveUser(ALfloat pos_x, ALfloat pos_y, ALfloat pos_z);
void SL_UpdateUser(SL_Listener *user);


// Two destructor functions
void SL_FreeSound(SL_Sound *src);
void SL_Uninit();
 #if defined(__cplusplus)
}  /* extern "C" */
#endif

#endif

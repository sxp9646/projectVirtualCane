#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "sound_library.h"


#define	TEST_WAVE_FILE		"Footsteps.wav"
#define PI (3.14)

#define TRUE 1
#define FALSE 0

int main()
{
	SL_Sound test_sound;
	SL_Listener test_listener;
	SL_Init();
	SL_InitSource(&test_sound);
	
// Initialization
    char fname[1024];

    printf("Name of .wav file?\n");
    scanf("%s", &fname);

	SL_LoadSound(&test_sound,fname);
	
    ALfloat pos[3] = {0.0, 0.0, 2.0};
    ALfloat orient[6] = {0,0, -1, 0,1,0}; // AT, AT, AT, UP, UP, UP ?
    double degree = 90.0;

	test_listener.pos_x = 0;
	test_listener.pos_y = 0;
	test_listener.pos_z = 0;

	test_listener.up_x = 0;
	test_listener.up_y = 1;
	test_listener.up_z = 0;
	
	test_listener.at_x = orient[0];
	test_listener.at_y = 0;
	test_listener.at_z = orient[2];
	SL_UpdateUser(&test_listener);

	test_sound.x = pos[0];
	test_sound.y = pos[1];
	test_sound.z = pos[2];
	SL_PlaceSound(&test_sound);

    // Wait for the song to complete
    printf("playing sound\n");
	SL_PlaySound(&test_sound, TRUE, TRUE);
	char dir = '0';
    do {

        dir = fgetc(stdin);

        for(int k = 0; k < 5; k++)
        {
			if(dir != 'z')
			{
				switch(dir)
				{
					case 'w':
						pos[2] -= 0.1;
						break;
					case 's':
						pos[2] += 0.1;
						break;
					case 'a':
						pos[0] += 0.1;
						break;
					case 'd':
						pos[0] -= 0.1;
						break;
					case 'q':
						degree += 6.0;
						break;
					case 'e':
						degree -= 6.0;
						break;
					case 'r':
						degree = 0.0;
						//orient[0] = 0.0;
						//orient[2] = -1.0;
						pos[0] = 0.0;
						pos[2] = 0.0;
						break;
					default:
						continue;
						break;
				}
				orient[0] = cos(degree * (PI / 180.0));
				orient[2] = sin(degree * (PI / 180.0));
				printf("x:%g\ty:%g\tz:%g\n", pos[0], pos[1], pos[2]);
				printf("Degree: %.2f\n", degree);
				printf("at x: %.2f\tat y: %.2f\tat z: %.2f\n\n", orient[0], orient[1], orient[2]);
				
				test_sound.x = pos[0];
				test_sound.y = pos[1];
				test_sound.z = pos[2];
				SL_PlaceSound(&test_sound);

				test_listener.at_x = orient[0];
				test_listener.at_y = 0;
				test_listener.at_z = orient[2];
				SL_UpdateUser(&test_listener);
				//alListenerfv(AL_POSITION,pos); 
			}
        }
    } while (dir != 'z');
	SL_PlaySound(&test_sound, FALSE, FALSE);
	SL_FreeSound(&test_sound);
	printf("free'd sound\n");
	SL_Uninit();
    printf("done\n");

}
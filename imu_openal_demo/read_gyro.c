#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>

// MAYBE USE THESE?
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "openal_gyro.h"

#define DEVICE "/dev/ttyACM0"

int set_interface_attribs (int fd, int speed, int parity)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                printf ("error %d from tcgetattr", errno);
                return -1;
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break
        // as \000 chars
        tty.c_iflag &= ~IGNBRK;         // disable break processing
        tty.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
        tty.c_oflag = 0;                // no remapping, no delays
        tty.c_cc[VMIN]  = 0;            // read doesn't block
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

        tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                        // enable reading
        tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;
//        tty.c_cflag &= ~CRTSCTS;

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
        {
                printf ("error %d from tcsetattr", errno);
                return -1;
        }
        return 0;
}

void set_blocking (int fd, int should_block)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                printf ("error %d from tggetattr", errno);
                return;
        }

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
	{
		printf("whatevs");
	}
                //error_message ("error %d setting term attributes", errno);
}

int main()
{
	char *portname = DEVICE;
	double zero;
	double degree = 0.0;
	char buff [16];
	int fd = open (portname, O_RDWR | O_NOCTTY | O_SYNC);

	if (fd < 0)
	{
		printf ("error %d opening %s: %s", errno, portname, strerror (errno));
		return 0;
	}

	set_interface_attribs (fd, B115200, 0);  // set speed to 115,200 bps, 8n1 (no parity)
	set_blocking (fd, 0);                // set no blocking

	init();

	/*
	int n = read (fd, buff, sizeof(buff));  // read up to 100 characters if ready to read
	zero = 360 - atof(buff);
	degree= ((360 - atof(buff)) - zero)+ 90;
	//sscanf(buff, "%f", &degree); 
	printf("Buffer in: %s\n", buff);
	printf("Degrees: %f\n\n", degree);
	turn(degree);
	*/

	do {
		printf("Input Angle:\n");
		int angle;

		scanf("%d", &angle);

		//orient[0] = cos(degree * (PI / 180.0));
		//orient[2] = sin(degree * (PI / 180.0));

		printf("Angle: %d\n",  angle);

		//alListenerfv(AL_POSITION,pos); 

		play();

		// Zero out the gyro and turn the user to face it.
		int n = read (fd, buff, sizeof(buff));  // read up to 100 characters if ready to read
		zero = atof(buff) + angle;
		degree= (atof(buff)) - zero;
		if(degree < 0)
			degree += 360;
		//sscanf(buff, "%f", &degree); 
		printf("Buffer in: %s\n", buff);
		printf("Degrees: %f\n\n", degree );
		turn(degree);

		//
		while(1)
		{
			// Zero out the gyro and turn the user to face it.
			int n = read (fd, buff, sizeof(buff));  // read up to 100 characters if ready to read
			degree= (atof(buff)) - zero;
			if(degree < 0)
				degree += 360;
			//sscanf(buff, "%f", &degree); 
			printf("Buffer in: %s\n", buff);
			printf("Degrees: %f\n\n", degree);
			turn(degree);
		}
		printf("Result turn: %f\n\n", degree);
		stop();
		//alListenerfv(AL_ORIENTATION,orient); 

	} while (1);

	end();
	return 0;
}

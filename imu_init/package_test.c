#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include "LSM9DS1.h"
#include "imu_init.c"
#include <sys/ioctl.h> 
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <linux/i2c.h>


int main (void){
	float reading;
        for(;;){
                //getLimit();
		reading = getReading(0);
		sleep(1);
		//hardIron();
                printf("%.2f\n", reading);
                //sleep(1);
        }

}


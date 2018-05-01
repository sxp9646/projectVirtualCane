/*
Author: Abdulaziz Alorifi
File: imu_init.c
Last Modification Date: 04/17/2018
Rochester institue of Technology student project
Project: Virtual Cane P18047

Description:
This file contains functions to initilize and read data from
the Adafruit LSM9DS1 IMU. In adition, it contains fucntions
to interpret the data into meaningful angles for pitch,
roll, and heading.


IMPORTANT: MAXIMUM AND MINIMUM VALUES FOR EACH MAGNOMETER
AXIS MUST BE DETERMINED USING THE FUNCTION "hardiron". THESE
VALUES MUST BE MODIFIED IN the function "getReading"'S
HARD IRON PART.

*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include "LSM9DS1.h"
#include "imu_init.h"

#define M_PI 3.14159265359
#define RAD_TO_DEG 180/M_PI


int file;
int acc[3];
int gyr[3];
int magx[3];
int magy[3];
int magz[3];
//int accX = 0;
//int accY = 0;
int zero = 0;
//float accXnorm, accYnorm, p, r, magX, magY;
//int accZ = 0;
//int magA = 0;
//float  magDeg = 0.0;
//float res = 0.0;
//float mx, my, mz;



/*
Description: delay function, not operational
*/
void delay (int t){
        int delayt = 1000* t;
        clock_t start_time = clock();
        while (clock() < start_time + delayt){
        }

}

/*
Description: select the sensor to be read

Input: device address

*/
void selectDevice (int addr){
	char filename[20];
	//int file;
	sprintf(filename, "/dev/i2c-%d",1);
	file = open(filename, O_RDWR);
	if (file < 0){
		printf("Bus not opened");
		exit(1);
	}
	if (ioctl(file, I2C_SLAVE, addr) <0){
		printf("Device can not be selected");
	}else{}
}

/*
Description: write  bits to control registers to
configure the sensors.

Input:	int device: address of the device to read
	uint8_t reg: address of the register to read from
	uint8_t value: bits to write

*/
void writeReg(int  device, uint8_t reg, uint8_t value)
{

        selectDevice(device);

        int result = i2c_smbus_write_byte_data(file, reg, value);
        if (result == -1){
                printf ("Failed to write to register");
        exit(1);
    }
}

/*
Description: Initilize all 3 LSM9DS1 sensors
*/

void enableImu(){
 /* enable mag */
        writeReg(0x1e, LSM9DS1_CTRL_REG1_M, (0b10011100)); //set xy to hi-pro & $
        writeReg(0x1e, LSM9DS1_CTRL_REG2_M, (0b01000000)); //full 16 gauss 0b01100000
        writeReg(0x1e, LSM9DS1_CTRL_REG3_M, (0b00000000)); //conti-mode
        writeReg(0x1e, LSM9DS1_CTRL_REG4_M, (0b00000000)); //set z to hi-pro(++i$
        /* enable mag end */
        /* enable Gyro */
        writeReg(0x6b, LSM9DS1_CTRL_REG4, (0b00111000));
        writeReg(0x6b, LSM9DS1_CTRL_REG1_G, (0b10111000));
	writeReg(0x6b, LSM9DS1_ORIENT_CFG_G, (0b10111000));
        /* enable gyro end */
        /* enable acc */
        writeReg(0x6b, LSM9DS1_CTRL_REG5_XL, (0b00111000));
        writeReg(0x6b, LSM9DS1_CTRL_REG6_XL, (0b00101000));
	//printf("working again");

}

/*
Description: read a block of data from register.

Parameters:
Input:	uint_8 command takes address of to be read
	uint_8 size: the size of words to read
	uint_8 *data: address to save the data


*/
void readBlock(uint8_t command, uint8_t size, uint8_t *data)
{	//read dara
	int result = i2c_smbus_read_i2c_block_data(file, command, size, data);
    	//reading verification
	if (result != size){
                printf("Failed to read block from I2C.");
                exit(1);
        }
}

/*
Description: read data register for the specified sensor

Parameters:
Input:  -int array[] takes an array of size 3
	 array must be global as it's shared by multiple
	 functions.
	-int device takes a sensor's data register address

Output: values are saved in the array passed
*/
void readreg(int  array[], int reg, int device)
{
        uint8_t block[6];
        selectDevice(device);
	//read register
        readBlock(0x80 |  reg, sizeof(block), block);
        // Combine readings for each axis.
        array[0] = (int16_t)(block[0] | block[1] << 8);
        array[1] = (int16_t)(block[2] | block[3] << 8);
        array[2] = (int16_t)(block[4] | block[5] << 8);
}


/*
Description: this function calculates max and minvalues
for the magnometer hard iron calibration

How To: cal the funtion and start moving the imu
in all direction slowly. New min and max values will
be printed as they are refreshed. The function runs
on a timer and will show all the final values.

Parameters: No input and output


//statments need to corrected
*/
void hardIron(){
        int maxx = 0, maxy = 0, maxz = 0, minx = 0, miny = 0, minz = 0;
        for (int i=0; i<100000; i++){
		//initilize the imu and read magnometer data
                enableImu();
                readreg(magx, LSM9DS1_OUT_X_L_M, 0x1e);
                readreg(magy, LSM9DS1_OUT_Y_L_M, 0x1e);
                readreg(magz, LSM9DS1_OUT_Z_L_M, 0x1e);

		//check for min and max values
                if (magx[0] < (minx-1)){
                        minx = magx[0];
                        printf("MinX = %i\n", minx);
                }else if (magx[0] > (maxx+1)){
                        maxx = magx[0];
                        printf("MaxX = %i\n", maxx);
                }
                if (magy[0] < (miny-1)){
                        miny = magy[0];
                        printf("MinY = %i\n", miny);
                }else if (magy[0]> (maxy+1)){
                        maxy = magy[0];
                        printf("MaxY = %i\n", miny);
                }
                if (magz[0] < (minz-1)){
                        minz = magz[0];
                        printf("MinZ = %i\n", minz);
                }else if (magz[0]> (maxz+1)){
                        maxz = magz[0];
                        printf("MinX = %i\n", minx);
                }
        }
		printf("MaxX = %i\tMinX = %i\nMaxY = %i\tMinY = %i\nMaxZ = %i\tMinZ = %i\n\n\n", maxx ,minx, maxy,miny,maxz, minz);

}


/**
Description: reads data from sensors
and return pitch, roll, heading

Parameters:

input: int angle (0 for hrading,
1 for pitch, and 2 for roll.

output: float number of the angle in degress

**/
float getReading (int angle){
	int maxx = 0, maxy = 0, maxz = 0, minx = 0, miny = 0, minz = 0;
	int accX = 0, accY = 0;
	float accXnorm, accYnorm, p, r, magX, magY;
	float  magDeg = 0.0;
	enableImu();

	if (angle == 0){//heading
		//read magnometer and accelerometer
		readreg(magx, LSM9DS1_OUT_X_L_M, 0x1e);
 		readreg(magy, LSM9DS1_OUT_Y_L_M, 0x1e);
 		readreg(magz, LSM9DS1_OUT_Z_L_M, 0x1e);
		readreg(acc, LSM9DS1_OUT_X_L_XL, 0x6b);


		/**
		Hard Iron calibration
		specific values for offset were obtained
		by the max and min values for each axis
		**/
		magx[0] -= (1690 + (-1030)) /2;
		magy[0] -= (2015 + (-700 )) /2;
		magz[0] -= (2100 + (-1600)) /2;


		//Normalize accelerometer for tilt compensation
		accXnorm = acc[0]/sqrt(acc[0]*acc[0]+acc[1]*acc[1]+acc[2]*acc[2]);
		accYnorm = acc[1]/sqrt(acc[0]*acc[0]+acc[1]*acc[1]+acc[2]*acc[2]);

		p = asin(accXnorm);
		r = -asin(accYnorm/cos(p));

		/*calculate the x and y values after hard iron calibration
		and tilt compensation
		*/
		magX = magx[0]*cos(p)+magz[0]*sin(p);
		magY = magx[0]*sin(r)*sin(p)+magy[0]*cos(r)+magz[0]*sin(r)*cos(p);

		//calculate heading
		magDeg = 180 * atan2(magY,magX)/M_PI;

		//zero out the angle
		//if (zero == 0){
		//	zero = magDeg;
		//}
		//magDeg -= zero;

		//Limit heading within 0-360
                if(magDeg <0){
                        magDeg += 360;
                }

		return magDeg;
	}else if (angle == 1){//pitch
		//read data from accelerometer
        	readreg(acc, LSM9DS1_OUT_X_L_XL, 0x6b);

		//calculate pitch angle
        	accX = (float) (atan2(acc[1], acc[2])+M_PI)* RAD_TO_DEG;
        	accX -= (float)180.0;
		return accX;
	}else if (angle == 2){//roll
		//read accelerometer data
                readreg(acc, LSM9DS1_OUT_X_L_XL, 0x6b);

		//calculate angle
		accY = (float) (atan2(acc[2], acc[0])+M_PI)* RAD_TO_DEG;

		//Limit angle between 0-360
		if(accY >90){
                        accY -= (float)270;
                }else{
                        accY += (float)90;
                }
		return accY;
	}else{
		return 0.0;
	}

}

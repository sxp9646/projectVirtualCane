#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include <linux/i2c-dev.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
//#include <avr>
//#include "imu_read.h"
#include "Adafruit_LSM9DS1.h"
//#include "./Adafruit_Sensor.h"
//#include "./Wire.h"
int file;
int *a;
float AccXangle, AccYangle;
#define RAD_TO_DEG 57.29578;
//#define M_PI 3.14159265358979323846;
void selectDevice(int file, int addr)
{
        /* selecting the device to read/write accl+mag (0x6b) or gyro(0x1e) */
        char device[3];
        if (ioctl(file, I2C_SLAVE, 0x6b) < 0){
                printf("Couldn't select device");
        }

}

void  readBlock(uint8_t command, uint8_t size, uint8_t *data)
{
    int result = i2c_smbus_read_i2c_block_data(file, command, size, data);
    if (result != size){
                printf("Failed to read block from I2C.");
                exit(1);
        }
}

void readACC(int  *a)
{
        uint8_t block[6];
        selectDevice(file,LSM9DS1_ACC_ADDRESS);
        readBlock(0x80 |  LSM9DS1_OUT_X_L_XL, sizeof(block), block);

        // Combine readings for each axis.
        *a = (int16_t)(block[0] | block[1] << 8);
        *(a+1) = (int16_t)(block[2] | block[3] << 8);
        *(a+2) = (int16_t)(block[4] | block[5] << 8);

}

void readMAG(int  *m)
{
        uint8_t block[6];
        selectDevice(file,LSM9DS1_MAG_ADDRESS);
        readBlock(0x80 |  LSM9DS1_OUT_X_L_M, sizeof(block), block);
        // Combine readings for each axis.
        *m = (int16_t)(block[0] | block[1] << 8);
        *(m+1) = (int16_t)(block[2] | block[3] << 8);
        *(m+2) = (int16_t)(block[4] | block[5] << 8);

}

void readGYR(int *g)
{
        uint8_t block[6];
        selectDevice(file,LSM9DS1_GYR_ADDRESS);
        readBlock(0x80 |  LSM9DS1_OUT_X_L_G, sizeof(block), block);
        // Combine readings for each axis.
        *g = (int16_t)(block[0] | block[1] << 8);
        *(g+1) = (int16_t)(block[2] | block[3] << 8);
        *(g+2) = (int16_t)(block[4] | block[5] << 8);
}

void writeAccReg(uint8_t reg, uint8_t value)
{
        selectDevice(file,LSM9DS1_ACC_ADDRESS);

        int result = i2c_smbus_write_byte_data(file, reg, value);
        if (result == -1){
                printf ("Failed to write byte to I2C Acc.");
        exit(1);
    }
}

void writeMagReg(uint8_t reg, uint8_t value)
{
        selectDevice(file,LSM9DS1_MAG_ADDRESS);
        int result = i2c_smbus_write_byte_data(file, reg, value);
        if (result == -1){
                printf("Failed to write byte to I2C Mag.");
                exit(1);
        }
}
void writeGyrReg(uint8_t reg, uint8_t value)
{
        selectDevice(file,LSM9DS1_GYR_ADDRESS);
        int result = i2c_smbus_write_byte_data(file, reg, value);
        if (result == -1){
                printf("Failed to write byte to I2C Gyr.");
                exit(1);
        }
}



int init()
{
	/* Open the i2c bus */
	char filename[20];
	sprintf(filename, "/dev/i2c-%d", 1);
	file = open (filename, O_RDWR);
	if (file<0) {
		printf("Can't open i2c bus");
		exit(1);
	}
}

	/* end of opening i2c bus*/
	/* enable mag */
	writeMagReg(LSM9DS1_CTRL_REG1_M(), (0b01011100)); //set xy to hi-pro & dt= 80Hz
	writeMagReg(LSM9DS1_CTRL_REG2_M(), (0b01100000)); //full 16 gauss
	writeMagReg(LSM9DS1_CTRL_REG3_M(), (0b00000000)); //conti-mode
	writwMagReg(LSM9DS1_CTRL_REG4_M(), (0b00001000)); //set z to hi-pro(++ifUltra)
	/* enable mag end */
	/* enable Gyro */
	writeGyrReg(LSM9DS1_CTRL_REG4, (0b00111000));
	writeGyrReg(LSM9DS1_CTRL_REG1_G, (0b10111000));
	writeGyrReg(LSM9DS1_ORIENT_CFG_G, (0b10111000));
	/* enable gyro end */
	/* enable acc */
	writeAccReg(LSM9DS1_CTRL_REG5_XL, (0b00111000));
	writeAccReg(LSM9DS1_CTRL_REG6_XL, (0b00101000));

}

int read_acc(int *a)
{
	uint8_t block[6];
	selectDevice(file,LSM9DS1_ADDRESS_ACCELGYRO);
	readBlock((0x80 | LSM9DS1_REGISTER_OUT_X_L_M), sizeof(block), block);

	*a = (int16_t)(block[0] | block[1] << 8);
	*(a+1) = (int16_t)(block[2] | block[3] << 8);
	*(a+2) = (int16_t)(block[4] | block[5] << 8);
	AccXangle = (float) (atan2(a[1],a[2])+ M_PI) * RAD_TO_DEG;
	AccYangle = (float) (atan2(a[2],a[0])+ M_PI) * RAD_TO_DEG;
	printf("%.5f", AccXangel);
	printf("%.5f", AccYangel);

}
int main()
{
	init();
	selectDevice(file, 5);
	for(;;){
		read_ACC(a);
		AccXangle = (float) (atan2(a[1],a[2])+ M_PI) * RAD_TO_DEG;
	        AccYangle = (float) (atan2(a[2],a[0])+ M_PI) * RAD_TO_DEG;
       		printf("%.5f", AccXangel);
       		printf("%.5f", AccYangel);

	}

}

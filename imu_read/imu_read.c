#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <linux/i2c-dev.h>
#include <stdint.h>
#include "./imu_read.h"
#include "./Adafruit_LSM9DS1.h"
#include "./Adafruit_Sensor.h"
//#include "./Wire.h"
int file;
int *a;
float AccXangle, AccYangle, 
float RAD_TO_DEG = 57.29578;
int init()
{
	/* Open the i2c bus */
	char filename[20];
	sprintf(filename, "/dev/i2c-%d", 1);
	file = open (filename, O_RDWR);
	if (file<0) {
		printf("Can't open bus");
		exit(1);
	}
	/* end of opening i2c bus*/
	/* enable mag */
	LSM9DS1_CTRL_REG1_M = 0b01011100; //set xy to hi-pro & dt= 80Hz
	LSM9DS1_CTRL_REG2_M = 0b01100000; //full 16 gauss
	LSM9DS1_CTRL_REG3_M = 0b00000000 //conti-mode
	LSM9DS1_CTRL_REG1_M = 0b00001000; //set z to hi-pro(++ifUltra)
	//write8 (CTRL_REG2_M, 0b01100000); //full 16 gauss
	//write8(CTRL_REG3_M, 0b00000000); //conti-mode
	//writeAccReg(CTRL_REG4_M, 0b00001000); //set z to hi-pro(++ifultra)


}
void selectDevice(int file, int addr)
{
        /* selecting the device to read/write accl+mag (0x6b) or gyro(0x1e) */
        char device[3];
        if (ioctl(file, I2C_SLAVE, 0x6b) < 0){
                printf("Couldn't select device");
        }

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
		read_acc(a);
	}

}

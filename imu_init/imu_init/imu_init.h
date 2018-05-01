/*
Author: Abdulaziz Alorifi
File: imu_init.h (header file for init_imu.c)
Last Modification Date: 04/17/2018
Rochester institue of Technology student project
Project: Virtual Cane P18047

Description:
This file contains functions to initilize and read data from
the Adafruit LSM9DS1 IMU. In adition, it contains fucntions
to interpret the data into meaningful angles for pitch,
roll, and heading.
*/

#ifndef __IMU_INIT
#define __IMU_INIT


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


void delay (int t);
void selectDevice(int addr);
void writeReg(int  device, uint8_t reg, uint8_t value);
void enableImu();
void readBlock(uint8_t command, uint8_t size, uint8_t *data);
void readreg(int array[], int reg, int device);
void hardIron();
float getReading(int angle);
#endif

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LSM9DS1.h>
#include <Adafruit_Sensor.h>  // not used in this demo but required!
#include "math.h"

// i2c
Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1();

#define LSM9DS1_SCK A5
#define LSM9DS1_MISO 12
#define LSM9DS1_MOSI A4
#define LSM9DS1_XGCS 6
#define LSM9DS1_MCS 5
// You can also use software SPI
//Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1(LSM9DS1_SCK, LSM9DS1_MISO, LSM9DS1_MOSI, LSM9DS1_XGCS, LSM9DS1_MCS);
// Or hardware SPI! In this case, only CS pins are passed in
//Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1(LSM9DS1_XGCS, LSM9DS1_MCS);


void setupSensor()
{
  // 1.) Set the accelerometer range
  lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_2G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_4G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_8G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_16G);
  
  // 2.) Set the magnetometer sensitivity
  lsm.setupMag(lsm.LSM9DS1_MAGGAIN_4GAUSS);
  //lsm.setupMag(lsm.LSM9DS1_MAGGAIN_8GAUSS);
  //lsm.setupMag(lsm.LSM9DS1_MAGGAIN_12GAUSS);
  //lsm.setupMag(lsm.LSM9DS1_MAGGAIN_16GAUSS);

  // 3.) Setup the gyroscope
  lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_245DPS);
  //lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_500DPS);
  //lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_2000DPS);
}


void setup() 
{
  Serial.begin(115200);

  while (!Serial) {
    delay(1); // will pause Zero, Leonardo, etc until serial console opens
  }
  
  Serial.println("LSM9DS1 data read demo");
  
  // Try to initialise and warn if we couldn't detect the chip
  if (!lsm.begin())
  {
    Serial.println("Oops ... unable to initialize the LSM9DS1. Check your wiring!");
    while (1);
  }
  Serial.println("Found LSM9DS1 9DOF");

  // helper to just set the default scaling we want, see above!
  setupSensor();
}

void loop() 
{
  lsm.read();  /* ask it to read in the data */ 

  /* Get a new sensor event */ 
  sensors_event_t a, m, g, temp;

  lsm.getEvent(&a, &m, &g, &temp); 

  //Serial.print("Accel X: "); Serial.print(a.acceleration.x); Serial.print(" m/s^2");
  //Serial.print("\tY: "); Serial.print(a.acceleration.y);     Serial.print(" m/s^2 ");
  //Serial.print("\tZ: "); Serial.print(a.acceleration.z);     Serial.println(" m/s^2 ");
  printOrientation(a.acceleration.x,a.acceleration.y,a.acceleration.z);

  

  //Serial.print("Mag X: "); Serial.print(m.magnetic.x);   Serial.print(" gauss");
  //Serial.print("\tY: "); Serial.print(m.magnetic.y);     Serial.print(" gauss");
  //Serial.print("\tZ: "); Serial.print(m.magnetic.z);     Serial.println(" gauss");
  printHeading(m.magnetic.x,m.magnetic.y);

  //Serial.print("Gyro X: "); Serial.print(g.gyro.x);   Serial.print(" dps");
  //Serial.print("\tY: "); Serial.print(g.gyro.y);      Serial.print(" dps");
  //Serial.print("\tZ: "); Serial.print(g.gyro.z);      Serial.println(" dps");

  Serial.println();
  delay(200);
}
void printOrientation(float x, float y, float z)
{
  float pitch, roll;
  
  roll = atan2(x, sqrt(y * y) + (z * z));
  pitch = atan2(y, sqrt(x * x) + (z * z));
  roll *= 180.0 / M_PI;
  pitch *= 180.0 / M_PI;


  Serial.print(F("Roll: "));
  Serial.print(roll);
  Serial.print(F("; "));
  Serial.print(F("Pitch: "));
  Serial.print(pitch);
  Serial.print(F("; "));
  
  //Serial.print("Pitch, Roll: ");
  //Serial.print(pitch, 2);
  //Serial.print(", ");
  //Serial.println(roll, 2);
}

void printHeading(float hx, float hy)
{
  float heading;
  
  if (hy > 0)
  {
    heading = 90 - (atan(hx / hy) * (180 / M_PI));
  }
  else if (hy < 0)
  {
    heading = - (atan(hx / hy) * (180 / M_PI));
  }
  else // hy = 0
  {
    if (hx < 0) heading = 180;
    else heading = 0;
  }
  Serial.print(F("Heading: "));
  Serial.print(heading);
  Serial.print(F("; "));
    
  //Serial.print("Heading: ");
  //Serial.println(heading, 2);
}



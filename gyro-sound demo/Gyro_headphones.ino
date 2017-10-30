#include <Wire.h>
#include <DueTimer.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_L3GD20_U.h>
#include <Adafruit_9DOF.h>

#define SAMPLE_RATE     (16000)   // How many times the DAC will sample per second
#define TIME_STEP       (0.0000625) // based on sample rate, 1/SAMPLE_RATE
#define CHIRP_DURATION  (0.15)    // How long the chirp will last
#define PITCH_START     (200)     // What pitch the chirp will begin at
#define PITCH_END       (1000)    // What pitch the chirp will end at 

/* Assign a unique ID to the sensors */
Adafruit_9DOF                dof   = Adafruit_9DOF();
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(30301);
Adafruit_LSM303_Mag_Unified   mag   = Adafruit_LSM303_Mag_Unified(30302);

/* Update this with the correct SLP for accurate altitude measurements */
float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;
float RadOrien;
float RadInit = -1.0;

// Arduino DUE runs at 84 MHz
// divisor options:
// 1, 8, 64, 256, 1024
// We want 16 KHz interrupt rate
// Output cmopare at count 5250
// 
// Counter register count up to 32 bits

// Counter ranges from 0 to 16,0000
const unsigned int samples_dur = (unsigned int) ((double) SAMPLE_RATE * (double) CHIRP_DURATION);
const double w_step = (( (double) PITCH_END - (double) PITCH_START) / ((double) CHIRP_DURATION));
int t0_counter = 0;
double elapsed_time = 0.0;

double *chirp;

// Gain ranges from 0 to 1
double left_gain;
double right_gain; 

bool chirping = false;

// Timer 4 interrupts 16,000 times per second
void myHandler()
{ 
  if( t0_counter < samples_dur)
  {
    // Compute chirp DAC value
    unsigned int ear_r = chirp[t0_counter] * right_gain * 4095;
    unsigned int ear_l = chirp[t0_counter] * left_gain * 4095;
    // DAC0 is right ear
    //Serial.println(ear_r);
    //Serial.println("");
    //Serial.println(ear_l);
    analogWrite(DAC0, ear_r);

    // DAC1 is left ear
    analogWrite(DAC1, ear_l);
    // modify DAC value to output sine wave to both channels, dac0 and dac1
    elapsed_time = elapsed_time + TIME_STEP;
  }
  else if ((t0_counter >= samples_dur) && chirping)
  {
    analogWrite(DAC0, 0);
    analogWrite(DAC1, 0);
    chirping = false;
    // Turn off sound
  }
  
  t0_counter++;
  if( t0_counter >= SAMPLE_RATE)
  {
    t0_counter = 0;
    elapsed_time = 0.0;
    chirping = true;
  }
}

/**************************************************************************/
/*!
    @brief  Initialises all the sensors used by this example
*/
/**************************************************************************/
void initSensors()
{
  if(!accel.begin())
  {
    /* There was a problem detecting the LSM303 ... check your connections */
    Serial.println(F("Ooops, no LSM303 detected ... Check your wiring!"));
    while(1);
  }
  if(!mag.begin())
  {
    /* There was a problem detecting the LSM303 ... check your connections */
    Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
    while(1);
  }
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void setup(void)
{
  Serial.begin(115200);
  Serial.println(F("Adafruit 9 DOF Pitch/Roll/Heading Example")); Serial.println("");
  
  /* Initialise the sensors */
  initSensors();

  analogWriteResolution(12);

  Timer3.attachInterrupt(myHandler);
  Timer3.start(62.5);

  chirp = (double *) malloc(SAMPLE_RATE * CHIRP_DURATION * sizeof(double));
  if(chirp == NULL)
  {
    Serial.println( "CRITICAL ERROR :(");
    while(1);
  }
  // Get the current facing:
  sensors_event_t accel_event;
  sensors_event_t mag_event;
  sensors_vec_t   orientation;

  /* Calculate the heading using the magnetometer */
  Serial.println( "Finding current facing:");
  while(RadInit == -1)
  {
    mag.getEvent(&mag_event);
    if (dof.magGetOrientation(SENSOR_AXIS_Z, &mag_event, &orientation))
    {
      RadInit = (orientation.heading)*(PI/180);
    }
  }
  Serial.print( "Current facing is: ");
  Serial.println( RadInit);

  for(int i =0; i < (SAMPLE_RATE * CHIRP_DURATION); i++)
  {
    // Compute chirp frequency
    double w_t = PITCH_START + w_step * TIME_STEP * i;
    // Compute chirp sine-wave value
    
    chirp[i] = (double) (1.0 + sin( 2 * PI * w_t * TIME_STEP * i)) / 2.0; 
  }
  
  // Modify DAC 0 and DAC1 to output at 12 bit resolution
}

/**************************************************************************/
/*!
    @brief  Constantly check the roll/pitch/heading/altitude/temperature
*/
/**************************************************************************/
void loop(void)
{
  sensors_event_t accel_event;
  sensors_event_t mag_event;
  sensors_vec_t   orientation;

  /* Calculate the heading using the magnetometer */
  mag.getEvent(&mag_event);
  if (dof.magGetOrientation(SENSOR_AXIS_Z, &mag_event, &orientation))
  {
    /* 'orientation' should have valid .heading data now */
    //Serial.print(F("Heading: "));
    //Serial.print(orientation.heading);
    //Serial.print(F("; "));
    RadOrien = (orientation.heading)*(PI/180) - RadInit;
    //Serial.print(F("Heading in Rad: "));
    //Serial.print(RadOrien);
    //Serial.print(F("; "));
  }

  //Serial.println(F(""));

  // Compute gain HERE because this is when GAIN CHANGES
  // Should probably be lowered to <16 khz frequency ?
  right_gain = abs((sin((RadOrien + 45*PI/180)) + 1)/2);
  left_gain  = abs((cos((RadOrien + 45*PI/180)) + 1)/2);
  //Serial.print("Right: ");
  //Serial.print(right_gain);
  //Serial.print("Left: ");
  //Serial.println(left_gain);
  
  delay(100);
}


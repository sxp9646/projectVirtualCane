#include <Wire.h>
#include <DueTimer.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_L3GD20_U.h>
#include <Adafruit_9DOF.h>

#define SAMPLE_RATE     (16000)   // How many times the DAC will sample per second
#define TIME_STEP       (0.0000625) // based on sample rate, 1/SAMPLE_RATE
#define CHIRP_DURATION  (0.20)    // How long the chirp will last
#define PITCH_START     (200)     // What pitch the chirp will begin at
#define PITCH_END       (2000)    // What pitch the chirp will end at 

#define HEAD_RADIUS     (0.57)    // Radius of average male head in meters
#define SOUND_SPEED     (343.0)     // Speed of sound in some medium, meters/sec

/* Assign a unique ID to the sensors */
Adafruit_9DOF                dof   = Adafruit_9DOF();
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(30301);
Adafruit_LSM303_Mag_Unified   mag   = Adafruit_LSM303_Mag_Unified(30302);

/* Update this with the correct SLP for accurate altitude measurements */
float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;
float RadOrien;
float RadInit = -1.0;
float DegInit = -1.0;

// Arduino DUE runs at 84 MHz
// divisor options:
// 1, 8, 64, 256, 1024
// We want 16 KHz interrupt rate
// Output cmopare at count 5250
// 
// Counter register count up to 32 bits

// Counter ranges from 0 to 16,0000

const double beta = (2.0 * SOUND_SPEED / HEAD_RADIUS);
const double beta_sq = beta * beta;
const double head_sound_delay = HEAD_RADIUS / SOUND_SPEED;   //Delay of sound going over your head.
const unsigned int samples_dur = (unsigned int) ((double) SAMPLE_RATE * (double) CHIRP_DURATION);
const double w_step = (( (double) PITCH_END - (double) PITCH_START) / ((double) CHIRP_DURATION));
int t0_counter = 0;

int *t_delay;
double *chirp;
double *w_t;

// Gain ranges from 0 to 1
double left_gain, l_alpha;
double right_gain, r_alpha; 
unsigned int l_delay;
unsigned int r_delay;

bool chirping = false;

// Timer 4 interrupts 16,000 times per second
void myHandler()
{ 
  unsigned int ear_r = 0;
  unsigned int ear_l = 0;
  //double w_t, w_t_sq;
  //w_t = PITCH_START + w_step * TIME_STEP * (t0_counter);
  //w_t_sq = w_t[t0_counter] * w_t[t0_counter];

  // Compute chirp DAC value
  if((t0_counter - r_delay) >= 0 && (t0_counter - r_delay) < samples_dur)
  {
    // Approximation
    right_gain = (w_t[t0_counter - r_delay] * r_alpha + beta) / (w_t[t0_counter - r_delay] + beta);
    //w_t = PITCH_START + w_step * TIME_STEP * (t0_counter - r_delay);
    //w_t_sq = w_t * w_t;
    //right_gain = pow((beta_sq + w_t_sq * r_alpha), 2) + beta_sq * w_t_sq * pow(r_alpha,2);
    //right_gain = sqrt(right_gain) / (beta_sq + w_t_sq);
    ear_r = chirp[t0_counter - r_delay] * right_gain * 2047;
  }

  if((t0_counter - l_delay) >= 0 && (t0_counter - l_delay) < samples_dur)
  {
    left_gain = (w_t[t0_counter - l_delay] * l_alpha + beta) / (w_t[t0_counter - l_delay] + beta);
    //w_t = PITCH_START + w_step * TIME_STEP * (t0_counter - l_delay);
    //w_t_sq = w_t * w_t;
    //left_gain = pow((beta_sq + w_t_sq * l_alpha), 2) + beta_sq * w_t_sq * pow(l_alpha,2);
    //left_gain = sqrt(left_gain) / (beta_sq + w_t_sq);
    ear_l = chirp[t0_counter - l_delay] * left_gain * 2047;
  }
  // DAC0 is right ear
  analogWrite(DAC0, ear_r);

  // DAC1 is left ear
  analogWrite(DAC1, ear_l);
  // modify DAC value to output sine wave to both channels, dac0 and dac1
  
  t0_counter++;
  if( t0_counter >= SAMPLE_RATE)
  {
    t0_counter = 0;
  }
}

int time_delay(double degree)
{
  degree = abs(degree);
  
  if(degree > 360)
    degree = degree - 360;

  if(degree > 180)
    degree = 360-degree;

  if(degree > 180 || degree < 0)
  {
    Serial.println("I can't believe you've done this");
  }
  return t_delay[round(degree)];
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
  Timer3.start(1000000 * TIME_STEP);

  // 0 to 180
  t_delay = (int *) malloc( 181 * sizeof(int));
  if(t_delay == NULL)
  {
    Serial.println( "CRITICAL ERROR :(");
    while(1);
  }

  chirp = (double *) malloc(SAMPLE_RATE * CHIRP_DURATION * sizeof(double));
  w_t   = (double *) malloc(SAMPLE_RATE * CHIRP_DURATION * sizeof(double));
  if(chirp == NULL || w_t == NULL)
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
  while(DegInit == -1)
  {
    mag.getEvent(&mag_event);
    if (dof.magGetOrientation(SENSOR_AXIS_Z, &mag_event, &orientation))
    {
      DegInit = (orientation.heading);
      //RadInit = (orientation.heading)*(PI/180);
    }
  }
  Serial.print( "Current facing is: ");
  Serial.println( DegInit);

  // t_delay tells the processor how many cycles it will offset
  // the chirp from realtime (0 offset)
  for(int i=0; i<= 180; i++)
  {
    if ( i < 90)
    {
      t_delay[i] = round((head_sound_delay  * (1 - cos(i*PI/180.0))) / (TIME_STEP));
    }else
    {
      t_delay[i] = round((head_sound_delay  * (i*PI/180.0 + 1 - PI/2.0)) / (TIME_STEP));
    }
  }

  for(int i =0; i < (SAMPLE_RATE * CHIRP_DURATION); i++)
  {
    // Compute chirp frequency
    w_t[i] = PITCH_START + w_step * TIME_STEP * i;
    // Compute chirp sine-wave value
    
    chirp[i] = (double) (1.0 + sin( 2 * PI * w_t[i] * TIME_STEP * i)) / 2.0; 
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
  double DegOrien;
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
    DegOrien = (orientation.heading) - DegInit;
    RadOrien = DegOrien*(PI/180);
    //Serial.print(F("Heading in Rad: "));
    //Serial.print(RadOrien);
    //Serial.print(F("; "));
  }

  //Serial.println(F(""));

  r_delay = time_delay(DegOrien - 90);
  l_delay = time_delay(DegOrien + 90);
  //Serial.print("Right: ");
  //Serial.print(r_delay );
  //Serial.print("Left: ");
  //Serial.println(l_delay );

  // Compute gain HERE because this is when GAIN CHANGES
  // Should probably be lowered to <16 khz frequency ?
  //right_gain = abs((sin((RadOrien + 45*PI/180)) + 1)/2);
  //left_gain  = abs((cos((RadOrien + 45*PI/180)) + 1)/2);
  
  l_alpha =  1 + cos(RadOrien + PI / 2.0);
  r_alpha =  1 + cos(RadOrien - PI / 2.0);
  //Serial.print("Right: ");
  //Serial.print(right_gain);
  //Serial.print("Left: ");
  //Serial.println(left_gain);
  
  delay(100);
}


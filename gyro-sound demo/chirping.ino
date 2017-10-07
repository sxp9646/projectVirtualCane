#include <DueTimer.h>
#include <Wire.h>

#define SAMPLE_RATE     (16000)   // How many times the DAC will sample per second
#define TIME_STEP       (0.0000625) // based on sample rate, 1/SAMPLE_RATE
#define CHIRP_DURATION  (0.10)    // How long the chirp will last
#define PITCH_START     (200)     // What pitch the chirp will begin at
#define PITCH_END       (1000)    // What pitch the chirp will end at 

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
double left_gain = 1;
double right_gain = 1; 

bool chirping = true;

// Interrupt for timer 4
// Timer 4 interrupts 16,000 times per second
void myHandler()
{ 
  if( t0_counter < samples_dur)
  {
    // Compute chirp DAC value
    unsigned int ear_r = chirp[t0_counter] * right_gain * 4095;
    unsigned int ear_l = chirp[t0_counter] * left_gain * 4095;
    // DAC0 is right ear
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

*/
/**************************************************************************/
void setup(void)
{
  Serial.begin(115200);
  Serial.println(F("stuart burtner")); Serial.println("");

  analogWriteResolution(12);

  Timer3.attachInterrupt(myHandler);
  Timer3.start(62.5);

  chirp = (double *) malloc(SAMPLE_RATE * CHIRP_DURATION * sizeof(double));
  if(chirp == NULL)
  {
    Serial.println( "CRITICAL ERROR :(");
    while(1);
  }

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
  delay(200);
}

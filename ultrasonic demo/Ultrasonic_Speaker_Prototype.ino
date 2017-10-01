

// Pin ""defines""
const int trigPin = 2;
const int echoPin = 4;
const int speaker = 6;

// Probably shouldn't be global - sonar variables
long duration;
int distance;

// Really sloppy 5-datapoint-averager
double data[] = {0.0,0.0,0.0,0.0,0.0};
int index = 0;

//Timer variables
int counter = 0;              // Counts how many times interupt has occured (~1KHz frequency)
int freq_overflow = 1000;     // How many counts before reseting the timer
int freq_toneStop = 1000/3;   // How many counts before turning off the tone
bool tone_active = true;      // Current state of tone
bool freq_enable = true;

// Interrupt is called once a millisecond (mostly)
SIGNAL(TIMER0_COMPA_vect) 
{
  if(counter >= freq_overflow)
  {
    // When out of range - disable the speaker
    // but keep counting
    if(freq_enable == true)
    {
      tone(speaker, 440);
      tone_active = true;
    }
    counter = 0;
  }
  if((tone_active == true) && (counter >= freq_toneStop))
  {
    noTone(speaker);
    tone_active = false;
  }
  counter++;
}

void setup(){
  // Use timer 0 as our interrupt pin
  // This works because we're just going to assume the timer
  // based interrupts are occuring once every millisecond
  OCR0A = 0x00;
  TIMSK0 |= _BV(OCIE0A);

  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);
  Serial.begin(9600);
}

void add(double in){
  data[index] = in;
  index = index + 1;
  if (index > 5)
    index = 0;
}

double avg(){
  double avg = 0;
  for(int i=0;i<5;i++)
  {
    avg += data[i];
  }
  return (avg/5);
}

void dist()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance= duration*0.034/2;
  add(distance);
}

// Sets the frequency at which the speaker will produce sound
void freq(double hz)
{
  // Sets the counter to overflow at the desired rate (minium once every 2 seconds)
  freq_overflow = 1000/max(hz,0.5);
  freq_toneStop = freq_overflow/2;  
}

void loop(){
  double val;

  //Gather and record distance into data array then get average of distance values
  dist();
  val = avg();

  // "transforms" the distance value into a HZ frequency
  // You may change this as much as you wish.
  // Maps 100 distance to 0 HZ and 1 distance to 10 Hz
  val = 10-(min(val,95)/10);
  freq(val);
  //Serial.println(val);
  
  //if the value was < 1 HZ, then don't play noise at all
  if(val > 1)
  {
    freq_enable = true;
  }
  else
  {
    freq_enable = false;
  }
  //Serial.println(freq_enable);
  
  // I *STRONGLY* recommend we not over-stress the poor ultrasonic sensor
  delay(2);
}

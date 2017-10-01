

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

void setup(){
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

void loop(){
  double val;
  int pitch;
  //Gather and record distance into data array then get average of distance values
  dist();
  val = avg();

  // At large distances, plays a low pitch D
  // At small distances, plays a high pitch D
  pitch = 294 + 294*((100.0-min(val,100))/100);
  tone(speaker, pitch);

  // (Always plays a pitch)
  // noTone(speaker);
  
  // I *STRONGLY* recommend we not over-stress the poor ultrasonic sensor
  delay(2);
}

const int trig = 9; //send the wave
const int echo = 10; //recieve the wave
const int led = 3; //Light the LED
double SoundSpeed = .0343; //speed of sound in centimeters per microsecond

long duration; //the time it takes for the signal to return;
double distance; //I'm using AI for help since I'm a beginner and this is how I learned java. It said to use an int but I feel a long works better


void setup() {
  pinMode(trig, OUTPUT); //from the arduino's perspective, it is outputting a signal into the trigger 
  pinMode(echo, INPUT); //and recieving a signal from the echo
  pinMode(led, OUTPUT);

  Serial.begin(9600); //Serial is basically talking to the arduino and 9600 is the "baud rate" which is how quickly the computer sends and recieves signals from the arduino
}

void loop() {
  digitalWrite(trig, LOW); //No signal sent
  delayMicroseconds(2);
  digitalWrite(trig, HIGH); //Send signals (sends 8)
  delayMicroseconds(10);
  digitalWrite(trig, LOW); //Stop sending

  duration = pulseIn(echo, HIGH); //internally the ultrasonic sensor will turn echo high when trig sends a wave and turns it low when echo recieves a wave
  distance = SoundSpeed * duration / 2; //some math to turn time into distance

  if (distance > 50) {
    distance = 50;
  } //for the map I need a max value and I just want the logic to be 100% sound
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print("\n"); //new line character
  delay(100);//so it doesn't run to fast and think the distance is 0

  int brightness = map(distance, 0, 50, 255, 0); //variable, lowest value, highest value, new lowest value, new highest value. Should flip so the highest value is the new lowest value vice versa

  analogWrite(led, brightness);

}

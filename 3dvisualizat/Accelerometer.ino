#include "I2Cdev.h"
#include "MPU6050.h"
#include "math.h"
#include "Adafruit_VL53L0X.h"

Adafruit_VL53L0X tof = Adafruit_VL53L0X();
MPU6050 mpu(0x68);

int16_t ax, ay, az;
int16_t gx, gy, gz;
float last_gx, last_gy, last_gz, last_ax, last_ay, last_az, new_ax, new_ay, new_az, new_gx, new_gy, new_gz;
float dx, dy, dz, px, vx, steady, vdy, lastvdy, usdz, vdz, lastusdz, steadily, calculatedx, calculatedy, calculatedz;
int ldy, lastldy, reset;
long duration;
bool pressed;
double timer, elapsed, dt;
const int echo = 3;
const int trig = 5;
const int btn = 6;
double SoundSpeed = .0343;
VL53L0X_RangingMeasurementData_t measure;

//the following two comments are from me trying to do the math and are I believe very WRONG

//z changes based on x (yaw) and z (roll)
//y changes nased on y (pitch) and z (roll)

class EmaFilter { //using this bad boy again, puts exponential filter between data points so if there is a massive jump in the data it won't completely ruin everything
  public:
      EmaFilter(double alpha) : m_alpha(alpha), m_lastOutput(0.0) {}

      double Run(double input)
      {
          m_lastOutput = m_alpha * input + (1 - m_alpha) * m_lastOutput;
          return m_lastOutput;
      }
  private:
      double m_alpha;
      double m_lastOutput;
};

EmaFilter emaFilter(0.125);
EmaFilter emAFilter(0.125); //the objects hold the last few data points so I need two different objects that way they don't mix data

void setup() {
  Wire.begin();
  Serial.begin(115200); //this is a higher Baud rate so that the things run faster, I need the code to run as fast as possible so that it can be accurate. Of course it can't run too fast but the ultrasonic sensor fixes that
  while(!Serial); //Serial monitor would take too long to open and data would be thrown to the void causing the code to never actually run
  mpu.initialize();

  if(mpu.testConnection() == false){
    Serial.println("MPU6050 connection failed");
    while(mpu.testConnection() == false); //this way I don't have to redeploy, unplugging and plugging the arduino back in usually fixes it
  }
  if(!tof.begin()) { //tof stands for time of flight, the module is called a lazar ranging module but better known as a time of flight module, in my opinion it seems to be more accurate than the ultrasonic sensor
    Serial.print("VL53L0X connection failed");
    while(!tof.begin());
  }
  pinMode(trig, OUTPUT); //send data to the trigger pin 
  pinMode(echo, INPUT); //recieve data from the echo pin
  pinMode(btn, INPUT_PULLUP); //initially high but if a pullup resistor changes it goes to low, recieve data
  mpu.setDLPFMode(MPU6050_DLPF_BW_20); //run as fast as possible
  mpu.setXAccelOffset(19600/8 - 3200/8 + 400/8 - 400/8); //don't mind, calibration is hard, when calibrating for some reason it automatically multiplies by about 8 so I have to divide by 8
  mpu.setYAccelOffset(-5900/8 + 1600/8 - 800/8 + 100/8);
  mpu.setZAccelOffset(-10000/8 + 1800/8 - 400/8 + 100/8);
  mpu.setXGyroOffset(610/8 + 320/8 + 160/8 + 80/4);
  mpu.setYGyroOffset(-170/8 - 100/4 + 20/8);
  mpu.setZGyroOffset(50/8 + 30/4);
  digitalWrite(trig, LOW); // if you see my previous code, I moved this here because it's redundant to run it every time if it is already LOW from the previous loop
  timer = micros(); //dt isn't 0 but also makes sure that the time it takes for the loop to start is accounted for in the math
}

void loop() {
  dt = micros() - timer; //change in time since last loop
  timer = micros();
  tof.rangingTest(&measure, false); //checks if it's ranging properly, put true to get an actual value
  ldy = measure.RangeMilliMeter; //I lost track of variables because I kept adding and removing variables that were technically the same thing after realizing how inaccurate the mpu is so ldy stands for lazar distance y
  reset = digitalRead(btn); //if the button is pressed reset will be LOW, else HIGH

  if (reset == LOW && pressed == false) { //including the boolean "pressed" so that it doesn't keep resetting while the button is pressed, idk if this was a good idea and it might actually be better if it didn't have it but for safety I'm keeping it
    pressed = true;
    dx -= dx; //the same as just setting it to 0 idk why I did it this way but it made sense in my head at the time of coding it
    dy -= dy;
    dz -= dz;
  }
  else if (reset == HIGH && pressed == true) pressed = false;

  
  if (ldy > 2000 || ldy < 50) ldy = steady; //the approximate minimum value of 50 comes from what it was reading when I was touch the sensor and 2000 is an arbitrary number that is less than its out of bounds return value of ~8000 but greater than any value I will probably have it read
  else {
    steady = emaFilter.Run(ldy); // make sure there are no random huge jumps while not allowing 0 to be added to the filter nor the same number to clutter the filter
  }
  
  if (dt != 0) {
    vdy = ((ldy - lastldy)) / (dt / 1000); //change in distance over change in time, a simple "AROC" or average rate of change equation
  }
  if (fabs(vdy) < .25) vdy = 0; //it jitters a lot, this removes that small bit of noise from say my slight hand shaking while testing

  lastvdy = vdy;
  lastldy = ldy;
  digitalWrite(trig, HIGH); //Send signals (sends 8)
  delayMicroseconds(10); //necessary for the ultrasonic sensor to have time to get enough data that it knows the distance, it does not use just one data point for each value it sends to the code
  digitalWrite(trig, LOW); //Stop sending

  duration = pulseIn(echo, HIGH, 20000); //the 20 ms time cap seems to censor out out of range things which is fantastic, the final number is the cap in how long it will wait for a wave to come back in microseconds, echo is just which pin, HIGH means it's reading when ECHOs high
  usdz = SoundSpeed * duration / 2 / 100; //some math to turn time into distance, since we know how fast the wave is traveling at


  vdz = (usdz - lastusdz) / (dt / 1000000); //usdz stands for ultrasonic sensor distance Z, vdZ does not account for rotation
  lastusdz = usdz;
  
  if (fabs(vdz) < .15) vdz = 0; //a deadband to ignore slight shakes like before
  if (usdz != 0 && usdz < 2) { //filter out 0, in reality I could probably ignore when it is less than 5 centimeters or .05 since that's about what touching it gets but when you actually touch it it jumps to its max value so
    steadily = emAFilter.Run(usdz);
  }
  
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz); //this gets all 6 axes that the mpu can get without a magnetometer attached to it
  float virtual_x = az / 16384.0 * 9.81; // the division converts it into g forces then the multiplication is the approximate meters per second that one g force is turning it into the SI for acceleration
  float virtual_y = ax / 16384.0 * 9.81;
  float virtual_z = ay / 16384.0 * 9.81;
  float real_gx = gz / 131.0; // turns it from raw data to degrees per second
  float real_gy = gx / 131.0;
  float real_gz = gy / 131.0;

  new_ax = sensor_reading(virtual_x); //its just a deadband that I messed up initially and was too lazy to simplify
  last_ax = virtual_x; //I had a reason for this I think it might have been velocity I don't remember
  virtual_x = new_ax;
  new_ay = sensor_reading(virtual_y);
  last_ay = virtual_y;
  virtual_y = new_ay;
  new_az = sensor_reading(virtual_z);
  last_az = virtual_z;
  virtual_z = new_az;
  new_gx = dps(real_gx);
  last_gx = real_gx;
  real_gx = new_gx;
  new_gy = dps(real_gy);
  last_gy = real_gy;
  real_gy = new_gy;
  new_gz = dps(real_gz);
  last_gz = real_gz;
  real_gz = new_gz;

  if (fabs(real_gx) > 0) {
    dx += (real_gx * dt / 1000000.0); //dt is in microseconds and I need it to be in seconds. Degrees adds the integral of the degrees per second over the time it takes for one loop to happen
  }

  if (fabs(real_gy) > 0) {
    dy += (real_gy * dt / 1000000.0);
  }

  if (fabs(real_gz) > 0) {
    dz += (real_gz * dt / 1000000.0);
  }

  if (fabs(virtual_x) > 0) {
    vx += (virtual_x * dt / 1000000.0); //this was super innacurate and is left here to show I tried it
  }
  else{
    vx = 0;
  }
  px += (vx * dt / 1000000.0); //I am overwriting this with a flat floor assumption and 2 other sensors but since this is practice I will keep this here. 
  //Basically since the IMU is a not very accurate accelerometer, velocity is basically impossible to measure because if it is moving at a steady speed, acceleration is 0, then the sensor drift filtering over 2 integrals leads to being very inaccurate. 
  //I am instead bringing back the ultrasonic sensor for the left to right axis and a lazar ranging module for up and down translation. I will assume it doesn't move forward and backward unless it is rotated where I will use trigonometry to track movement in each direction. Assume a mecanum drive where it can't move up and down unless a ramp is introduced.
  //I will use the MPU6050 for acceleration and sensor delay correction but will use the 2 position sensors for position and derive them for approximate velocity

  float rz = dx * PI / 180.0; // Yaw (dx), converts from degrees to radians
  float rx = dy * PI / 180.0; // Roll (dy)
  float ry = dz * PI / 180.0; // Pitch (dz)

  float cz = cos(rx), sz = sin(rx); //this is how 3D matrices work, they aren't fun to read normally and I'm not going to try to make it easier to read. Just trust that the math is about correct, might be slightly off but it's close enough.
  float cx = cos(ry), sx = sin(ry);
  float cy = cos(rz), sy = sin(rz);

  calculatedx = (steady / 1000) * (cz * sy * sx - sz * cx) + steadily * (cz * sy * -cx + sz * sx); //this gets a steady position accounting for rotation and not just trusting that the sensor will always get the axis I set it to face on the breadboard

  calculatedy = -(steady / 1000) * (sz * sy * sx + cz * cx) + steadily * (-sz * sy * cx - cz * sx);

  calculatedz = -(steady / 1000) * (cy * sx) + steadily * (cy * cx); //probably should have just divided by 1000 when defining steady but oh well, steady is the lazar sensor if you don't remember and steadily is the ultrasonic sensor after they are run through the EMAfilters

  //I got lost in the math, I think it works tho0ugh

  //heres all my debugging code, it was a lot to understand exactly how it all worked

  /* Serial.print("vdy: "); Serial.println(vdy);
  Serial.println(steady / 1000);
  Serial.print("dz: "); Serial.println(steadily);
  Serial.print("vdz: "); Serial.println(vdz);
  Serial.print(virtual_x);
  Serial.print(", ");
  Serial.print(virtual_y);
  Serial.print(", ");
  Serial.print(virtual_z);
  Serial.print(", ");
  Serial.print(real_gx);
  Serial.print(", ");
  Serial.print(real_gy);
  Serial.print(", ");
  Serial.print(real_gz);
  Serial.print(", vx: ");
  Serial.print(vx);
  Serial.print(", px: ");
  Serial.print(px);
  Serial.print(", dgx: ");
  Serial.print(dx);
  Serial.print(", dgy: ");
  Serial.print(dy);
  Serial.print(", dgz: ");
  Serial.println(dz);
  Serial.print(", offsetx: "); */

  //organized data, check the other file to see why I organized it this way. It goes from position and then each derivative that I have
  Serial.print(calculatedx);
  Serial.print(",");
  Serial.print(calculatedy);
  Serial.print(",");
  Serial.print(calculatedz);
  Serial.print(",");
  Serial.print(dx);
  Serial.print(",");
  Serial.print(dy);
  Serial.print(",");
  Serial.print(dz);
  Serial.print(",");
  Serial.print(vdy);
  Serial.print(",");
  Serial.print(vdz);
  Serial.print(",");
  Serial.print(real_gx);
  Serial.print(",");
  Serial.print(real_gy);
  Serial.print(",");
  Serial.print(real_gz);
  Serial.print(",");
  Serial.print(virtual_x);
  Serial.print(",");
  Serial.print(virtual_y);
  Serial.print(",");
  Serial.println(virtual_z);
}

//I know this is redundant, it wasn't when I wrote it I just wrote it very poorly and I don't feel like going through and cleaning it up

float sensor_reading(float current) {
  if (fabs(current / 9.81) < .1) {
    return 0;
  }
  else {
    return current;
  }
}
float dps(float current) {
  if (fabs(current) < 1) {
    return 0;
  }
  else {
    return current;
  }
}

//here are my sources, plus I had to learn how processing works but if you just download (many examples) or search it up its pretty self-explanatory
//https://github.com/ElectronicCats/mpu6050/blob/master/examples/MPU6050_raw/MPU6050_raw.ino
//https://github.com/adafruit/Adafruit_VL53L0X/blob/master/examples/vl53l0x/vl53l0x.ino
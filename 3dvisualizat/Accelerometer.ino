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
int ldy, lastldy;
long duration;
double timer, elapsed, dt;
const int echo = 3;
const int trig = 5;
double SoundSpeed = .0343;
VL53L0X_RangingMeasurementData_t measure;

//z changes based on x (yaw) and z (roll)
//y changes nased on y (pitch) and z (roll)

class EmaFilter { //using this bad boy again
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
EmaFilter emAFilter(0.125);

void setup() {
  Wire.begin();
  Serial.begin(115200);
  while(!Serial);
  mpu.initialize();

  if(mpu.testConnection() == false){
    Serial.println("MPU6050 connection failed");
    while(mpu.testConnection() == false);
  }
  if(!tof.begin()) {
    Serial.print("VL53L0X connection failed");
    while(!tof.begin());
  }
  pinMode(trig, OUTPUT); 
  pinMode(echo, INPUT); 
  mpu.setDLPFMode(MPU6050_DLPF_BW_20);
  mpu.setXAccelOffset(19600/8 - 3200/8 + 400/8 - 400/4); //don't mind, calibration is hard, when calibrating for some reason it automatically multiplies by about 8 so I have to divide by 8
  mpu.setYAccelOffset(-5900/8 + 1600/8 - 800/8 + 100/8);
  mpu.setZAccelOffset(-10000/8 + 1800/8 - 400/8 + 100/8);
  mpu.setXGyroOffset(610/8 + 320/8 + 160/8 + 80/4);
  mpu.setYGyroOffset(-170/8 - 100/4 + 20/8);
  mpu.setZGyroOffset(50/8 + 30/4);
  digitalWrite(trig, LOW);
  timer = micros();
}

void loop() {
  dt = micros() - timer;
  timer = micros();
  tof.rangingTest(&measure, false); //checks if it's ranging properly, put true to get an actual value
  ldy = measure.RangeMilliMeter;

  
  if (ldy > 2000 || ldy < 50) ldy = steady;
  else {
    steady = emaFilter.Run(ldy); // make sure there are no random huge jumps 
  }
  
  if (dt != 0) {
    vdy = ((ldy - lastldy)) / (dt / 1000);
  }
  if (fabs(vdy) < .25) vdy = 0;

  lastvdy = vdy;
  lastldy = ldy;
  digitalWrite(trig, HIGH); //Send signals (sends 8)
  delayMicroseconds(10);
  digitalWrite(trig, LOW); //Stop sending

  duration = pulseIn(echo, HIGH, 20000); //the 20 ms time cap seems to censor out out of range things which is fantastic
  usdz = SoundSpeed * duration / 2 / 100; //some math to turn time into distance


  vdz = (usdz - lastusdz) / (dt / 1000000);
  lastusdz = usdz;
  
  if (fabs(vdz) < .15) vdz = 0;
  if (usdz != 0 && usdz < 2) {
    steadily = emAFilter.Run(usdz);
  }
  
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  float virtual_x = az / 16384.0 * 9.81;
  float virtual_y = ax / 16384.0 * 9.81;
  float virtual_z = ay / 16384.0 * 9.81;
  float real_gx = gz / 131.0;
  float real_gy = gx / 131.0;
  float real_gz = gy / 131.0;

  new_ax = sensor_reading(virtual_x);
  last_ax = virtual_x;
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
    dx += (real_gx * dt / 1000000.0);
  }

  if (fabs(real_gy) > 0) {
    dy += (real_gy * dt / 1000000.0);
  }

  if (fabs(real_gz) > 0) {
    dz += (real_gz * dt / 1000000.0);
  }

  if (fabs(virtual_x) > 0) {
    vx += (virtual_x * dt / 1000000.0);
  }
  else{
    vx = 0;
  }
  px += (vx * dt / 1000000.0); //I am overwriting this with a flat floor assumption and 2 other sensors but since this is practice I will keep this here. 
  //Basically since the IMU is a not very accurate accelerometer, velocity is basically impossible to measure because if it is moving at a steady speed, acceleration is 0, then the sensor drift filtering over 2 integrals leads to being very inaccurate. 
  //I am instead bringing back the ultrasonic sensor for the left to right axis and a lazar ranging module for up and down translation. I will assume it doesn't move forward and backward unless it is rotated where I will use trigonometry to track movement in each direction. Assume a mecanum drive where it can't move up and down unless a ramp is introduced.
  //I will use the MPU6050 for acceleration and sensor delay correction but will use the 2 position sensors for position and derive them for approximate velocity

  float rz = dx * PI / 180.0; // Yaw (dx)
  float rx = dy * PI / 180.0; // Roll (dy)
  float ry = dz * PI / 180.0; // Pitch (dz)

  float cz = cos(rx), sz = sin(rx);
  float cx = cos(ry), sx = sin(ry);
  float cy = cos(rz), sy = sin(rz);

  calculatedx = (steady / 1000) * (cz * sy * sx - sz * cx) + steadily * (cz * sy * -cx + sz * sx);

  calculatedy = -(steady / 1000) * (sz * sy * sx + cz * cx) + steadily * (sz * sy * cx - cz * sx);

  calculatedz = -(steady / 1000) * (cy * sx) + steadily * (cy * cx);

  //I got lost in the math, I think it works tho0ugh

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
  Serial.println(dz);

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
//https://github.com/ElectronicCats/mpu6050/blob/master/examples/MPU6050_raw/MPU6050_raw.ino
//https://github.com/adafruit/Adafruit_VL53L0X/blob/master/examples/vl53l0x/vl53l0x.ino
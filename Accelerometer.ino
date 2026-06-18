#include "I2Cdev.h"
#include "MPU6050.h"
#include "math.h"

MPU6050 mpu(0x68);

const int INT = 3;

int16_t ax, ay, az;
int16_t gx, gy, gz;
float last_gx, last_gy, last_gz, last_ax, last_ay, last_az, new_ax, new_ay, new_az, new_gx, new_gy, new_gz;
float dx, dy, dz, px, py, pz, change_ax, change_ay, change_az, change_gx, change_gy, change_gz;

void setup() {
  Wire.begin();
  Serial.begin(9600);
  while(!Serial);
  Serial.println("hi");
  mpu.initialize();

  if(mpu.testConnection() ==  false){
    Serial.println("MPU6050 connection failed");
    for(;;);
  }
  mpu.setDLPFMode(MPU6050_DLPF_BW_20);
  mpu.setXAccelOffset(19600/8 - 3200/8 + 400/8); //don't mind, calibration is hard, when calibrating for some reason it automatically multiplies by about 8 so I have to divide by 8
  mpu.setYAccelOffset(-5900/8 + 1600/8 - 800/8 + 100/8);
  mpu.setZAccelOffset(-10000/8 + 1800/8 - 400/8 + 100/8);
  mpu.setXGyroOffset(610/8 + 320/8 + 160/8 + 80/4);
  mpu.setYGyroOffset(-170/8 - 100/4 + 20/8);
  mpu.setZGyroOffset(50/8 + 30/4);
}

void loop() {
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  float virtual_x = az / 16384.0;
  float virtual_y = ax / 16384.0;
  float virtual_z = ay / 16384.0; //in terms of g forces
  float real_gx = gx / 131.0;
  float real_gy = gy / 131.0;
  float real_gz = gz / 131.0;

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
    float time = micros() - change_gx; //time since last update
    dx += (real_gx * time / 1000000);
    change_gx = micros();
  }
  else {
    change_gx = micros();
  }

  if (fabs(real_gy) > 0) {
    float time = micros() - change_gy; //time since last update
    dy += (real_gy * time / 1000000);
    change_gy = micros();
  }
  else {
    change_gy = micros();
  }

  if (fabs(real_gz) > 0) {
    float time = micros() - change_gz; //time since last update
    dz += (real_gz * time / 1000000);
    change_gz = micros();
  }
  else {
    change_gz = micros();
  }

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
  Serial.print(", dgx: ");
  Serial.print(dx);
  Serial.print(", dgy: ");
  Serial.print(dy);
  Serial.print(", dgz: ");
  Serial.println(dz);
  delay(5);
}

float sensor_reading(float current) {
  if (fabs(current) < .1) {
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
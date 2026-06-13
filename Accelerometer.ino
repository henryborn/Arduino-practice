#include "I2Cdev.h"
#include "MPU6050.h"
#include "math.h"

MPU6050 mpu(0x68);

const int INT = 3;

int16_t ax, ay, az;
int16_t gx, gy, gz;
float last_gx, last_gy, last_gz, last_ax, last_ay, last_az, new_ax, new_ay, new_az, new_gx, new_gy, new_gz;

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
  mpu.setYGyroOffset(-170/8 - 100/4);
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
  new_ax = sensor_reading(last_ax, virtual_x);
  last_ax = virtual_x;
  virtual_x = new_ax;
  new_ay = sensor_reading(last_ay, virtual_y);
  last_ay = virtual_y;
  virtual_y = new_ay;
  new_az = sensor_reading(last_az, virtual_z);
  last_az = virtual_z;
  virtual_z = new_az;
  new_gx = sensor_reading(last_gx, real_gx);
  last_gx = real_gx;
  real_gx = new_gx;
  new_gy = sensor_reading(last_gy, real_gy);
  last_gy = real_gy;
  real_gy = new_gy;
  new_gz = sensor_reading(last_gz, real_gz);
  last_gz = real_gz;
  real_gz = new_gz;
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
  Serial.println(real_gz);
  delay(100);
}

float sensor_reading(float last, float current) {
  if (fabs(current - last) < .1) {
    return 0;
  }
  else {
    return current;
  }
}

//https://github.com/ElectronicCats/mpu6050/blob/master/examples/MPU6050_raw/MPU6050_raw.ino
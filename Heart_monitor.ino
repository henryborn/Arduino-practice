#include <Wire.h>
#include "MAX30105.h"
#include <stdio.h>
#include <cstdint>
#include "spo2_algorithm.h"
//blood oxygen level was too hard and annoying

MAX30105 particleSensor;
double lastBeat;
const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot;
int beatAvg;
long counter = 0;
long IRAvg = 0;
long start;
bool heartBeat = false;
int32_t spo2;
int32_t heartRate;
int8_t validHeartRate;
int8_t validSPO2;

class EmaFilter { //this is an exponential filter kind of thing. Alpha sets the cutoff frequency
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

EmaFilter emaFilter(0.1125);

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; 
  }

  // Initialize sensor
  if (particleSensor.begin() == false) {
    Serial.println("MAX30102 was not found. Please check wiring/power.");
    while (1)
      ;
  }
  particleSensor.begin(Wire, I2C_SPEED_FAST);
  particleSensor.setup(0x1F, RATE_SIZE, 2, 400, 411, 4096); 

}

void loop() {
  uint32_t redBuffer[100];
  uint32_t irBuffer[100];
  long IR = particleSensor.getIR();
  if (IR > 10000) {
    int bufferLength = 100; //buffer length of 100 stores 4 seconds of samples running at 25sps
    for (byte i = 0 ; i < bufferLength ; i++)
      {
        counter++;
        long Red = particleSensor.getRed();
        IR = particleSensor.getIR();
        long steady = emaFilter.Run(IR);
      while (particleSensor.available() == false) //do we have new data?
        particleSensor.check(); //Check the sensor for new data
      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      particleSensor.nextSample(); //We're finished with this sample so move to next sample
      if (IR - steady < -20 && !heartBeat) {
        heartBeat = true;
        double error = millis() - lastBeat;
        lastBeat = millis();

        long BPM = 60.0 / (error / 1000.0);
        if (BPM > 40) {
          rates[rateSpot++] = (byte)BPM;
          rateSpot %= RATE_SIZE;
          beatAvg = 0;
          for (byte x = 0; x < RATE_SIZE; x++) beatAvg += rates[x];
          beatAvg /= RATE_SIZE;
        }

        double temp = particleSensor.readTemperatureF();
        Serial.print("Temp:");
        Serial.print(temp);
        Serial.print(", BPM:");
        Serial.print(beatAvg);

        maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
        Serial.print(F(", SPO2="));
        Serial.println(spo2, DEC);
      }
      else if (IR - steady > 20 && heartBeat) heartBeat = false;
    }
  }
}
//https://github.com/sparkfun/SparkFun_MAX3010x_Sensor_Library/tree/master good luck if you wanna do this too. 5 and a half hours
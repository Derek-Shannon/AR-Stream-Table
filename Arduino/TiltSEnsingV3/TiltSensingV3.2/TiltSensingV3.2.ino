#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <math.h>

Adafruit_MPU6050 mpu;

//Filter variables
float filteredX = 0;
float filteredY = 0;
float filteredZ = 0;
float alpha = 0.1; //Adjust this (0.05 to 0.5) to balance smoothness vs lag

//Averaging variables
float sumPitch = 0;
float sumRoll = 0;
int sampleCount = 0;
unsigned long lastReportTime = 0;
const unsigned long reportInterval = 500; // 0.5 seconds

void setup() {
  Serial.begin(115200);
  
  // Initialize the sensor
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) { delay(10); }
  }

  Serial.println("MPU6050 Found!");
  //Set filter with a baseline reading
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  filteredX = a.acceleration.x;
  filteredY = a.acceleration.y;
  filteredZ = a.acceleration.z;
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  //Low-Pass Filter formula
  filteredX = (alpha * a.acceleration.x) + ((1.0 - alpha) * filteredX);
  filteredY = (alpha * a.acceleration.y) + ((1.0 - alpha) * filteredY);
  filteredZ = (alpha * a.acceleration.z) + ((1.0 - alpha) * filteredZ);

  //calculate tilt angle
  float roll = atan2(filteredY, -filteredZ) * 180.0 / M_PI;
  float pitch = atan2(-filteredX, sqrt(filteredY * filteredY + filteredZ * filteredZ)) * 180.0 / M_PI;

  //add for the average
  sumPitch += pitch;
  sumRoll += roll;
  sampleCount++;

  //send to output
  if (millis() - lastReportTime >= reportInterval) {
    float avgPitch = sumPitch / sampleCount;
    float avgRoll = sumRoll / sampleCount;

    //send data
    Serial.print("Pitch: ");
    Serial.print(avgPitch, 2); //2 decimal places
    Serial.print(" , Roll: ");
    Serial.println(avgRoll, 2);

    //reset counts
    sumPitch = 0;
    sumRoll = 0;
    sampleCount = 0;
    lastReportTime = millis();
  }

  delay(10); //100 samples per second
}
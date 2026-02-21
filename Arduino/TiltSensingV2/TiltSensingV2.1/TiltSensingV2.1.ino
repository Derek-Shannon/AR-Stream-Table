#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;

//Filter variables
float filteredX = 0;
float filteredY = 0;
float filteredZ = 0;
float alpha = 0.1; //Adjust this (0.05 to 0.5) to balance smoothness vs lag

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

  // Send to Computer
  Serial.print(round(filteredX*10));
  Serial.print(",");
  Serial.print(round(filteredY*10));
  Serial.print(",");
  Serial.println(round(filteredZ*10));

  delay(100);
}
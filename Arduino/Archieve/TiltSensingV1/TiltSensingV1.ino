#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

//Filter variables
float filteredX = 0;
float filteredY = 0;
float alpha = 0.2; //Adjust this (0.05 to 0.5) to balance smoothness vs lag

void setup() {
  Serial.begin(9600);
  if(!accel.begin()){
    Serial.println("No sensor found");
    while(1);
  }

  //Set filter with a baseline reading
  sensors_event_t event;
  accel.getEvent(&event);
  filteredX = event.acceleration.x;
  filteredY = event.acceleration.y;
}

void loop() {
  sensors_event_t event;
  accel.getEvent(&event);

  //Low-Pass Filter formula
  filteredX = (alpha * event.acceleration.x) + ((1.0 - alpha) * filteredX);
  filteredY = (alpha * event.acceleration.y) + ((1.0 - alpha) * filteredY);

  // Send to Computer
  Serial.print(filteredX);
  Serial.print(",");
  Serial.println(filteredY);

  delay(20); //Fast sampling(50Hz)
}
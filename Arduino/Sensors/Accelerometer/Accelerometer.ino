#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>

#define LIMIT_ACC 8

//LSM303DLHC - GY-511 on aliexpress 
//SDA is A4 on Nano
//SCL is A5 on Nano
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);
float a, new_a;

void setup(void){
  
  Serial.begin(115200);
  accel.begin();

  sensors_event_t event;
  accel.getEvent(&event);
  float ax = event.acceleration.x;
  float ay = event.acceleration.y;
  float az = event.acceleration.z;
  a = sqrt(pow(ax, 2) + pow(ay, 2) + pow(az, 2));
  
}

void loop(void){
  
  sensors_event_t event;
  accel.getEvent(&event);
  float new_ax = event.acceleration.x;
  float new_ay = event.acceleration.y;
  float new_az = event.acceleration.z;
  new_a = sqrt(pow(new_ax, 2) + pow(new_ay, 2) + pow(new_az, 2));
  Serial.println(new_a - a);

  /*
  if (abs(new_a - a) >= LIMIT_ACC)
    notify();
  */

  a = new_a;

  delay(50);
}

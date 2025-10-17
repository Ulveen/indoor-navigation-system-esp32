#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;

float zAngle = 0.0;
unsigned long lastTime = 0;

void setup(void) {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println("MPU6050 Z-Axis Rotation Tracker");

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setGyroRange(MPU6050_RANGE_500_DEG);

  lastTime = millis();
}

void loop() {
  unsigned long currentTime = millis();
  float dt = (currentTime - lastTime) / 1000.0;
  lastTime = currentTime;
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float zRate = g.gyro.z;

  zAngle += (zRate * 180.0 / M_PI) * dt;

  if (zAngle >= 360.0) {
    zAngle -= 360.0;
  } else if (zAngle < 0.0) {
    zAngle += 360.0;
  }

  Serial.print("Z Angle: ");
  Serial.print(zAngle);
  Serial.println(" degrees");

  delay(10);
}
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>
#define MOTOR_DUTY_CYCLE 255
#define MOTOR_FREQUENCY 30000
#define MOTOR_RESOLUTION 8
#define LINEAR_MS 300

enum MotorState {
  FORWARD,
  REVERSE,
  STOP
};

enum Direction {
  FRONT,
  RIGHT,
  BACK,
  LEFT
} currDirection = BACK;

struct MotorPin {
  int pin1;
  int pin2;
  int enablePin;
  int pwmChannel;
} leftMotor = { 27, 26, 14, 0 }, rightMotor = { 32, 33, 25, 1 };

Adafruit_MPU6050 mpu;

struct DirectionInfo {
  int dy;
  int dx;
  Direction dir;
} directions[] = {
  { -1, 0, FRONT },
  { 0, 1, RIGHT },
  { -1, 0, BACK },
  { 0, -1, LEFT }
};

void setMotorState(MotorPin motor, MotorState state) {
  if (state == FORWARD) {
    digitalWrite(motor.pin1, LOW);
    digitalWrite(motor.pin2, HIGH);
  } else if (state == REVERSE) {
    digitalWrite(motor.pin1, HIGH);
    digitalWrite(motor.pin2, LOW);
  } else if (state == STOP) {
    digitalWrite(motor.pin1, LOW);
    digitalWrite(motor.pin2, LOW);
  }
}

void rotateByAngle(float targetAngle) {
  float currentZAngle = 0.0;
  unsigned long lastTime = millis();

  if (targetAngle > 0) {
    setMotorState(leftMotor, FORWARD);
    setMotorState(rightMotor, STOP);
  } else {
    setMotorState(leftMotor, STOP);
    setMotorState(rightMotor, FORWARD);
  }

  Serial.println("Starting rotation");
  while (abs(currentZAngle) < abs(targetAngle)) {
    unsigned long currentTime = millis();
    float dt = (currentTime - lastTime) / 1000.0;
    lastTime = currentTime;

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    float zRate = g.gyro.z;
    currentZAngle += (zRate * 180.0 / M_PI) * dt;
  }

  Serial.println("rotating finished");
  setMotorState(leftMotor, STOP);
  setMotorState(rightMotor, STOP);
}

void moveForward() {
  setMotorState(leftMotor, FORWARD);
  setMotorState(rightMotor, FORWARD);
  delay(LINEAR_MS);
}

void turnLeft() {
  rotateByAngle(-70.0);
}

void turnRight() {
  rotateByAngle(70.0);
}

void turn180() {
  rotateByAngle(140.0);
}

void handleMove(Direction dir) {
  ledcWrite(leftMotor.enablePin, MOTOR_DUTY_CYCLE);
  ledcWrite(rightMotor.enablePin, MOTOR_DUTY_CYCLE);
  if (dir == LEFT) {
    turnLeft();
  } else if (dir == RIGHT) {
    turnRight();
  } else if (dir == BACK) {
    turn180();
  }
  moveForward();
  ledcWrite(leftMotor.enablePin, 0);
  ledcWrite(rightMotor.enablePin, 0);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Running");

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) delay(10);
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);

  pinMode(leftMotor.pin1, OUTPUT);
  pinMode(rightMotor.pin1, OUTPUT);

  pinMode(leftMotor.pin2, OUTPUT);
  pinMode(rightMotor.pin2, OUTPUT);

  pinMode(leftMotor.enablePin, OUTPUT);
  pinMode(rightMotor.enablePin, OUTPUT);

  ledcAttachChannel(leftMotor.enablePin, MOTOR_FREQUENCY, MOTOR_RESOLUTION, leftMotor.pwmChannel);
  ledcAttachChannel(rightMotor.enablePin, MOTOR_FREQUENCY, MOTOR_RESOLUTION, leftMotor.pwmChannel);

  delay(5000);

  Serial.println("moving left");
  handleMove(LEFT);
  delay(3000);

  Serial.println("moving right");
  handleMove(RIGHT);
  delay(3000);

  Serial.println("moving back");
  handleMove(BACK);
}

void loop() {
}

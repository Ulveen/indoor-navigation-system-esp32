#include <string>
#include <queue>
#include <vector>
#include <climits>
#define SOUND_SPEED 0.0343
#define GRID_HEIGHT 30
#define GRID_WIDTH 30
#define LINEAR_MS 300
#define TURN_MS 450
#define MOTOR_DUTY_CYCLE 255
#define MOTOR_FREQUENCY 30000
#define MOTOR_RESOLUTION 8

using namespace std;

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
};

struct MotorPin {
  int pin1;
  int pin2;
  int enablePin;
  int pwmChannel;
} leftMotor = { 27, 36, 14, 0 }, rightMotor = { 32, 33, 25, 1 };

struct UltrasonicPin {
  const int trigPin;
  const int echoPin;
} rightUltrasonic = { 16, 4 }, frontUltrasonic = { 17, 5 }, leftUltrasonic = { 18, 19 };

struct Coordinate {
  int y;
  int x;
} currCoord = { 0, 0 }, endCoord = { GRID_HEIGHT - 1, GRID_WIDTH - 1 };

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

int weights[GRID_HEIGHT][GRID_WIDTH] = { 0 };

void printWeights() {
  for (int i = 0; i < GRID_HEIGHT; i++) {
    for (int j = 0; j < GRID_WIDTH; j++) {
      if (weights[i][j] == INT_MAX) {
        Serial.printf("%3s ", "##");
      } else {
        Serial.printf("%3d ", weights[i][j]);
      }
    }
    Serial.println();
  }
}

float scanDistance(UltrasonicPin sensor) {
  digitalWrite(sensor.trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(sensor.trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(sensor.trigPin, LOW);

  long duration = pulseIn(sensor.echoPin, HIGH, 25000);
  float distance = duration * SOUND_SPEED / 2.0;

  if (distance < 2.0 || distance > 400.0) {
    return -1;
  }
  return distance;
}

bool isOutOfBounds(int y, int x) {
  return y < 0 || x < 0 || y > GRID_HEIGHT - 1 || x > GRID_WIDTH - 1;
}

void floodfill() {
  bool isVisited[GRID_HEIGHT][GRID_WIDTH] = { false };

  queue<Coordinate> q;
  q.push(endCoord);
  weights[endCoord.y][endCoord.x] = 0;
  isVisited[endCoord.y][endCoord.x] = true;

  while (!q.empty()) {
    Coordinate coord = q.front();
    q.pop();

    for (DirectionInfo dirInfo : directions) {
      int newY = coord.y + dirInfo.dy;
      int newX = coord.x + dirInfo.dy;

      if (isOutOfBounds(newY, newX) || isVisited[newY][newX] || weights[newY][newX] == INT_MAX) {
        continue;
      }

      isVisited[newY][newX] = true;
      weights[newY][newX] = weights[coord.y][coord.x] + 1;
      q.push({ newY, newX });
    }
  }
}

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

void moveForward() {
  setMotorState(leftMotor, FORWARD);
  setMotorState(rightMotor, FORWARD);
  delay(LINEAR_MS);
}

void turnLeft() {
  setMotorState(leftMotor, STOP);
  setMotorState(rightMotor, FORWARD);
  delay(TURN_MS);
}

void turnRight() {
  setMotorState(leftMotor, FORWARD);
  setMotorState(rightMotor, STOP);
  delay(TURN_MS);
}

void turn180() {
  setMotorState(leftMotor, FORWARD);
  setMotorState(rightMotor, STOP);
  delay(TURN_MS * 2);
}

void handleMove(Direction dir) {
  ledcWrite(leftMotor.enablePin, MOTOR_DUTY_CYCLE);
  ledcWrite(rightMotor.enablePin, MOTOR_DUTY_CYCLE);
  if (dir == FRONT) {
    moveForward();
  } else if (dir == LEFT) {
    turnLeft();
  } else if (dir == RIGHT) {
    turnRight();
  } else if (dir == BACK) {
    turn180();
  }
  ledcWrite(leftMotor.enablePin, 0);
  ledcWrite(rightMotor.enablePin, 0);
}

bool isObstructed(Direction dir) {
  float distance;
  if (dir == FRONT) {
    distance = scanDistance(frontUltrasonic);
  } else if (dir == LEFT) {
    distance = scanDistance(leftUltrasonic);
  } else if (dir == RIGHT) {
    distance = scanDistance(rightUltrasonic);
  } else if (dir == BACK) {
    distance = -1;
  }
  return distance > 0 && distance < 10;
}

void pathfind() {
  bool moved = false;

  for (DirectionInfo dirInfo : directions) {
    int newY = currCoord.y + dirInfo.dy;
    int newX = currCoord.x + dirInfo.dx;

    if (isOutOfBounds(newY, newX) || weights[newY][newX] > weights[currCoord.y][currCoord.x] || isObstructed(dirInfo.dir)) {
      continue;
    }

    handleMove(dirInfo.dir);
    currCoord.y = newY;
    currCoord.x = newX;
    moved = true;

    break;
  }

  if (!moved) {
    floodfill();
    printWeights();
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(rightUltrasonic.trigPin, OUTPUT);
  pinMode(frontUltrasonic.trigPin, OUTPUT);
  pinMode(leftUltrasonic.trigPin, OUTPUT);

  pinMode(rightUltrasonic.echoPin, INPUT);
  pinMode(frontUltrasonic.echoPin, INPUT);
  pinMode(leftUltrasonic.echoPin, INPUT);

  pinMode(leftMotor.pin1, OUTPUT);
  pinMode(rightMotor.pin1, OUTPUT);

  pinMode(leftMotor.pin2, OUTPUT);
  pinMode(rightMotor.pin2, OUTPUT);

  pinMode(leftMotor.enablePin, OUTPUT);
  pinMode(rightMotor.enablePin, OUTPUT);

  ledcAttachChannel(leftMotor.pin1, MOTOR_FREQUENCY, MOTOR_RESOLUTION, leftMotor.pwmChannel);
  ledcAttachChannel(rightMotor.pin1, MOTOR_FREQUENCY, MOTOR_RESOLUTION, leftMotor.pwmChannel);

  floodfill();
  printWeights();

  delay(7000);
}

void loop() {
  if (currCoord.y != endCoord.y || currCoord.x != endCoord.y) {
    pathfind();
  }
}

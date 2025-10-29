#include <queue>
#include <string>
#include <vector>
#include <climits>
#include <WiFi.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <NimBLEDevice.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>
#define SOUND_SPEED 0.0343
#define GRID_HEIGHT 30
#define GRID_WIDTH 30
#define LINEAR_MS 500
#define SLOW_SPEED 230
#define FAST_SPEED 240
#define MOTOR_FREQUENCY 30000
#define MOTOR_RESOLUTION 8
#define RSSI_COUNT 7
#define DEGREE 79

using namespace std;

enum MotorState {
  FORWARD,
  REVERSE,
  STOP
};

enum CarState {
  STARTED,
  RUNNING,
  STOPPED,
  WAITING
} car = WAITING;

enum Direction {
  FRONT,
  RIGHT,
  BACK,
  LEFT
} currDirection = BACK;

enum SamplingState {
  PAUSED,
  IDLE,
  SAMPLING
} samplingState = PAUSED;

struct MotorPin {
  int pin1;
  int pin2;
  int enablePin;
  int pwmChannel;
} leftMotor = { 27, 26, 14, 0 }, rightMotor = { 32, 33, 25, 1 };

struct UltrasonicPin {
  const int trigPin;
  const int echoPin;
} rightUltrasonic = { 16, 4 }, frontUltrasonic = { 17, 5 }, leftUltrasonic = { 18, 19 };

struct Coordinate {
  int y;
  int x;
} currCoord = { 0, 0 }, endCoord = { 29, 29 };

struct DirectionInfo {
  int dy;
  int dx;
  Direction dir;
} directions[] = {
  { -1, 0, FRONT },
  { 0, 1, RIGHT },
  { 1, 0, BACK },
  { 0, -1, LEFT }
};

int weights[GRID_HEIGHT][GRID_WIDTH] = { 0 };
float carAngle = 0;

int numSteps = 0;

Adafruit_MPU6050 mpu;
NimBLEScan* pBLEScan;
WiFiClient espClient;
PubSubClient client(espClient);

const int mqttPort = 1883;
const char *ssid = "Xiaomi 12T", *password = "hehehehe";
const char *mqttHost = "148.230.101.206", *mqttUser = "dk", *mqttPass = "dkdkdk";

StaticJsonDocument<256> doc;
JsonArray rssi1, rssi2, rssi3;
const string rssiBaseTopic = "things/rssi";
const string startTopic = "navigation/start/";
const string endTopic = "navigation/end/";

void resetDoc() {
  rssi1 = doc.createNestedArray("r1");
  rssi2 = doc.createNestedArray("r2");
  rssi3 = doc.createNestedArray("r3");
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32Client", mqttUser, mqttPass)) {
      Serial.println("MQTT Connected");

      client.subscribe(startTopic.c_str());
    } else {
      Serial.print(client.state());
    }
  }
}

bool publish(const char* topic, const char* payload) {
  if (!client.connected()) {
    reconnect();
  }
  if (client.publish(topic, payload)) {
    Serial.println("Publish success");
    return true;
  } else {
    Serial.println("Publish failed");
    Serial.println(client.state());
    return false;
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

void sampleDistance() {
  doc["u1"] = scanDistance(rightUltrasonic);
  doc["u2"] = scanDistance(frontUltrasonic);
  doc["u3"] = scanDistance(leftUltrasonic);
}

bool publishSensorData(string endpoint) {
  string topic = rssiBaseTopic + endpoint;

  char payload[256];
  serializeJson(doc, payload);

  return publish(topic.c_str(), payload);
}

class ScanCallback : public NimBLEScanCallbacks {
  void onResult(const NimBLEAdvertisedDevice* advertisedDevice) {

    if (samplingState == IDLE) {
      resetDoc();
      samplingState = SAMPLING;
    } else if (samplingState != SAMPLING) {
      return;
    }

    string address = advertisedDevice->getAddress().toString();

    // Serial.println(address.c_str());

    if (rssi1.size() == RSSI_COUNT && rssi2.size() == RSSI_COUNT && rssi3.size() == RSSI_COUNT) {
      if (car == RUNNING) {
        sampleDistance();
        publishSensorData("/path");
        samplingState = IDLE;
      }
      return;
    }

    int rssi = advertisedDevice->getRSSI();

    // Serial.println(rssi);

    if (address == "68:25:dd:44:e6:c2" && rssi1.size() < RSSI_COUNT) {
      Serial.println(1);
      // Serial.println("add 1");
      rssi1.add(rssi);
    } else if (address == "b0:a7:32:2a:69:56" && rssi2.size() < RSSI_COUNT) {
      Serial.println(2);
      rssi2.add(rssi);
      // Serial.println("add 2");
    } else if (address == "b0:a7:32:14:26:6a" && rssi3.size() < RSSI_COUNT) {
      Serial.println(3);
      rssi3.add(rssi);
      // Serial.println("add 3");
    }
  }
} scanCallbacks;

void callback(char* topic, uint8_t* payload, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.println(topic);

  char buffer[length + 1];
  memcpy(buffer, payload, length);
  buffer[length] = '\0';

  StaticJsonDocument<128> jsonDoc;
  DeserializationError error = deserializeJson(jsonDoc, buffer);
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
    return;
  }

  // if (topic == topicMotor) {
  //   dir = jsonDoc["dir"].as<String>();
  //   en = jsonDoc["en"].as<String>();
  // }

  if (topic == startTopic) {
    int x = jsonDoc["x"].as<int>();
    int y = jsonDoc["y"].as<int>();

    Serial.printf("y: %d x: %d\n", y, x);

    endCoord.y = y;
    endCoord.x = x;

    car = STARTED;
  }
}

Direction getRotation(Direction targetDirection) {
  int diff = (targetDirection - currDirection + 4) % 4;
  return static_cast<Direction>(diff);
}

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
      int newX = coord.x + dirInfo.dx;

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

void rotateByAngle(float targetAngle) {
  ledcWrite(leftMotor.enablePin, FAST_SPEED);
  ledcWrite(rightMotor.enablePin, FAST_SPEED);

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

  carAngle = 0;

  Serial.println("rotating finished");
  setMotorState(leftMotor, STOP);
  setMotorState(rightMotor, STOP);
}

void moveForward() {
  setMotorState(leftMotor, FORWARD);
  setMotorState(rightMotor, FORWARD);

  unsigned long currTime, startTime, lastTime;
  currTime = startTime = lastTime = millis();

  while (currTime - startTime < LINEAR_MS) {
    float correctedSpeed = min(abs(carAngle), 3.0f) / 2.0 * (FAST_SPEED - SLOW_SPEED) + SLOW_SPEED;

    if (carAngle > 0) {
      ledcWrite(leftMotor.enablePin, correctedSpeed);
      ledcWrite(rightMotor.enablePin, SLOW_SPEED);
    } else {
      ledcWrite(leftMotor.enablePin, SLOW_SPEED);
      ledcWrite(rightMotor.enablePin, correctedSpeed);
    }

    currTime = millis();
    float dt = (currTime - lastTime) / 1000.0;
    lastTime = currTime;

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    float zRate = g.gyro.z;
    carAngle += (zRate * 180.0 / M_PI) * dt;
  }

  setMotorState(leftMotor, STOP);
  setMotorState(rightMotor, STOP);
}

void handleMove(Direction dir) {
  if (dir == LEFT) {
    rotateByAngle(-DEGREE);
  } else if (dir == RIGHT) {
    rotateByAngle(DEGREE);
  } else if (dir == BACK) {
    rotateByAngle(DEGREE * 2);
  }
  moveForward();
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

int stoppedCount = 0;

void pathfind() {
  bool moved = false;

  Serial.printf("y: %d x: %d\n", currCoord.y, currCoord.x);

  for (DirectionInfo dirInfo : directions) {
    int newY = currCoord.y + dirInfo.dy;
    int newX = currCoord.x + dirInfo.dx;
    Direction newDirection = getRotation(dirInfo.dir);

    Serial.printf("ny: %d nx: %d\n", newY, newX);
    if (isOutOfBounds(newY, newX) || weights[newY][newX] > weights[currCoord.y][currCoord.x]) {
      continue;
    }

    if (isObstructed(newDirection)) {
      weights[newY][newX] = INT_MAX;
      continue;
    }

    handleMove(newDirection);
    moved = true;
    stoppedCount = 0;

    currCoord.y = newY;
    currCoord.x = newX;
    currDirection = dirInfo.dir;

    break;
  }

  if (!moved) {
    floodfill();
    printWeights();
    stoppedCount++;
  }

  if (stoppedCount > 1) {
    car = STOPPED;
    stoppedCount = 0;
  }
}

void setupNetwork() {
  NimBLEDevice::init("");
  pBLEScan = NimBLEDevice::getScan();

  Serial.println("HALOOOOOOOOOOOOOOOOOOOOOOOOOOOO");

  const char* macAddresses[] = {
    "68:25:dd:44:e6:c2",
    "b0:a7:32:2a:69:56",
    "b0:a7:32:14:26:6a",
  };

  for (int i = 0; i < 3; i++) {
    NimBLEAddress pAddress(macAddresses[i], BLE_ADDR_PUBLIC);
    NimBLEDevice::whiteListAdd(pAddress);
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.println("\nWiFi Connected");
  client.setServer(mqttHost, mqttPort);
  client.setCallback(callback);

  pBLEScan->setScanCallbacks(&scanCallbacks);
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(150);
  pBLEScan->setWindow(99);

  pBLEScan->setDuplicateFilter(false);
  pBLEScan->setFilterPolicy(BLE_HCI_SCAN_FILT_USE_WL);
  pBLEScan->start(0, false);
}

void setupPins() {
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) delay(10);
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);

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

  ledcAttachChannel(leftMotor.enablePin, MOTOR_FREQUENCY, MOTOR_RESOLUTION, leftMotor.pwmChannel);
  ledcAttachChannel(rightMotor.enablePin, MOTOR_FREQUENCY, MOTOR_RESOLUTION, rightMotor.pwmChannel);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Testing 123");

  setupPins();
  setupNetwork();

  Serial.printf("Slow: %d\n", SLOW_SPEED);
  Serial.printf("Fast: %d\n", FAST_SPEED);
  Serial.printf("Linear ms %d\n", LINEAR_MS);

  resetDoc();
  samplingState = SAMPLING;

  while (rssi1.size() < RSSI_COUNT || rssi2.size() < RSSI_COUNT || rssi3.size() < RSSI_COUNT) {
    delay(1000);
  }
  sampleDistance();

  while (!publishSensorData("/start")) {}
  samplingState = PAUSED;
}

void startCar() {
  floodfill();
  printWeights();
  car = RUNNING;
  samplingState = IDLE;
}

void stopCar() {
  publish(endTopic.c_str(), "");
  car = WAITING;
  samplingState = PAUSED;
}

void loop() {
  client.loop();
  if (car == RUNNING) {
    if (currCoord.y != endCoord.y || currCoord.x != endCoord.x) {
      pathfind();
    } else {
      car = STOPPED;
    }
  } else if (car == STARTED) {
    startCar();
  } else if (car == STOPPED) {
    stopCar();
  } else if (car == WAITING) {
    delay(1000);
  }
}

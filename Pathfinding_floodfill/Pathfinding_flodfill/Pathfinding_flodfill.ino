#include <string>
#include <queue>
#include <vector>
#include <climits>
#define SOUND_SPEED 0.0343
#define SIZEI 30
#define SIZEJ 30
#define  MILISECOND_FRONT 300
#define MILISECOND_TURN 450

using namespace std;
const int motor1Pin1 = 27, motor1Pin2 = 26, enable1Pin = 14;
const int motor2Pin1 = 32, motor2Pin2 = 33, enable2Pin = 25;

const int rightTrigPin = 16, rightEchoPin = 4, frontTrigPin = 17, frontEchoPin = 5, leftTrigPin = 18, leftEchoPin = 19;

const int freq = 30000;
const int pwmChannel1 = 0,pwmChannel2 = 1;
const int resolution = 8;
int dutyCycle = 255;

struct entity {
    int i;
    int j;
};
entity e{0,0};

int state = 2;
// 0 -> atas
// 1 -> kanan
// 2 -> bawah
// 3 -> kiri
vector<vector<int>> dir = {{0,1,1},{1,0,2},{0,-1,3},{-1,0,0}};
// kanan, bawah, kiri, atas

int weights[SIZEI][SIZEJ] = {0};

void printWeights(){
  for(int i=0;i<SIZEI;i++){
    for(int j=0;j<SIZEI;j++){
      if(weights[i][j] == INT_MAX){
        Serial.printf("%3s ","##");

      }else{
        Serial.printf("%3d ",weights[i][j]);

      }
    }
    Serial.println("");
  }
}




float scanDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 25000);

  float distance = duration * SOUND_SPEED / 2.0;
  if (distance < 2.0 || distance > 400.0) return -1;
  return distance;
}


void floodFill(int i, int j, vector<vector<bool>> &vis) {
    queue<pair<int,int>> q;
    q.push({i, j});
    weights[i][j] = 0;

    while (!q.empty()) {
        auto [i, j] = q.front();
        // Serial.printf("I FLOODFILL %d\n", i);
        // Serial.printf("J FLOODFILL %d\n", j);
        q.pop();
        vis[i][j] = true;

        for (auto &d : dir) {
            int ni = i + d[0];
            int nj = j + d[1];

            // check boundary and wall
            if (ni < 0 || nj < 0 || ni >= SIZEI || nj >= SIZEJ) continue;
            if (!vis[ni][nj]) {
                if (weights[ni][nj] != INT_MAX) {
                    vis[ni][nj] = true;
                    weights[ni][nj] = weights[i][j] + 1;
                    q.push({ni, nj});
                }
            }
        }
    }
}


void moveLinear(){
  ledcWrite(enable1Pin, dutyCycle);   
  ledcWrite(enable2Pin, dutyCycle);

  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, HIGH); 
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, HIGH); 
  
  delay(MILISECOND_FRONT);
  ledcWrite(enable1Pin, 0);   
  ledcWrite(enable2Pin, 0);
}

void turn90deg(bool right){
  ledcWrite(enable1Pin, dutyCycle);   
  ledcWrite(enable2Pin, dutyCycle); 
  if(right){
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, HIGH);
    digitalWrite(motor2Pin1, LOW);
    digitalWrite(motor2Pin2, LOW);
  }else{
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, LOW);
    digitalWrite(motor2Pin1, LOW);
    digitalWrite(motor2Pin2, HIGH);
  }
  delay(MILISECOND_TURN);
  ledcWrite(enable1Pin, 0);   
  ledcWrite(enable2Pin, 0); 
}


int getTurnDir(int dir){
  int diff = state-dir;
  Serial.printf("Diff %d\n", diff);
  if(diff == 0){
    return 0;
  }
  if(abs(diff) % 2 == 0){
    return 180;
  }
  if(diff > 0) return -90;
  return 90;

}

bool checkUltrasonicDistance(int dir){
  int checkDir = getTurnDir(dir);
  float distanceUltrasonic; 
  if(checkDir == 90){
    distanceUltrasonic = scanDistance(rightTrigPin, rightEchoPin);
  }else if (checkDir == -90) {
    distanceUltrasonic = scanDistance(leftTrigPin, leftEchoPin);
  }else if(checkDir == 0){
    distanceUltrasonic = scanDistance(frontTrigPin, frontEchoPin);
  }else{
    Serial.println("Valid 180");
    return true;
  }

  Serial.printf("Distance ultrasonic %f\n", distanceUltrasonic);

  return !(distanceUltrasonic < 10 && distanceUltrasonic > 0); 
}

bool validateCoordinate(int i, int j, int dir){
  if (i < 0 || j < 0 || i >= SIZEI || j >= SIZEJ) return false;
  if(!checkUltrasonicDistance(dir)){
    Serial.println("Ultrasonic validate");
    weights[i][j] = INT_MAX;
    return false;
  } 

  return true;
  
}

void handleMove(int deg){
  if(deg == 90){
    turn90deg(true);
  }else if(deg == -90){
    turn90deg(false);
  }else if(deg == 180){
    turn90deg(true);
    turn90deg(true);
  }else{
    moveLinear();
  }

}

bool debugValidate(bool moved, int newI, int newJ, int d){
  // if (!moved && validateCoordinate(newI,newJ, d) && weights[e.i][e.j] > weights[newI][newJ]) {
  //   return true;
  // }
  if(moved){
    Serial.println("Move Invalid");
    return false;
  }
  if(!validateCoordinate(newI,newJ, d)){
    Serial.println("Out of bounds or sensor");
    return false;
  }
  if(weights[e.i][e.j] <= weights[newI][newJ]){
    Serial.println("Weights invalid");
    return false;
  }
  return true;
  
}

void pathFind(){
  bool moved = false;

  while (e.i != SIZEI-1 || e.j != SIZEJ-1) {
    Serial.println(e.i);
    Serial.println(e.j);
    Serial.println(state);
    Serial.println("================================");
    // Serial.printf("EI %d\n", e.i);
    // Serial.printf("EJ %d\n", e.j);

      moved = false;

      for (auto &d : dir) {
          int newI = e.i + d[0], newJ = e.j + d[1];
          Serial.printf("newI %d\n", newI);
          Serial.printf("newJ %d\n", newJ);
          // if (!moved && validateCoordinate(newI,newJ, d[2]) && weights[e.i][e.j] > weights[newI][newJ]) {
          if (debugValidate(moved, newI, newJ, d[2])) {
              Serial.println("validate valid");
              e.i += d[0];
              e.j += d[1];
              moved=true;
              int deg = getTurnDir(d[2]);
              handleMove(deg);

              state = d[2];
          }
      }

      if (!moved) {
        Serial.println("Floodfill");
          vector<vector<bool>> vis(SIZEI,vector<bool>(SIZEJ,false));
          floodFill(SIZEI-1, SIZEJ-1, vis);
          printWeights();

      }


  }
}

void setup() {
  Serial.begin(115200);

  pinMode(rightTrigPin, OUTPUT);
  pinMode(rightEchoPin, INPUT);
  pinMode(frontTrigPin, OUTPUT);
  pinMode(frontEchoPin, INPUT);
  pinMode(leftTrigPin, OUTPUT);
  pinMode(leftEchoPin, INPUT);
  
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(enable1Pin, OUTPUT);

  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);
  pinMode(enable2Pin, OUTPUT);

  ledcAttachChannel(enable1Pin, freq, resolution, pwmChannel1);
  ledcAttachChannel(enable2Pin, freq, resolution, pwmChannel2);

  vector<vector<bool>> vis(SIZEI,vector<bool>(SIZEJ,false));
  floodFill(SIZEI-1,SIZEJ-1, vis);
  printWeights();

  delay(7000);

}



void loop() {
  
  pathFind();




}
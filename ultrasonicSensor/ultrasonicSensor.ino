#define SOUND_SPEED 0.0343

const int trigPin1 = 12;
const int echoPin1 = 14;

const int trigPin2 = 27;
const int echoPin2 = 26;

const int trigPin3 = 25;
const int echoPin3 = 33;

float distance1 = 0;
float distance2 = 0;
float distance3 = 0;

float scan(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 25000);
  
  float distance = duration * 0.0343 / 2.0;
  return distance;
}

void setup() {
  Serial.begin(115200);
  
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  pinMode(trigPin3, OUTPUT);
  pinMode(echoPin3, INPUT);
}

void loop() {
  distance1 = scan(trigPin1, echoPin1);
  distance2 = scan(trigPin2, echoPin2);
  distance3 = scan(trigPin3, echoPin3);

  Serial.print("Dist 1: ");
  Serial.print(distance1);
  Serial.print(" cm  |  Dist 2: ");
  Serial.print(distance2);
  Serial.print(" cm  |  Dist 3: ");
  Serial.print(distance3);
  Serial.println(" cm");

  delay(500);
}
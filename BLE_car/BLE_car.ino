#include <NimBLEDevice.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <string>
#include <ArduinoJson.h>

using namespace std;

// motor pin settings
const int motor1Pin1 = 27, motor1Pin2 = 26, enable1Pin = 14;
const int motor2Pin1 = 32, motor2Pin2 = 33, enable2Pin = 25;

// Setting PWM properties
const int freq = 30000;
const int pwmChannel1 = 0,pwmChannel2 = 1;
const int resolution = 8;
int dutyCycle = 255;

String dir;
String en;

const int trigPin1 = 16, echoPin1 = 4, trigPin2 = 17, echoPin2 = 5, trigPin3 = 18, echoPin3 = 19;
const int mqttPort = 1883;
const char *ssid = "Br", *password = "dk-dutisa", *mqttHost = "148.230.101.206", *mqttUser = "dk", *mqttPass = "dkdkdk";
const string topicRssi = "things/rssi", topicMotor = "things/motor/";

string rssi1_data = "", rssi2_data = "", rssi3_data = "";

int scanTimeMs = 200;
NimBLEScan* pBLEScan;



class ScanCallback : public NimBLEScanCallbacks {
  void onResult(const NimBLEAdvertisedDevice* advertisedDevice) {
    string address = advertisedDevice->getAddress().toString();
    int rssi = advertisedDevice->getRSSI();
    if (address == "68:25:dd:44:e6:c2") {
      if (rssi1_data.length() > 0) rssi1_data += ",";
      rssi1_data += to_string(rssi);
    } else if (address == "b0:a7:32:2a:69:56") {
      if (rssi2_data.length() > 0) rssi2_data += ",";
      rssi2_data += to_string(rssi);
    } else if (address == "b0:a7:32:14:26:6a") {
      if (rssi3_data.length() > 0) rssi3_data += ",";
      rssi3_data += to_string(rssi);
    }
  }
} scanCallbacks;

void motorLogic() {
  printf("En : %s Dir %s\n", en,dir);
  ledcWrite(enable1Pin, dutyCycle);   
  ledcWrite(enable2Pin, dutyCycle);   
  // printf("EN : %s DIR %s\n", en, dir);
  // digitalWrite(motor1Pin1, HIGH);
  // digitalWrite(motor1Pin2, LOW);
  // digitalWrite(motor2Pin1, HIGH);
  // digitalWrite(motor2Pin2, LOW);

  if (en == "true") {
    
    if (dir == "w") {
      digitalWrite(motor1Pin1, LOW);
      digitalWrite(motor1Pin2, HIGH); 
      digitalWrite(motor2Pin1, LOW);
      digitalWrite(motor2Pin2, HIGH); 

    } else if (dir == "s") {
      digitalWrite(motor1Pin1, HIGH);
      digitalWrite(motor1Pin2, LOW); 
      digitalWrite(motor2Pin1, HIGH);
      digitalWrite(motor2Pin2, LOW); 

    } else if (dir == "d") {
      digitalWrite(motor1Pin1, LOW);
      digitalWrite(motor1Pin2, HIGH);
      digitalWrite(motor2Pin1, LOW);
      digitalWrite(motor2Pin2, LOW);

    } else if (dir == "a") {
      digitalWrite(motor1Pin1, LOW);
      digitalWrite(motor1Pin2, LOW);
      digitalWrite(motor2Pin1, LOW);
      digitalWrite(motor2Pin2, HIGH);
    }
  } else {
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, LOW);
    digitalWrite(motor2Pin1, LOW);
    digitalWrite(motor2Pin2, LOW);
  }
}

void callback(char* topic, uint8_t* payload, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.println(topic);

  // Copy payload into a temporary buffer (make sure it’s null-terminated)
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

  if (topic == topicMotor) {
    dir = jsonDoc["dir"].as<String>();
    en = jsonDoc["en"].as<String>();
  }
}

void scanRSSI() {
  rssi1_data = "";
  rssi2_data = "";
  rssi3_data = "";

  for (int i = 0; i < 7; i++) {
    pBLEScan->getResults(scanTimeMs, false);
  }
  pBLEScan->clearResults();
}

float scanDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 25000);

  float distance = duration * 0.0343 / 2.0;
  return distance;
  // return 0;
}

string scanAll() {
  scanRSSI();

  string payload = "{";
  payload += "\"r1\":[" + rssi1_data + "],";
  payload += "\"r2\":[" + rssi2_data + "],";
  payload += "\"r3\":[" + rssi3_data + "],";

  float distance1 = scanDistance(trigPin1, echoPin1);
  float distance2 = scanDistance(trigPin2, echoPin2);
  float distance3 = scanDistance(trigPin3, echoPin3);

  // float distance1 = 0;
  // float distance2 = 0;
  // float distance3 = 0;

  payload += "\"u1\":" + to_string(distance1) + ",\"u2\":" + to_string(distance2) + ",\"u3\":" + to_string(distance3) + "}";

  return payload;
}

WiFiClient espClient;
PubSubClient client(espClient);

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32Client", mqttUser, mqttPass)) {
      Serial.println("MQTT Connected");
      client.subscribe("things/motor/");
    } else {
      Serial.print(client.state());
      delay(5000);
    }
  }
}

void publish(const char* topic, const char* payload) {
  if (!client.connected()) {
    reconnect();
  }
  if (client.publish(topic, payload)) {
    Serial.println("Publish success");
  } else {
    Serial.println("Publish failed");
    Serial.println(client.state());
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  pinMode(trigPin3, OUTPUT);
  pinMode(echoPin3, INPUT);

  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(enable1Pin, OUTPUT);

  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);
  pinMode(enable2Pin, OUTPUT);

  ledcAttachChannel(enable1Pin, freq, resolution, pwmChannel1);
  ledcAttachChannel(enable2Pin, freq, resolution, pwmChannel2);


  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi Connected");
  client.setServer(mqttHost, mqttPort);
  client.setCallback(callback);

  NimBLEDevice::init("");
  pBLEScan = NimBLEDevice::getScan();

  const char* macAdresses[] = {
    "68:25:dd:44:e6:c2",
    "b0:a7:32:2a:69:56",
    "b0:a7:32:14:26:6a",
  };

  for (int i = 0; i < 3; i++) {
    NimBLEAddress pAddress(macAdresses[i], BLE_ADDR_PUBLIC);
    NimBLEDevice::whiteListAdd(pAddress);
  }

  pBLEScan->setScanCallbacks(&scanCallbacks);
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setFilterPolicy(BLE_HCI_SCAN_FILT_USE_WL);
  pBLEScan->setWindow(99);

  string payload = scanAll();
  Serial.println(payload.c_str());
  publish((topicRssi + "/start").c_str(), payload.c_str());
}

void loop() {
  
  motorLogic();

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  // string payload = scanAll();
  // Serial.println(payload.c_str());
  // publish((topicRssi + "/path").c_str(), payload.c_str());
}
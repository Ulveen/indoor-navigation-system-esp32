#include <NimBLEDevice.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <string>

using namespace std;

const int trigPin1 = 12, echoPin1 = 14, trigPin2 = 27, echoPin2 = 26, trigPin3 = 25, echoPin3 = 33, mqttPort = 1883;
const char *ssid = "Xiaomi 12T", *password = "hehehehe", *mqttHost = "148.230.101.206", *mqttUser = "dk", *mqttPass = "dkdkdk";
const string topic = "things/rssi";

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

  payload += "\"u1\":" + to_string(distance1) + ",\"u2\":" + to_string(distance2) + ",\"u3\":" + to_string(distance3) + "}";

  return payload;
}

WiFiClient espClient;
PubSubClient client(espClient);

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32Client", mqttUser, mqttPass)) {
      Serial.println("MQTT Connected");
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

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi Connected");
  client.setServer(mqttHost, mqttPort);

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
  publish((topic + "/start").c_str(), payload.c_str());
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  string payload = scanAll();
  Serial.println(payload.c_str());
  publish((topic + "/path").c_str(), payload.c_str());
  delay(500);
}

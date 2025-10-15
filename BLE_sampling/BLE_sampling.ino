#include <NimBLEDevice.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <string>

#define DATA_COUNT 50
#define SAMPLING_COUNT 10

using namespace std;

NimBLEScan* pBLEScan;

int currDistance = 4;
int rssiData[3][DATA_COUNT];
int counts[] = { 0, 0, 0 };
int sentCount[] = { 0, 0, 0 };

int lastScanned = -1;
bool waiting = false;

WiFiClient espClient;
PubSubClient client(espClient);
const int mqttPort = 1883;
const char *ssid = "Xiaomi 12T", *password = "hehehehe", *mqttHost = "148.230.101.206", *mqttUser = "dk", *mqttPass = "dkdkdk";
const string topic = "things/calibrate";

class ScanCallback : public NimBLEScanCallbacks {
  void onResult(const NimBLEAdvertisedDevice* advertisedDevice) {
    string address = advertisedDevice->getAddress().toString();
    int idx = -1;

    if (address == "68:25:dd:44:e6:c2") {
      idx = 0;
    } else if (address == "b0:a7:32:2a:69:56") {
      idx = 1;
    } else if (address == "b0:a7:32:14:26:6a") {
      idx = 2;
    }

    if (idx == -1 || counts[idx] >= DATA_COUNT) {
      return;
    }

    if (waiting) {
      if (idx == lastScanned) {
        return;
      }
      waiting = false;
    }

    int rssi = advertisedDevice->getRSSI();
    rssiData[idx][counts[idx]++] = rssi;
    lastScanned = idx;
  }
} scanCallbacks;

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

bool sendData(int idx) {
  StaticJsonDocument<1024> doc;

  doc["a"] = idx;
  doc["b"] = currDistance;

  JsonArray data = doc.createNestedArray("c");
  for (int i = 0; i < DATA_COUNT; i++) {
    data.add(-1 * rssiData[idx][i]);
  }

  String output;
  serializeJson(doc, output);

  Serial.print("Sending data for device ");
  Serial.print(idx);
  Serial.println(":");
  Serial.println(output);

  return publish(topic.c_str(), output.c_str());
}


void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE Scanner...");

  NimBLEDevice::init("");
  pBLEScan = NimBLEDevice::getScan();

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
    delay(500);
  }
  Serial.println("\nWiFi Connected");
  client.setServer(mqttHost, mqttPort);

  pBLEScan->setScanCallbacks(&scanCallbacks);
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);

  pBLEScan->setDuplicateFilter(false);
  pBLEScan->setFilterPolicy(BLE_HCI_SCAN_FILT_USE_WL);

  pBLEScan->start(0, false);
}


void loop() {
  client.loop();

  for (int i = 0; i < 3; i++) {
    if (counts[i] >= DATA_COUNT && sentCount[i] < SAMPLING_COUNT) {
      if (sendData(i)) {
        sentCount[i]++;
        counts[i] = 0;
      }
    }
  }

  if (sentCount[0] >= SAMPLING_COUNT && sentCount[1] >= SAMPLING_COUNT && sentCount[2] >= SAMPLING_COUNT && currDistance < 5) {
    sentCount[0] = 0;
    sentCount[1] = 0;
    sentCount[2] = 0;

    counts[0] = 0;
    counts[1] = 0;
    counts[2] = 0;

    currDistance++;
    waiting = true;
  }

  delay(500);
}
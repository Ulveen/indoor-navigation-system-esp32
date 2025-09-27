#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <string>

using namespace std;

const char* ssid = "Xiaomi 12T";
const char* password = "hehehehe";
const char* mqttHost = "148.230.101.206";
const uint16_t mqttPort = 1883;
const char* mqttUser = "dk";
const char* mqttPass = "dkdkdk";
const char* mqttTopic = "things/rssi/curr";

WiFiClient espClient;
PubSubClient client(espClient);

BLEScan* pBLEScan;

String rssi1_data = "";
String rssi2_data = "";
String rssi3_data = "";

class ESPAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    String address = advertisedDevice.getAddress().toString();
    float rssi = advertisedDevice.getRSSI();

    if (address == "68:25:dd:44:e6:c2") {
      if (rssi1_data.length() > 0) rssi1_data += ",";
      rssi1_data += String(rssi);
    } else if (address == "b0:a7:32:2a:69:56") {
      if (rssi2_data.length() > 0) rssi2_data += ",";
      rssi2_data += String(rssi);
    } else if (address == "b0:a7:32:14:26:6a") {
      if (rssi3_data.length() > 0) rssi3_data += ",";
      rssi3_data += String(rssi);
    }
  }
};

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

String scan() {
  for (int i = 0; i < 7; i++) {
    pBLEScan->start(1, false);
    pBLEScan->clearResults();
    delay(50);
  }

  String payload = "{";
  payload += "\"rssi1\":[" + rssi1_data + "],";
  payload += "\"rssi2\":[" + rssi2_data + "],";
  payload += "\"rssi3\":[" + rssi3_data + "]";
  payload += "}";

  rssi1_data = "";
  rssi2_data = "";
  rssi3_data = "";

  return payload;
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi Connected");
  client.setServer(mqttHost, mqttPort);

  BLEDevice::init("ESP32_BLE_Scanner");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new ESPAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);

  String payload = scan();
  Serial.println(payload);

  if (!client.connected()) {
    reconnect();
  }

  // Serial.println("Publishing to MQTT...");
  // if (client.publish(mqttTopic, payload.c_str())) {
  //     Serial.println("Publish success");
  // } else {
  //     Serial.println("Publish failed");
  // }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  String payload = scan();
  Serial.println(payload);
  delay(1000);
}

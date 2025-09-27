#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

int scanTime = 1;

BLEScan* pBLEScan;

class ESPAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (!advertisedDevice.haveName()) {
      return;
    }
    String name = advertisedDevice.getName();
    if (name != "ESP32_Beacon") {
      return;
    }
    Serial.print(" | Name: ");
    Serial.print(name.c_str());
    Serial.print(" | RSSI: ");
    Serial.print(advertisedDevice.getRSSI());
    Serial.print(" | Adress: ");
    Serial.print(advertisedDevice.getAddress().toString().c_str());
    Serial.println();
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Scanning for BLE devices...");
  BLEDevice::init("ESP32_BLE_Scanner");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new ESPAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
}

void loop() {
  pBLEScan->start(scanTime, false);
  delay(1000);
}
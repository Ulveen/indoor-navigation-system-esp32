#include <Arduino.h>
#include <NimBLEDevice.h>

int scanTimeMs = 100;
NimBLEScan* pBLEScan;

class ScanCallbacks : public NimBLEScanCallbacks {
  void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override {
    Serial.println(advertisedDevice->getAddress().toString().c_str());
    Serial.println(advertisedDevice->getRSSI());
  }
} scanCallbacks;

void setup() {
  Serial.begin(115200);
  Serial.println("Scanning...");

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
}

void loop() {
  for (int i = 0; i < 10; i++) {
    NimBLEScanResults foundDevices = pBLEScan->getResults(scanTimeMs, false);
  }
  Serial.println("Scan done!");

  pBLEScan->clearResults();
  delay(5000);
}
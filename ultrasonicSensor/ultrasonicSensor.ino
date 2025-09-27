#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

BLEScan* pBLEScan;

// Struktur untuk menyimpan device + RSSI
struct DeviceInfo {
  String address;
  int rssi;
  String name;
};

// Array untuk menyimpan hasil scan
DeviceInfo devices[50];  // maksimal 50 device
int deviceCount = 0;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (deviceCount >= 50) return; // jangan sampai overflow array

    DeviceInfo info;
    info.address = advertisedDevice.getAddress().toString().c_str();
    info.rssi = advertisedDevice.getRSSI();
    info.name = advertisedDevice.haveName() ? advertisedDevice.getName().c_str() : "<Unknown>";

    devices[deviceCount++] = info;
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Scanning for BLE devices...");

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
}

void loop() {
  deviceCount = 0;  // reset hasil scan
  BLEScanResults* foundDevices = pBLEScan->start(5, false);  // scan 5 detik

  // --- Sort berdasarkan RSSI (descending)
  for (int i = 0; i < deviceCount - 1; i++) {
    for (int j = i + 1; j < deviceCount; j++) {
      if (devices[j].rssi > devices[i].rssi) {
        DeviceInfo temp = devices[i];
        devices[i] = devices[j];
        devices[j] = temp;
      }
    }
  }

  // --- Print hasil
  Serial.printf("Devices found: %d\n", deviceCount);
  for (int i = 0; i < deviceCount; i++) {
    Serial.print(i + 1);
    Serial.print(". Address: ");
    Serial.print(devices[i].address);
    Serial.print(" | RSSI: ");
    Serial.print(devices[i].rssi);
    Serial.print(" dBm | Name: ");
    Serial.println(devices[i].name);
  }

  Serial.println("Scan done!\n");

  pBLEScan->clearResults();
  delay(2000);
}

#include <BLEDevice.h>
#include <BLEAdvertising.h>
#include <BLEUtils.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  BLEDevice::init("My_ESP32_Beacon");
  Serial.print("Device MAC Address: ");
  Serial.println(BLEDevice::getAddress().toString().c_str());
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->start();
  Serial.println("BLE advertising started. Look for 'My_ESP32_Beacon' or the MAC address above.");
}

void loop() {}
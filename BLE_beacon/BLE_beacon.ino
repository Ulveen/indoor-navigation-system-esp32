#include <BLEDevice.h>
#include <BLEAdvertising.h>
#include <BLEUtils.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  BLEDevice::init("ESP32_Beacon");
  Serial.print("Device MAC Address: ");
  Serial.println(BLEDevice::getAddress().toString().c_str());
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P3);
  pAdvertising->start();
  Serial.println("BLE advertising started. Look for 'ESP32_Beacon' or the MAC address above.");
}

void loop() {}
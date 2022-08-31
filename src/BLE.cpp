#include "BLE.h"

#include <Arduino.h>
#include <NimBLEDevice.h>

#include "config.h"

BLEServer* setupBLE() {
  BLEDevice::init("MagicWheelchair-Savannah");
  NimBLEDevice::setMTU(517);

  BLEServer* server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  Serial.println("BLE init complete.");

  return server;
}

void attachUserDescriptionToCharacteristic(BLECharacteristic* iCharacteristic, const std::string& iName) {
  auto descriptor = iCharacteristic->createDescriptor("2901", NIMBLE_PROPERTY::READ);
  descriptor->setValue(iName);
}

void setCharacteristicPresentationFormat(BLECharacteristic* iCharacteristic, uint8_t iType) {
  BLE2904* characteristicPresentationFormatDescriptor = new BLE2904();
  characteristicPresentationFormatDescriptor->setFormat(iType);
  characteristicPresentationFormatDescriptor->setNamespace(1); // mandatory? 1 = Bluetooth SIG Assigned Numbers
  characteristicPresentationFormatDescriptor->setUnit(0x2700); // unitless
  iCharacteristic->addDescriptor(characteristicPresentationFormatDescriptor);
}

BLECharacteristic* getCharacteristicByUUID(BLEService* iService, BLEUUID iCharacteristicID) {
  assert(iService);
  auto characteristic = iService->getCharacteristic(iCharacteristicID);
  assert(characteristic);
  return characteristic;
}

void ServerCallbacks::onConnect(BLEServer* server, ble_gap_conn_desc* desc) {
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_CONN_HDL0, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_CONN_HDL1, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);

  // Taken from https://github.com/ClaesClaes/Arduino-ESP32-NimBLE-OTA-iOS-SwiftUI
  // I can't seem to find what the defaults for ESP32 are but they are NOT enough for our OTA over BLE use case with iOS: CoreBluetooth will
  // give up on the connection during esp_ota_begin >.< These values appear to help.
  //
  // Units:
  // Min/Max Intervals: 1.25 millisecond increments.
  // Latency: number of intervals allowed to skip.
  // Timeout: 10 millisecond increments, try for 5x interval time for best results.
  server->updateConnParams(desc->conn_handle, /*min conn interval*/12, /*max conn interval*/12, /*latency*/2, /*timeout*/100);

  deviceConnected = true;
  Serial.println("Central connected. Start sending updates.");
}

void ServerCallbacks::onDisconnect(BLEServer* server) {
  deviceConnected = false;
  Serial.println("Central disconnected; Advertising again...");
  //BLEDevice::startAdvertising();
}

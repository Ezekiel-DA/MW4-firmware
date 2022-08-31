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

void ServerCallbacks::onConnect(BLEServer* server) {
  deviceConnected = true;
  Serial.println("Central connected. Start sending updates.");
}

void ServerCallbacks::onDisconnect(BLEServer* server) {
  deviceConnected = false;
  Serial.println("Central disconnected; Advertising again...");
  //BLEDevice::startAdvertising();
}

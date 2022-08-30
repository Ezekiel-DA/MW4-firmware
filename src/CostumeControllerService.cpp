#include "CostumeControllerService.h"

#include <Arduino.h>
#include <BLE2902.h>
#include <BLE2904.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include "esp_ota_ops.h"

#include <string>

#include "BLE.h"
#include "config.h"

#define BLE_MAX_CHARACTERISTIC_SIZE 512

bool updateInProgress = false;

esp_ota_handle_t otaHandle = 0;

CostumeControlService::CostumeControlService(BLEServer* iServer, uint8_t iVersion) : _fwVersion(iVersion) {
  std::string serviceUUID = MW4_BLE_COSTUME_CONTROL_SERVICE_UUID;
  _service = iServer->createService(serviceUUID, 15);  // service shouldn't need many / any handles?

  BLECharacteristic* fwVersion = _service->createCharacteristic(MW4_BLE_COSTUME_CONTROL_FW_VERSION_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);
  fwVersion->setValue(&(this->_fwVersion), 1);
  attachUserDescriptionToCharacteristic(fwVersion, "FW version");

  BLECharacteristic* otaUpload = _service->createCharacteristic(MW4_BLE_COSTUME_CONTROL_OTA_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE |BLECharacteristic::PROPERTY_NOTIFY);
  otaUpload->setCallbacks(this);
  otaUpload->addDescriptor(new BLE2902());

  _service->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(MW4_BLE_COSTUME_CONTROL_SERVICE_UUID);

  Serial.println("Costume Controller init complete.");
}

void CostumeControlService::onWrite(BLECharacteristic* characteristic) {
  std::string data = characteristic->getValue();

  if (!updateInProgress) {
    Serial.println("Starting BLE OTA update");
    esp_ota_begin(esp_ota_get_next_update_partition(NULL), OTA_SIZE_UNKNOWN, &otaHandle);
    updateInProgress = true;
  }

  esp_ota_write(otaHandle, data.c_str(), data.length());

  // TODO: if we don't receive another BLE OTA packet for a while, we should probably esp_ota_abort here, to free memory allocated by esp_ota_begin

  if (data.length() != BLE_MAX_CHARACTERISTIC_SIZE) { // message was smaller than max possible size; we must have reached the end of the firmware upload
    // NB: what happens in the unlikely event that our firware size is exactly % 512?
    esp_ota_end(otaHandle);
    Serial.println("BLE OTA download complete.");
    if (ESP_OK == esp_ota_set_boot_partition(esp_ota_get_next_update_partition(NULL))) {
      Serial.println("Restarting...");
      delay(2000);
      esp_restart();
    } else {
      Serial.println("!!BLE OTA failure!!");
    }
  }

  // touch the characteristic, so that the central gets a notification and knows to push the next chunk of firmware
  uint8_t notification[2] = {4, 2};
  characteristic->setValue(notification, 2);
  characteristic->notify();
}

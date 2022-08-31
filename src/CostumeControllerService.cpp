#include "CostumeControllerService.h"

#include <Arduino.h>
#include <NimBLEDevice.h>

#include "esp_ota_ops.h"
#include "esp_task_wdt.h"

#include <string>

#include "BLE.h"
#include "config.h"

#define BLE_MAX_CHARACTERISTIC_SIZE 512

bool updateInProgress = false;

esp_ota_handle_t otaHandle = 0;

CostumeControlService::CostumeControlService(BLEServer* iServer, uint8_t iVersion) : _fwVersion(iVersion) {
  std::string serviceUUID = MW4_BLE_COSTUME_CONTROL_SERVICE_UUID;
  _service = iServer->createService(serviceUUID);

  BLECharacteristic* fwVersion = _service->createCharacteristic(MW4_BLE_COSTUME_CONTROL_FW_VERSION_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ);
  fwVersion->setValue(&(this->_fwVersion), 1);
  attachUserDescriptionToCharacteristic(fwVersion, "FW version");

  BLECharacteristic* otaData = _service->createCharacteristic(MW4_BLE_COSTUME_CONTROL_OTA_DATA_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::WRITE);
  otaData->setCallbacks(this);

  BLECharacteristic* otaControl = _service->createCharacteristic(MW4_BLE_COSTUME_CONTROL_OTA_CONTROL_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
  otaControl->setCallbacks(this);

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(_service->getUUID());

  Serial.println("Costume Controller init complete.");
}

void acknowledgeControlMessage(BLECharacteristic* characteristic) {
  uint8_t res = OTA_CONTROL_ACK;
  characteristic->setValue(&res, 1);
  characteristic->notify();
}

void CostumeControlService::onWrite(BLECharacteristic* characteristic) {
  std::string safeData = characteristic->getValue();
  uint8_t* data = (uint8_t*)safeData.data();  

  BLEUUID id = characteristic->getUUID();
    
  if (id.equals(std::string(MW4_BLE_COSTUME_CONTROL_OTA_CONTROL_CHARACTERISTIC_UUID))) {
    switch (*data) {
      case OTA_CONTROL_START:
        Serial.println("Got OTA START control message");
        assert(!updateInProgress);
        updateInProgress = true;

        // this supposedly helps prevent the BLE central from disconnecting while we perform esp_ota_begin, which is pretty slow?
        esp_task_wdt_init(10, false);
        vTaskDelay(5);

        if (esp_ota_begin(esp_ota_get_next_update_partition(NULL), OTA_SIZE_UNKNOWN, &otaHandle) != ESP_OK) {
          Serial.println("esp_ota_begin failed?!");
          assert(false);
        }

        acknowledgeControlMessage(characteristic);
        break;
      case OTA_CONTROL_END:
      Serial.println("Got OTA END control message");
        assert(updateInProgress);
        updateInProgress = false;

        esp_ota_end(otaHandle);
        otaHandle = 0;

        acknowledgeControlMessage(characteristic);

        Serial.println("BLE OTA download complete.");
        if (ESP_OK == esp_ota_set_boot_partition(esp_ota_get_next_update_partition(NULL))) {
          Serial.println("Restarting...");
          delay(2000);
          esp_restart();
        } else {
          Serial.println("!!BLE OTA failure!!");
          assert(false);
        }
      default:
        //
        break;
    }
  } else if (id.equals(std::string(MW4_BLE_COSTUME_CONTROL_OTA_DATA_CHARACTERISTIC_UUID))) {
    if (!updateInProgress) {
      Serial.println("Got OTA DATA while not performing OTA");
      // TODO: one day, we should probably use this to signal the buffer size from the central?
    } else {
      assert(otaHandle);
      esp_ota_write(otaHandle, data, safeData.length());
    }
  }
}

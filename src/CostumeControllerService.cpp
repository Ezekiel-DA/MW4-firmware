#include "CostumeControllerService.h"

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2904.h>
#include <BLE2902.h>

#include <string>

#include "config.h"
#include "BLE.h"

CostumeControlService::CostumeControlService(BLEServer* iServer) {
  std::string serviceUUID = MWNEXT_BLE_COSTUME_CONTROL_SERVICE_UUID;
  _service = iServer->createService(serviceUUID, 60);

  BLECharacteristic* frontPanelText = _service->createCharacteristic(MWNEXT_BLE_TEXT_DISPLAY_TEXT_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE |BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE);
  frontPanelText->setCallbacks(this);
  std::string textStartVal = "I WANT YOU";
  frontPanelText->setValue(textStartVal);
  attachUserDescriptionToCharacteristic(frontPanelText, "Front text");
  frontPanelText->addDescriptor(new BLE2902());

  BLECharacteristic* frontPanelOffset = _service->createCharacteristic(MWNEXT_BLE_TEXT_DISPLAY_OFFSET_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE |BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE);
  frontPanelOffset->setCallbacks(this);
  uint8_t startOffset = 0;
  frontPanelOffset->setValue(&startOffset, 1);
  attachUserDescriptionToCharacteristic(frontPanelOffset, "Text offset");
  frontPanelOffset->addDescriptor(new BLE2902());

  // BLECharacteristic* frontPanelCustomizeOffset = _service->createCharacteristic(MWNEXT_BLE_TEXT_DISPLAY_CUSTOMIZE_OFFSET_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE |BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE);
  // frontPanelCustomizeOffset->setCallbacks(this);
  // frontPanelCustomizeOffset->setValue(0);
  // attachUserDescriptionToCharacteristic(frontPanelCustomizeOffset, "Customize offset");
  // frontPanelCustomizeOffset->addDescriptor(new BLE2902());

  _service->start();
}

void CostumeControlService::onWrite(BLECharacteristic* characteristic) {
    BLEUUID id = characteristic->getUUID();
    std::string val = characteristic->getValue();
    auto data = characteristic->getData();

    if (id.equals(std::string(MWNEXT_BLE_TEXT_DISPLAY_TEXT_CHARACTERISTIC_UUID))) {
      this->_text = val;
    }
    else if (id.equals(std::string(MWNEXT_BLE_TEXT_DISPLAY_OFFSET_UUID))) {
      this->_offset = *data;
    }
    // else if (id.equals(std::string(MWNEXT_BLE_TEXT_DISPLAY_CUSTOMIZE_OFFSET_UUID))) {
    //   this->_customOffset = (bool) *data;
    // }
}

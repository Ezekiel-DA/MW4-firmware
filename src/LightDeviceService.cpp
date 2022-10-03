#include "LightDeviceService.h"

#include <Arduino.h>
#include <NimBLEDevice.h>

#include <FastLED.h>
#include <string>

#include "config.h"
#include "BLE.h"
#include "utils.h"

static uint8_t _autoId = 1;

static uint8_t colorCycle;

LightDeviceService::LightDeviceService(BLEServer* iServer, const size_t& len, const std::string& iName, uint8_t iId) : numLEDs(len) {
  this->service = iServer->createService(MW4_BLE_LIGHT_DEVICE_SERVICE_UUID);

  auto objectName = service->createCharacteristic((uint16_t)0x2ABE, NIMBLE_PROPERTY::READ);
  objectName->setValue(iName);

  auto idCharacteristic = service->createCharacteristic(MW4_BLE_ID_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ);
  auto id = iId != 0 ? iId : _autoId++;
  idCharacteristic->setValue(&id, 1);
  attachUserDescriptionToCharacteristic(idCharacteristic, "Id");

  auto stateCharacteristic = service->createCharacteristic(MW4_BLE_STATE_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE |NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::INDICATE);
  stateCharacteristic->setCallbacks(this);
  stateCharacteristic->setValue((uint8_t*) &(this->state), 1);
  attachUserDescriptionToCharacteristic(stateCharacteristic, "State");

  auto modeCharacteristic = service->createCharacteristic(MW4_BLE_MODE_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE |NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::INDICATE);
  modeCharacteristic->setCallbacks(this);
  modeCharacteristic->setValue(&(this->mode), 1);
  attachUserDescriptionToCharacteristic(modeCharacteristic, "Mode");

  auto hueCharacteristic = service->createCharacteristic(MW4_BLE_HUE_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE |NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::INDICATE);
  hueCharacteristic->setCallbacks(this);
  hueCharacteristic->setValue(&(this->hue), 1);
  attachUserDescriptionToCharacteristic(hueCharacteristic, "Hue");

  auto saturationCharacteristic = service->createCharacteristic(MW4_BLE_SATURATION_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE |NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::INDICATE);
  saturationCharacteristic->setCallbacks(this);
  saturationCharacteristic->setValue(&(this->saturation), 1);
  attachUserDescriptionToCharacteristic(saturationCharacteristic, "Saturation");

  auto valueCharacteristic = service->createCharacteristic(MW4_BLE_VALUE_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE |NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::INDICATE);
  valueCharacteristic->setCallbacks(this);
  valueCharacteristic->setValue(&(this->value), 1);
  attachUserDescriptionToCharacteristic(valueCharacteristic, "Value");

  prevUpdate = millis();
}

void LightDeviceService::onWrite(BLECharacteristic* characteristic) {
    BLEUUID id = characteristic->getUUID();

    std::string safeData = characteristic->getValue();
    uint8_t* data = (uint8_t*)safeData.data();

    if (id.equals(std::string(MW4_BLE_HUE_CHARACTERISTIC_UUID))) {
      this->hue = *data;
    } else if (id.equals(std::string(MW4_BLE_SATURATION_CHARACTERISTIC_UUID))) {
      this->saturation = *data;
    } else if (id.equals(std::string(MW4_BLE_VALUE_CHARACTERISTIC_UUID))) {
      this->value = *data;
    } else if (id.equals(std::string(MW4_BLE_STATE_CHARACTERISTIC_UUID))) {
      this->state = (*data) != 0;

    } else if (id.equals(std::string(MW4_BLE_MODE_CHARACTERISTIC_UUID))) {
      this->mode = *data;
    }
}

void LightDeviceService::globalAnimationUpdate() {
  auto now = millis();

  static auto lastRainbowUpdate = millis();

  if (now - lastRainbowUpdate > 100) {
    ++colorCycle;
    lastRainbowUpdate = now;
  }
}

void LightDeviceService::update(bool iAltMode) {
  auto now = millis();
  
  // local update for the current light strip only
  auto elapsed = now - prevUpdate;

  if (elapsed < 16) {
    return;
  }

  if ((this->state == false || this->mode == 0) && elapsed < 300) { // no need to update fast if we're not animating anything
    return;
  }
  
  // ready for a frame of display
  prevUpdate = now;

  if (this->state == false) {
    fill_solid(this->leds, this->numLEDs, CRGB::Black);
  } else {
    switch (this->mode) {
      case 0: // steady
        fill_solid(this->leds, this->numLEDs, CHSV(this->hue, this->saturation, this->value));
        break;
      case 1: // pulse
      {
        setAllLEDs(CHSV(this->hue, this->saturation, beatsin8(20, 20, this->value)), this->leds, this->numLEDs);
        break;
      }
      case 2: // rainbow fill
        fill_solid(this->leds, this->numLEDs, CHSV(colorCycle, 255, this->value));
        break;
      case 3: // rainbow wave
        fill_rainbow(this->leds, this->numLEDs, beat8(5), 5);
        break;
    }
  }

  controller->showLeds();
}

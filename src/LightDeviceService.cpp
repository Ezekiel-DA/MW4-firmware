#include "LightDeviceService.h"

#include <Arduino.h>
#include <NimBLEDevice.h>

#include <string>

#include "config.h"
#include "BLE.h"
#include "settingsManager.h"

static uint8_t _autoId = 1;

static uint8_t colorCycle;

BLECharacteristic* LightDeviceService::createCharacteristic(const char** uuids, const char* iDisplayName, bool iAltMode) {
  auto characteristic = service->createCharacteristic(uuids[iAltMode], NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::INDICATE);
  characteristic->setCallbacks(this);
  attachUserDescriptionToCharacteristic(characteristic, !iAltMode ? iDisplayName : std::string(iDisplayName) + "(Alt.)");

  return characteristic;
}

void LightDeviceService::createBLECharacteristics(LightDeviceSettings& iSettings, bool iAltMode) {
  auto state = createCharacteristic(MW4_BLE_STATE_CHARACTERISTIC_UUIDS, "State", iAltMode);
  state->setValue((uint8_t*) &(iSettings.state), 1);

  auto mode = createCharacteristic(MW4_BLE_MODE_CHARACTERISTIC_UUIDS, "Mode", iAltMode);
  mode->setValue(&(iSettings.mode), 1);

  auto hue = createCharacteristic(MW4_BLE_HUE_CHARACTERISTIC_UUIDS, "Hue", iAltMode);
  hue->setValue(&(iSettings.hue), 1);

  auto saturation = createCharacteristic(MW4_BLE_SATURATION_CHARACTERISTIC_UUIDS, "Saturation", iAltMode);
  saturation->setValue(&(iSettings.saturation), 1);

  auto value = createCharacteristic(MW4_BLE_VALUE_CHARACTERISTIC_UUIDS, "Value", iAltMode);
  value->setValue(&(iSettings.value), 1);
}

LightDeviceService::LightDeviceService(BLEServer* iServer, const std::string& iName, LightDeviceSettings* iSettings, LightDeviceSettings* iSettingsAlt, uint8_t iId)
    : settings(iSettings), settingsAlt(iSettingsAlt) {

  id = iId ? iId : _autoId++;
  
  this->service = iServer->createService(MW4_BLE_LIGHT_DEVICE_SERVICE_UUID);

  auto objectName = service->createCharacteristic((uint16_t)0x2ABE, NIMBLE_PROPERTY::READ);
  objectName->setValue(iName);

  auto idCharacteristic = service->createCharacteristic(MW4_BLE_ID_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ);
  idCharacteristic->setValue(&id, 1);
  attachUserDescriptionToCharacteristic(idCharacteristic, "Id");

  createBLECharacteristics(*settings);
  createBLECharacteristics(*settingsAlt, true);

  prevUpdate = millis();
}

void LightDeviceService::onWrite(BLECharacteristic* characteristic) {
  BLEUUID id = characteristic->getUUID();

  std::string safeData = characteristic->getValue();
  uint8_t* data = (uint8_t*)safeData.data();

  portENTER_CRITICAL(&settingsMutex);

  if (id.equals(std::string(MW4_BLE_HUE_CHARACTERISTIC_UUID))) {
    this->settings->hue = *data;
  } else if (id.equals(std::string(MW4_BLE_HUE_ALT_CHARACTERISTIC_UUID))) {
    this->settingsAlt->hue = *data;
  } else if (id.equals(std::string(MW4_BLE_SATURATION_CHARACTERISTIC_UUID))) {
    this->settings->saturation = *data;
  } else if (id.equals(std::string(MW4_BLE_SATURATION_ALT_CHARACTERISTIC_UUID))) {
    this->settingsAlt->saturation = *data;
  } else if (id.equals(std::string(MW4_BLE_VALUE_CHARACTERISTIC_UUID))) {
    this->settings->value = *data;
  } else if (id.equals(std::string(MW4_BLE_VALUE_ALT_CHARACTERISTIC_UUID))) {
    this->settingsAlt->value = *data;
  } else if (id.equals(std::string(MW4_BLE_STATE_CHARACTERISTIC_UUID))) {
    this->settings->state = (*data) != 0;
  } else if (id.equals(std::string(MW4_BLE_STATE_ALT_CHARACTERISTIC_UUID))) {
    this->settingsAlt->state = (*data) != 0;
  } else if (id.equals(std::string(MW4_BLE_MODE_CHARACTERISTIC_UUID))) {
    this->settings->mode = *data;
  } else if (id.equals(std::string(MW4_BLE_MODE_ALT_CHARACTERISTIC_UUID))) {
    this->settingsAlt->mode = *data;
  }

  portEXIT_CRITICAL(&settingsMutex);

  markSettingsModified();
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

  auto settingsToUse = !iAltMode ? this->settings : this->settingsAlt;
  
  // local update for the current light strip only
  auto elapsed = now - prevUpdate;

  if (elapsed < 16) {
    return;
  }

  if ((settingsToUse->state == false || settingsToUse->mode == 0) && elapsed < 300) { // no need to update fast if we're not animating anything
    return;
  }
  
  // ready for a frame of display
  prevUpdate = now;

  for (auto channel: channels) {
    if (settingsToUse->state == false) {
      channel->off();
    } else {
      switch (settingsToUse->mode) {
        case 0: // steady
          channel->fillSolid(CHSV(settingsToUse->hue, settingsToUse->saturation, settingsToUse->saturation == 255 ? BRIGHTNESS_LIMIT_AT_WHITE : settingsToUse->value));
          break;
        case 1: // pulse
        {
          channel->fillSolid(CHSV(settingsToUse->hue, settingsToUse->saturation, beatsin8(PULSE_BPM, 20, settingsToUse->saturation == 255 ? BRIGHTNESS_LIMIT_AT_WHITE : settingsToUse->value)));
          break;
        }
        case 2: // rainbow fill
          channel->fillSolid(CHSV(colorCycle, 255, settingsToUse->value));
          break;
        case 3: // rainbow wave
          channel->fillRainbow();
          break;
      }
    }
  }
}

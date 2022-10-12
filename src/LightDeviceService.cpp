#include "LightDeviceService.h"

#include <Arduino.h>
#include <NimBLEDevice.h>

#include <string>

#include "config.h"
#include "BLE.h"

static uint8_t _autoId = 1;

static uint8_t colorCycle;

LightDeviceService::LightDeviceService(BLEServer* iServer, const size_t& len, const std::string& iName, uint8_t iId) : numLEDs(len) {
  // default settings; TODO: move these
  settings.mode = 1;
  settings.saturation = 255;
  settings.hue = 255;
  settings.value = 255;

  settingsAlt.mode = 1;
  settingsAlt.saturation = 0;
  settingsAlt.hue = 255;
  settingsAlt.value = 255;


  this->service = iServer->createService(MW4_BLE_LIGHT_DEVICE_SERVICE_UUID);

  auto objectName = service->createCharacteristic((uint16_t)0x2ABE, NIMBLE_PROPERTY::READ);
  objectName->setValue(iName);

  auto idCharacteristic = service->createCharacteristic(MW4_BLE_ID_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ);
  auto id = iId != 0 ? iId : _autoId++;
  idCharacteristic->setValue(&id, 1);
  attachUserDescriptionToCharacteristic(idCharacteristic, "Id");

  auto stateCharacteristic = service->createCharacteristic(MW4_BLE_STATE_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE |NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::INDICATE);
  stateCharacteristic->setCallbacks(this);
  stateCharacteristic->setValue((uint8_t*) &(this->settings.state), 1);
  attachUserDescriptionToCharacteristic(stateCharacteristic, "State");

  auto modeCharacteristic = service->createCharacteristic(MW4_BLE_MODE_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE |NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::INDICATE);
  modeCharacteristic->setCallbacks(this);
  modeCharacteristic->setValue(&(this->settings.mode), 1);
  attachUserDescriptionToCharacteristic(modeCharacteristic, "Mode");

  auto hueCharacteristic = service->createCharacteristic(MW4_BLE_HUE_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE |NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::INDICATE);
  hueCharacteristic->setCallbacks(this);
  hueCharacteristic->setValue(&(this->settings.hue), 1);
  attachUserDescriptionToCharacteristic(hueCharacteristic, "Hue");

  auto saturationCharacteristic = service->createCharacteristic(MW4_BLE_SATURATION_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE |NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::INDICATE);
  saturationCharacteristic->setCallbacks(this);
  saturationCharacteristic->setValue(&(this->settings.saturation), 1);
  attachUserDescriptionToCharacteristic(saturationCharacteristic, "Saturation");

  auto valueCharacteristic = service->createCharacteristic(MW4_BLE_VALUE_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE |NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::INDICATE);
  valueCharacteristic->setCallbacks(this);
  valueCharacteristic->setValue(&(this->settings.value), 1);
  attachUserDescriptionToCharacteristic(valueCharacteristic, "Value");

  prevUpdate = millis();
}

void LightDeviceService::onWrite(BLECharacteristic* characteristic) {
    BLEUUID id = characteristic->getUUID();

    std::string safeData = characteristic->getValue();
    uint8_t* data = (uint8_t*)safeData.data();

    if (id.equals(std::string(MW4_BLE_HUE_CHARACTERISTIC_UUID))) {
      this->settings.hue = *data;
    } else if (id.equals(std::string(MW4_BLE_SATURATION_CHARACTERISTIC_UUID))) {
      this->settings.saturation = *data;
    } else if (id.equals(std::string(MW4_BLE_VALUE_CHARACTERISTIC_UUID))) {
      this->settings.value = *data;
    } else if (id.equals(std::string(MW4_BLE_STATE_CHARACTERISTIC_UUID))) {
      this->settings.state = (*data) != 0;

    } else if (id.equals(std::string(MW4_BLE_MODE_CHARACTERISTIC_UUID))) {
      this->settings.mode = *data;
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

  auto settingsToUse = !iAltMode ? this->settings : this->settingsAlt;
  
  // local update for the current light strip only
  auto elapsed = now - prevUpdate;

  if (elapsed < 16) {
    return;
  }

  if ((settingsToUse.state == false || settingsToUse.mode == 0) && elapsed < 300) { // no need to update fast if we're not animating anything
    return;
  }
  
  // ready for a frame of display
  prevUpdate = now;

  if (settingsToUse.state == false) {
    fill_solid(this->leds, this->numLEDs, CRGB::Black);
  } else {
    switch (settingsToUse.mode) {
      case 0: // steady
        if (this->_backScreenHack) {
          fill_solid(this->leds, BACK_SCREEN_NUM_BACKLIGHT_LEDS, CRGB::White);
          fill_solid(&(this->leds[BACK_SCREEN_NUM_BACKLIGHT_LEDS]), this->numLEDs - BACK_SCREEN_NUM_BACKLIGHT_LEDS, CHSV(settingsToUse.hue, settingsToUse.saturation, settingsToUse.saturation == 255 ? BRIGHTNESS_LIMIT_AT_WHITE : settingsToUse.value));
        } else {
          fill_solid(this->leds, this->numLEDs, CHSV(settingsToUse.hue, settingsToUse.saturation, settingsToUse.saturation == 255 ? BRIGHTNESS_LIMIT_AT_WHITE : settingsToUse.value));
        }
        break;
      case 1: // pulse
      {
        if (this->_backScreenHack) {
          fill_solid(this->leds, BACK_SCREEN_NUM_BACKLIGHT_LEDS, CRGB::White);
          fill_solid(&(this->leds[BACK_SCREEN_NUM_BACKLIGHT_LEDS]), this->numLEDs - BACK_SCREEN_NUM_BACKLIGHT_LEDS, CHSV(settingsToUse.hue, settingsToUse.saturation, beatsin8(PULSE_BPM, 20, settingsToUse.value)));
        } else {
          fill_solid(this->leds, this->numLEDs, CHSV(settingsToUse.hue, settingsToUse.saturation, beatsin8(PULSE_BPM, 20, settingsToUse.saturation == 255 ? BRIGHTNESS_LIMIT_AT_WHITE : settingsToUse.value)));
        }
        break;
      }
      case 2: // rainbow fill
        if (this->_backScreenHack) {
          fill_solid(this->leds, BACK_SCREEN_NUM_BACKLIGHT_LEDS, CRGB::White);
          fill_solid(&(this->leds[BACK_SCREEN_NUM_BACKLIGHT_LEDS]), this->numLEDs - BACK_SCREEN_NUM_BACKLIGHT_LEDS, CHSV(colorCycle, 255, settingsToUse.value));
        } else {
          fill_solid(this->leds, this->numLEDs, CHSV(colorCycle, 255, settingsToUse.value));
        }
        break;
      case 3: // rainbow wave
        if (this->_backScreenHack) {
          fill_solid(this->leds, BACK_SCREEN_NUM_BACKLIGHT_LEDS, CRGB::White);
          fill_rainbow(&(this->leds[BACK_SCREEN_NUM_BACKLIGHT_LEDS]), this->numLEDs - BACK_SCREEN_NUM_BACKLIGHT_LEDS, beat8(5), 5);
        } else {
          fill_rainbow(this->leds, this->numLEDs, beat8(5), 5);
        }
        break;
    }
  }

  //controller->showLeds();
}

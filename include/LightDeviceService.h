#pragma once

#include <NimBLEDevice.h>

#define FASTLED_RMT_MAX_CHANNELS 4 // required to work around a bug in FastLED's ESP32 SMT driver?
#include <FastLED.h>

#include "settingsManager.h"

class BLEService;

class LightDeviceService : public BLECharacteristicCallbacks {

public:
  LightDeviceService(BLEServer* iServer, const size_t& iLen, const std::string& iName, LightDeviceSettings* iSettings, LightDeviceSettings* iSettingsAlt);

  void update(bool iAltMode=false);  
  void onWrite(BLECharacteristic* characteristic);

  BLEService* service = nullptr;

  size_t numLEDs;

  LightDeviceSettings* settings;
  LightDeviceSettings* settingsAlt;

  // HACK avert your eyes
  bool _backScreenHack = false;

  template<uint8_t DATA_PIN> friend void addLEDsToLightDeviceService(LightDeviceService* iLightDevice);

  static void globalAnimationUpdate();

  private:
    CRGB* leds;
    CLEDController* controller;

    uint32_t prevUpdate;
};

template<uint8_t DATA_PIN> void addLEDsToLightDeviceService(LightDeviceService* iLightDevice) {
  iLightDevice->leds = (CRGB*) malloc(sizeof(CRGB) * iLightDevice->numLEDs);
  iLightDevice->controller = &FastLED.addLeds<WS2812B, DATA_PIN, GRB>(iLightDevice->leds, iLightDevice->numLEDs);
  iLightDevice->controller->setDither(0);
};

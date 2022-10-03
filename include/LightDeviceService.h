#pragma once

#include <NimBLEDevice.h>
#include <FastLED.h>

class BLEService;

class LightDeviceService : public BLECharacteristicCallbacks {

public:
  LightDeviceService(BLEServer* iServer, const size_t& iLen, const std::string& iName, uint8_t id=0);

  void update(bool iAltMode=false);  
  void onWrite(BLECharacteristic* characteristic);

  BLEService* service = nullptr;

  size_t numLEDs;

  uint8_t mode = 1; // steady, pulse,  rainbow pulse, rainbow wave
  bool state = true; // on / off
  uint8_t hue = 255;
  uint8_t saturation = 0;
  uint8_t value = 255;

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

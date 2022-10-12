#pragma once

#include <NimBLEDevice.h>

#define FASTLED_RMT_MAX_CHANNELS 1 // required to work around a bug in FastLED's ESP32 SMT driver?
#include <FastLED.h>

class BLEService;

struct LightDeviceServiceSettings {
  uint8_t mode = 0; // steady, pulse,  rainbow pulse, rainbow wave
  bool state = true; // on / off
  uint8_t hue = 255;
  uint8_t saturation = 255;
  uint8_t value = 255;
};

class LightDeviceService : public BLECharacteristicCallbacks {

public:
  LightDeviceService(BLEServer* iServer, const size_t& iLen, const std::string& iName, uint8_t id=0);

  void update(bool iAltMode=false);  
  void onWrite(BLECharacteristic* characteristic);

  BLEService* service = nullptr;

  size_t numLEDs;

  LightDeviceServiceSettings settings;
  LightDeviceServiceSettings settingsAlt;

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

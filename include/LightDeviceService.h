#pragma once

#include <NimBLEDevice.h>
#include <FastLED.h>

class BLEService;

class LightDeviceService : public BLECharacteristicCallbacks {

public:
  LightDeviceService(BLEServer* iServer, const size_t& iLen, const std::string& iName, uint8_t id=0);

  void update();  
  void onWrite(BLECharacteristic* characteristic);

  BLEService* service = nullptr;

  size_t numLEDs;

  uint8_t brightness = 10;

  uint8_t mode = 0; // steady, pulse, rainbow? (which would override hue, obviously)
  bool state = true; // on / off
  uint8_t hue = 255;
  uint8_t saturation = 255;
  uint8_t value = 255;

  template<uint8_t DATA_PIN> friend void addLEDsToLightDeviceService(LightDeviceService* iLightDevice);

  private:
    CRGB* leds;
};

template<uint8_t DATA_PIN> void addLEDsToLightDeviceService(LightDeviceService* iLightDevice) {
  iLightDevice->leds = (CRGB*) malloc(sizeof(CRGB) * iLightDevice->numLEDs);
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(iLightDevice->leds, iLightDevice->numLEDs);
};

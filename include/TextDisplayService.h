#pragma once

#include <NimBLEDevice.h>

#define FASTLED_RMT_MAX_CHANNELS 1 // required to work around a bug in FastLED's ESP32 SMT driver?
#include <FastLED.h>

#include "settingsManager.h"

class StripDisplay;

#define STRIPLED_W 64
#define STRIPLED_H 8

#define STARTING_OFFSET -STRIPLED_W + 1 // this is a function of the length of the panel and is independent of the length of the displayed text

/**
 * @brief Service to handle a text display made up of two 8x32 LED panels.
 * WARNING: THIS SERVICE IS A MESS OF GLOBALS AND WILL NOT SUPPORT MULTIPLE INSTANCES AT THIS TIME!
 * 
 */
class TextDisplayService : public BLECharacteristicCallbacks {

public:
  TextDisplayService(BLEServer* iServer, TextDisplaySettings* iSettings, TextDisplaySettings* iSettingsAlt);

  void update(bool iAltMode);
  
  void onWrite(BLECharacteristic* characteristic);

  BLEService* service = nullptr;

  TextDisplaySettings* settings;
  TextDisplaySettings* settingsAlt;

  template<uint8_t DATA_PIN> friend void addLEDsToTextDisplayService(TextDisplayService* iTextDisplayService);

private:
  CRGB leds[STRIPLED_W * STRIPLED_H];
  StripDisplay* strip;
  CLEDController* controller = nullptr;

  void createBLECharacteristics(TextDisplaySettings& settings, bool iAltMode=false);

  BLECharacteristic* createCharacteristic(const char** uuids, const char* iDisplayName, bool iAltMode=false);
};

template<uint8_t DATA_PIN> void addLEDsToTextDisplayService(TextDisplayService* iTextDisplayService) {
  iTextDisplayService->controller = &FastLED.addLeds<WS2812B, DATA_PIN, GRB>(iTextDisplayService->leds, STRIPLED_W * STRIPLED_H);
  iTextDisplayService->controller->setDither(0);
};

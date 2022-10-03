#pragma once

#include <NimBLEDevice.h>
#include <FastLED.h>

class StripDisplay;

#define STRIPLED_W 64
#define STRIPLED_H 8

#define STARTING_OFFSET -STRIPLED_W + 1 // this is a function of the length of the panel and is independent of the length of the displayed text

struct TextDisplayServiceSettings {
  std::string text;
  bool scrolling = true;
  uint8_t scrollSpeed = 50;
  uint8_t pauseTime = 3;
  int16_t offset = 0;
  uint8_t brightness = 255;

  uint8_t fgColor[3] = {255, 255, 255};
  uint8_t bgColor[3] = {0, 0, 0};
};

class TextDisplayService : public BLECharacteristicCallbacks {

public:
  TextDisplayService(BLEServer* iServer, const std::string& iDefaultText);

  void update(bool iAltMode);
  
  void onWrite(BLECharacteristic* characteristic);

  BLEService* service = nullptr;

  TextDisplayServiceSettings settings;
  TextDisplayServiceSettings settingsAlt;

  template<uint8_t DATA_PIN> friend void addLEDsToTextDisplayService(TextDisplayService* iTextDisplayService);

private:
  CRGB leds[STRIPLED_W * STRIPLED_H];
  StripDisplay* strip;
  CLEDController* controller = nullptr;

  void createBLECharacteristics(TextDisplayServiceSettings& settings, bool iAltMode=false);

  BLECharacteristic* createCharacteristic(const char** uuids, const char* iDisplayName, bool iAltMode=false);
};

template<uint8_t DATA_PIN> void addLEDsToTextDisplayService(TextDisplayService* iTextDisplayService) {
  iTextDisplayService->controller = &FastLED.addLeds<WS2812B, DATA_PIN, GRB>(iTextDisplayService->leds, STRIPLED_W * STRIPLED_H);
};
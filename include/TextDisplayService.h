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
  uint8_t brightness = 25;

  uint8_t fgColor[3] = {255, 255, 255};
  uint8_t bgColor[3] = {0, 0, 0};
};

class TextDisplayService : public BLECharacteristicCallbacks {

public:
  TextDisplayService(BLEServer* iServer, const std::string& iDefaultText);

  void update();
  
  void onWrite(BLECharacteristic* characteristic);

  BLEService* service = nullptr;

  TextDisplayServiceSettings settings;
  TextDisplayServiceSettings settingsAlt;

private:
  CRGB leds[STRIPLED_W * STRIPLED_H];
  StripDisplay* strip;

  void createBLECharacteristics(TextDisplayServiceSettings& settings);

  BLECharacteristic* createCharacteristic(const char* iDisplayName, const char* uuid);
};

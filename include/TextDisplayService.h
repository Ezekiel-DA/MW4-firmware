#pragma once

#include <NimBLEDevice.h>

class BLEService;

#define STRIPLED_GPIO 14
#define STRIPLED_W 64
#define STRIPLED_H 8

#define STARTING_OFFSET -STRIPLED_W + 1 // this is a function of the length of the panel and is independent of the length of the displayed text

class TextDisplayService : public BLECharacteristicCallbacks {
private:
  

public:
  TextDisplayService(BLEServer* iServer, const std::string& iDefaultText);

  void update();
  
  void onWrite(BLECharacteristic* characteristic);

  BLEService* _service = nullptr;

  std::string _text;
  bool _scrolling = true;
  uint8_t _scrollSpeed = 50;
  uint8_t _pauseTime = 3;
  int16_t _offset = 0;
  uint8_t _brightness = 10;

  uint8_t _fgColor[3] = {255, 255, 255};
  uint8_t _bgColor[3] = {0, 0, 0};
};

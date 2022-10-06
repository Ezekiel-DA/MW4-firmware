#pragma once

#include "config.h"

#include <NimBLEDevice.h>

class BLEService;

class MusicService : public BLECharacteristicCallbacks {

public:
  MusicService(BLEServer* iServer);

  void onWrite(BLECharacteristic* characteristic);

  void play();

  BLEService* service = nullptr;

  bool state = true; // on / off
  uint8_t volume = DEFAULT_VOLUME;
  uint8_t track = DEFAULT_TRACK;
};

#pragma once

#include <NimBLEDevice.h>

class BLEService;

class MusicService : public BLECharacteristicCallbacks {

public:
  MusicService(BLEServer* iServer);

  void onWrite(BLECharacteristic* characteristic);

  void play();

  BLEService* service = nullptr;

  bool state = true; // on / off
  uint8_t volume = 5;
  uint8_t track = 0;
};

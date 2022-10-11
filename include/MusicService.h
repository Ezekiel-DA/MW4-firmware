#pragma once

#include <NimBLEDevice.h>

#include "config.h"
#include "settingsManager.h"

class BLEService;

class MusicService : public BLECharacteristicCallbacks {

public:
  MusicService(BLEServer* iServer, MusicSettings* iSettings);

  void onWrite(BLECharacteristic* characteristic);

  void play();

  BLEService* service = nullptr;

  MusicSettings* settings;
};

#pragma once

#include <NimBLEDevice.h>

class BLEService;

class CostumeControlService : public BLECharacteristicCallbacks {
private:
  

public:
  CostumeControlService(BLEServer* iServer, uint8_t iVersion);
  
  void onWrite(BLECharacteristic* characteristic);

  BLEService* service = nullptr;

private:
  uint8_t _fwVersion;
};

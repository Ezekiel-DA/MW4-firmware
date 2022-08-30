#pragma once

#include <BLEDevice.h>

class BLEService;

class CostumeControlService : public BLECharacteristicCallbacks {
private:
  BLEService* _service = nullptr;

public:
  CostumeControlService(BLEServer* iServer, uint8_t iVersion);
  
  void onWrite(BLECharacteristic* characteristic);

private:
  uint8_t _fwVersion;
};

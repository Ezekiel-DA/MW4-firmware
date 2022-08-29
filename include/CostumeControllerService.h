#pragma once

#include <BLEDevice.h>

class BLEService;

class CostumeControlService : public BLECharacteristicCallbacks {
private:
  BLEService* _service = nullptr;

public:
  CostumeControlService(BLEServer* iServer);
  
  void onWrite(BLECharacteristic* characteristic);

  std::string _text = "I WANT YOU";
  bool _customOffset = true;
  int16_t _offset = 0;
};

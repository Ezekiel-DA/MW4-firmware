#pragma once

#include <NimBLEDevice.h>

#include "settingsManager.h"
#include "LEDChannel.h"

class BLEService;

class LightDeviceService : public BLECharacteristicCallbacks {

public:
  LightDeviceService(BLEServer* iServer, const std::string& iName, LightDeviceSettings* iSettings, LightDeviceSettings* iSettingsAlt, uint8_t iId=0);

  static void globalAnimationUpdate();

  void update(bool iAltMode=false);  
  void onWrite(BLECharacteristic* characteristic);

  BLEService* service = nullptr;

  LightDeviceSettings* settings;
  LightDeviceSettings* settingsAlt;

  std::vector<LEDChannel*> channels;

private:
  uint32_t prevUpdate;
  uint8_t id;

  void createBLECharacteristics(LightDeviceSettings& settings, bool iAltMode=false);

  BLECharacteristic* createCharacteristic(const char** uuids, const char* iDisplayName, bool iAltMode=false);
};


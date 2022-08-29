#pragma once

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#include "config.h"

struct MWNEXTDeviceInfo {
  MWNEXT_DEVICE_TYPE type;
  BLEUUID uuid;
  std::string name;
};

struct LightDataBlock {
  uint8_t cycleColor : 1;
  uint8_t patternID : 7; // max 128 patterns, wich should be more than fine!
  uint8_t hue;
  uint8_t saturation;
};

class LightService : public BLECharacteristicCallbacks {
private:
  MWNEXTDeviceInfo _deviceInfo;
  BLEService* _service = nullptr;

public:
  LightDataBlock _lightData;

  LightService(BLEServer* iServer, MWNEXTDeviceInfo& iDeviceInfo);
  
  void onWrite(BLECharacteristic* characteristic);

  void setHue(uint8_t iHue);
  void setSaturation(uint8_t iSaturation);
  void setPatternID(uint8_t iPatternID);
  void setCycleColor(bool iCycleColor);

  // used after unstreaming a tag, to force notification of new values, since unstream is done via memcpy and not the above setters!
  void forceBLEUpdate();

  void debugDump();
};

#pragma once

#include <bitset>

#include <NimBLEDevice.h>

#include "config.h"

constexpr std::bitset<8> CAPABILITY_COLOR       {	1 << 0 }; // 0000 0001
constexpr std::bitset<8> CAPABILITY_PATTERN     {	1 << 1 }; // 0000 0010
constexpr std::bitset<8> CAPABILITY_PWM         {	1 << 2 }; // 0000 0100
constexpr std::bitset<8> CAPABILITY_ADDRESSABLE {	1 << 3 }; // 0000 1000

// static information about a device, to be set at build time (code build time as well as costume build time)
struct MW4DeviceInfo {
  std::string name;
  uint8_t deviceId;
  uint16_t shapeH;
  uint16_t shapeV;
  std::bitset<8> capabilities;
};

// dynamic information about a light device
struct LightData {
  uint8_t cycleColor : 1;
  uint8_t state : 1; // on / off
  uint8_t reserved : 1; // for future use
  uint8_t patternID : 5; // max 32 patterns, wich should be more than fine!
  uint8_t hue;
  uint8_t saturation;
  uint8_t value;
};

class LightService : public BLECharacteristicCallbacks {
private:
  MW4DeviceInfo _deviceInfo;
  BLEService* _service = nullptr;

public:
  LightData _lightData;

  // TODO store addressable light data buffer? somehow?

  LightService(BLEServer* iServer, MW4DeviceInfo& iDeviceInfo);
  
  void onWrite(BLECharacteristic* characteristic);

  void setHue(uint8_t iHue);
  void setSaturation(uint8_t iSaturation);
  void setPatternID(uint8_t iPatternID);
  void setCycleColor(bool iCycleColor);

  // used after unstreaming a tag, to force notification of new values, since unstream is done via memcpy and not the above setters!
  void forceBLEUpdate();

  void debugDump();
};

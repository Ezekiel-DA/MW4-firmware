#pragma once

#include <string>

#include <BLEServer.h>

class BLECharacteristic;
class BLEService;
class BLEUUID;

static bool deviceConnected = false;

/**
 * Create a new BLE Descriptor with the 0x2901 UUID (i.e. "Characteristic User Description"), set it to the provided name, and attach it to the given BLE Characteristic.
 * Note: to save on memory, names should not exceed 15 characters.
 */
void attachUserDescriptionToCharacteristic(BLECharacteristic* iCharacteristic, const std::string& iName);

/**
 * DO NOT USE. This seems to break characteristics / services it's applied to? I guess I'm using it incorrectly, but I'm not sure why at this point.
 * Set various formatting information for a characteristic, via the Characteristic Presentation Format Descriptor (UUID: 0x2904).
 * Right now, only supports setting a type; the constants for this can be found in BLE2904.h from arduino-esp32
 */
void setCharacteristicPresentationFormat(BLECharacteristic* iCharacteristic, uint8_t iType);

BLECharacteristic* getCharacteristicByUUID(BLEService* iService, BLEUUID iCharacteristicID);


class ServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* server);
  void onDisconnect(BLEServer* server);
};
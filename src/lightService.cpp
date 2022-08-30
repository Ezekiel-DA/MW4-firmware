#include "LightService.h"

#include <Arduino.h>
#include <BLE2904.h>
#include <BLE2902.h>

#include "BLE.h"

static uint8_t instId = 0;

LightService::LightService(BLEServer* iServer, MW4DeviceInfo& iDeviceInfo) : _deviceInfo(iDeviceInfo) {
  _lightData.cycleColor = 0;
  _lightData.patternID = 0;
  _lightData.hue = 0;
  _lightData.saturation = 255;

  // NB: the default of 15 handles (for Characteristics, Descriptors, etc.) is definitely not enough; hopefully this should do for a bit!
  // NB2: with this change, adding Presentation Format Descriptors would probably not break anymore!
  _service = iServer->createService(BLEUUID(MW4_BLE_LIGHT_DEVICE_SERVICE_UUID), 100, instId++);

  auto objectName = _service->createCharacteristic((uint16_t)0x2ABE, BLECharacteristic::PROPERTY_READ);
  objectName->setValue(_deviceInfo.name);
  // setCharacteristicPresentationFormat(objectName, BLE2904::FORMAT_UTF8);

  auto id = _service->createCharacteristic(MW4_BLE_ID_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);
  id->setValue(&_deviceInfo.deviceId, 1);
  attachUserDescriptionToCharacteristic(id, "Id");

  auto capabilities = _service->createCharacteristic(MW4_BLE_CAPABILITIES_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);
  uint8_t capa_tmp = static_cast<uint8_t>(_deviceInfo.capabilities.to_ullong());
  capabilities->setValue(&capa_tmp, 1);
  attachUserDescriptionToCharacteristic(capabilities, "Capabilities");

  auto state = _service->createCharacteristic(MW4_BLE_STATE_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE);
  state->setCallbacks(this);
  uint8_t stateStartVal = _lightData.state;
  state->setValue(&stateStartVal, 1);
  attachUserDescriptionToCharacteristic(state, "Device state");
  //  setCharacteristicPresentationFormat(cycleColor, BLE2904::FORMAT_BOOLEAN);
  state->addDescriptor(new BLE2902());

  if ((_deviceInfo.capabilities & CAPABILITY_PATTERN).any()) {
    auto mode = _service->createCharacteristic(MW4_BLE_MODE_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE);
    mode->setCallbacks(this);
    uint8_t modeStartVal = _lightData.patternID;
    mode->setValue(&modeStartVal, 1);
    attachUserDescriptionToCharacteristic(mode, "Mode");
    // setCharacteristicPresentationFormat(mode, BLE2904::FORMAT_UINT8);
    mode->addDescriptor(new BLE2902());
  }

  if ((_deviceInfo.capabilities & CAPABILITY_COLOR).any()) {
    auto cycleColor = _service->createCharacteristic(MW4_BLE_CYCLE_COLOR_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE);
    cycleColor->setCallbacks(this);
    uint8_t cycleColorStartVal = _lightData.cycleColor;
    cycleColor->setValue(&cycleColorStartVal, 1);
    attachUserDescriptionToCharacteristic(cycleColor, "Cycle color");
    //  setCharacteristicPresentationFormat(cycleColor, BLE2904::FORMAT_BOOLEAN);
    cycleColor->addDescriptor(new BLE2902());

    auto hue = _service->createCharacteristic(MW4_BLE_HUE_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE);
    hue->setCallbacks(this);
    hue->setValue(&_lightData.hue, 1);
    attachUserDescriptionToCharacteristic(hue, "Hue");
    //  setCharacteristicPresentationFormat(hue, BLE2904::FORMAT_UINT8);
    hue->addDescriptor(new BLE2902());

    auto saturation = _service->createCharacteristic(MW4_BLE_SATURATION_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE);
    saturation->setCallbacks(this);
    saturation->setValue(&_lightData.saturation, 1);
    attachUserDescriptionToCharacteristic(saturation, "Saturation");
    // setCharacteristicPresentationFormat(saturation, BLE2904::FORMAT_UINT8);
    saturation->addDescriptor(new BLE2902());
  }

  if ((_deviceInfo.capabilities & CAPABILITY_PWM).any()) {
    auto value = _service->createCharacteristic(MW4_BLE_VALUE_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE);
    value->setCallbacks(this);
    value->setValue(&_lightData.value, 1);
    attachUserDescriptionToCharacteristic(value, "Value");
    // setCharacteristicPresentationFormat(saturation, BLE2904::FORMAT_UINT8);
    value->addDescriptor(new BLE2902());
  }

  _service->start();
};

void LightService::onWrite(BLECharacteristic* characteristic) {
    BLEUUID id = characteristic->getUUID();
    uint8_t* val = characteristic->getData();

    // Reminder: we intentionally bypass the setters here, since they would cause a BLE notify, but these values just came from a BLE write

    // Serial.print("Got new value from central for Service "); Serial.print(this->_deviceInfo.name.c_str()); Serial.print(" for characteristic ");
    if (id.equals(std::string(MW4_BLE_HUE_CHARACTERISTIC_UUID))) {
        // Serial.print("Hue");
        this->_lightData.hue = (uint8_t) *val;
    } else if (id.equals(std::string(MW4_BLE_CYCLE_COLOR_CHARACTERISTIC_UUID))) {
        // Serial.print("Cycle Color");
        this->_lightData.cycleColor = (bool) *val;
    } else if (id.equals(std::string(MW4_BLE_MODE_CHARACTERISTIC_UUID))) {
        // Serial.print("Mode");
        this->_lightData.patternID = *val;
    } else if (id.equals(std::string(MW4_BLE_SATURATION_CHARACTERISTIC_UUID))) {
        // Serial.print("Saturation");
        this->_lightData.saturation = *val;
    } else {
        Serial.print("Unrecognized characteristic!!!");
    }

    // Serial.print(": "); Serial.println(*val);
}

void LightService::setHue(uint8_t iHue) {
  _lightData.hue = iHue;
  auto characteristic = getCharacteristicByUUID(_service, BLEUUID(MW4_BLE_HUE_CHARACTERISTIC_UUID));
  characteristic->setValue(&_lightData.hue, 1);
  characteristic->notify();
}

void LightService::setSaturation(uint8_t iSaturation) {
  _lightData.saturation = iSaturation;
  auto characteristic = getCharacteristicByUUID(_service, BLEUUID(MW4_BLE_SATURATION_CHARACTERISTIC_UUID));
  characteristic->setValue(&_lightData.saturation, 1);
  characteristic->notify();
}

void LightService::setPatternID(uint8_t iPatternID) {
  _lightData.patternID = iPatternID;
  auto characteristic = getCharacteristicByUUID(_service, BLEUUID(MW4_BLE_MODE_CHARACTERISTIC_UUID));
  characteristic->setValue(&iPatternID, 1);
  characteristic->notify();
}

void LightService::setCycleColor(bool iCycleColor) {
  _lightData.cycleColor = iCycleColor;
  uint8_t tempCycleColor = iCycleColor; // because we need to be able to take the address of this value converted to a uint8_t, which neither our input bool or this bitfield will allow
  auto characteristic = getCharacteristicByUUID(_service, BLEUUID(MW4_BLE_CYCLE_COLOR_CHARACTERISTIC_UUID));
  characteristic->setValue(&tempCycleColor, 1);
  characteristic->notify();
}

void LightService::forceBLEUpdate() {
  // yes, this is very dumb and should be cleaned up; we should probably unserialize tags in a way that calls the setters, instead
  setPatternID(_lightData.patternID);
  
  if ((_deviceInfo.capabilities & CAPABILITY_COLOR).any()) {
    setHue(_lightData.hue);
    setSaturation(_lightData.saturation);
    setCycleColor(_lightData.cycleColor);
  }
}

void LightService::debugDump() {
    Serial.print(_deviceInfo.name.c_str()); Serial.print("\t\t\t cycle: "); Serial.print(_lightData.cycleColor); Serial.print(" - patternID: "); Serial.print(_lightData.patternID); Serial.print(" - hue: "); Serial.print(_lightData.hue); Serial.print(" - Saturation: ");  Serial.println(_lightData.saturation);
  };

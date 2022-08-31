#include <Arduino.h>

#include <AceButton.h>
using namespace ace_button;

#include "config.h"
#include "utils.h"
#include "buttons.h"
#include "BLE.h"
#include "CostumeControllerService.h"
#include "TextDisplayService.h"
#include "LightService.h"

#define FW_VERSION 1

CostumeControlService* costumeController = nullptr;
TextDisplayService* frontText = nullptr;

void setup()
{
  Serial.begin(115200);

  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);

  setupButtons();

  auto bleServer = setupBLE();

  costumeController = new CostumeControlService(bleServer, FW_VERSION);
  frontText = new TextDisplayService(bleServer, "I WANT YOU");

  costumeController->_service->start();
  frontText->_service->start();

  BLEDevice::startAdvertising();

  Serial.print("MW4 init complete - running version: "); Serial.println(FW_VERSION);
}

void loop()
{
  checkButtons();

  if (!deviceConnected && oldDeviceConnected) {
    delay(100);
    BLEDevice::startAdvertising();
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  if (deviceConnected && !oldDeviceConnected) {
    Serial.println("main loop started");
    oldDeviceConnected = deviceConnected;
  }

  frontText->update();
}

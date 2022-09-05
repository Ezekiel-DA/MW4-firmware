#include <Arduino.h>

#include <AceButton.h>
using namespace ace_button;

#include "config.h"
#include "utils.h"
#include "buttons.h"
#include "BLE.h"
#include "CostumeControllerService.h"
#include "TextDisplayService.h"
#include "LightDeviceService.h"

#define FW_VERSION 1

CostumeControlService* costumeController = nullptr;
TextDisplayService* frontText = nullptr;
LightDeviceService* frontStrip = nullptr;
LightDeviceService* backStrip = nullptr;
LightDeviceService* racetrackStrip = nullptr;

void setup()
{
  Serial.begin(115200);

  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);

  setupButtons();

  auto bleServer = setupBLE();

  costumeController = new CostumeControlService(bleServer, FW_VERSION);
  frontText = new TextDisplayService(bleServer, "I WANT YOU");
  frontStrip = new LightDeviceService(bleServer, 200, "Front strip");
  addLEDsToLightDeviceService<FRONT_STRIP_PIN>(frontStrip);
  backStrip = new LightDeviceService(bleServer, 400, "Back strip");
  addLEDsToLightDeviceService<BACK_STRIP_PIN>(backStrip);
  racetrackStrip = new LightDeviceService(bleServer, 300, "Racetrack");
  addLEDsToLightDeviceService<RACETRACK_STRIP_PIN>(racetrackStrip);

  costumeController->service->start();
  frontText->service->start();
  frontStrip->service->start();
  backStrip->service->start();
  racetrackStrip->service->start();

  BLEDevice::startAdvertising();

  Serial.print("MW4 init complete - running version: "); Serial.println(FW_VERSION);
}

void loop()
{
  checkButtons();

  if (altMode) {
    digitalWrite(STATUS_LED_PIN, HIGH);
  } else {
    digitalWrite(STATUS_LED_PIN, LOW);
  }

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
  frontStrip->update();
  backStrip->update();
  racetrackStrip->update();

  FastLED.show();
}

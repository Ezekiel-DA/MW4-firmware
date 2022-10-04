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
#include "MusicService.h"
#include "SD_FS.h"
#include "SX1509.h"

#define FW_VERSION 1

CostumeControlService* costumeController = nullptr;
TextDisplayService* frontText = nullptr;

LightDeviceService* racetrackStrip = nullptr;
LightDeviceService* bottomVStrip = nullptr;
LightDeviceService* frontUStrip = nullptr;
LightDeviceService* backUStrip = nullptr;
LightDeviceService* backScreenStrip = nullptr;
LightDeviceService* pedestalStrip = nullptr;

extern SX1509 io;

void setup()
{
  Serial.begin(115200);

  setupButtons();
  
  // setup optional modules
  //setupSX1509();
  setupSD();
  audioInit();

  auto bleServer = setupBLE();
  costumeController = new CostumeControlService(bleServer, FW_VERSION);

  // LED strips
  frontText = new TextDisplayService(bleServer, "I WANT YOU");
  addLEDsToTextDisplayService<FRONT_TEXT_PIN>(frontText);
  racetrackStrip = new LightDeviceService(bleServer, 300, "Racetrack");
  addLEDsToLightDeviceService<RACETRACK_STRIP_PIN>(racetrackStrip);
  bottomVStrip = new LightDeviceService(bleServer, 20, "Bottom V");
  addLEDsToLightDeviceService<BOTTOM_V_PIN>(bottomVStrip);
  frontUStrip = new LightDeviceService(bleServer, 155, "Front U");
  addLEDsToLightDeviceService<FRONT_U_PIN>(frontUStrip);
  backUStrip = new LightDeviceService(bleServer, 120, "Back U");
  addLEDsToLightDeviceService<BACK_U_PIN>(backUStrip);
  backScreenStrip = new LightDeviceService(bleServer, 200, "Back screen");
  backScreenStrip->mode = 0; // steady
  addLEDsToLightDeviceService<BACK_SCREEN_PIN>(backScreenStrip);
  pedestalStrip = new LightDeviceService(bleServer, 300, "Pedestal");
  addLEDsToLightDeviceService<PEDESTAL_PIN>(pedestalStrip);

  costumeController->service->start();
  frontText->service->start();
  racetrackStrip->service->start();
  bottomVStrip->service->start();
  frontUStrip->service->start();
  backUStrip->service->start();
  backScreenStrip->service->start();
  pedestalStrip->service->start();

  BLEDevice::startAdvertising();

  Serial.print("MW4 init complete - running version: "); Serial.println(FW_VERSION);
}

std::string title;
std::string artist;

void loop()
{
  //checkButtons();

  // if (altMode) {
  //   io.digitalWrite(SX1509_STATUS_LED_PIN, HIGH);
  //   frontText->settingsAlt.text = artist + " - " + title;
  // } else {
  //   io.digitalWrite(SX1509_STATUS_LED_PIN, LOW);
  // }

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

  if (costumeController->dangerZone) {
    Serial.println("danger");
  }

  LightDeviceService::globalAnimationUpdate();
  
  // WARNING: DO NOT DISABLE ANY OF THESE! These end up calling each FastLED controller separately (so we can update at different speeds to deal with our sheer amount of pixels).
  // I'm not sure why but it looks like failing to update all controllers at least occasionally causes FastLED to hang?
  frontText->update(altMode);
  racetrackStrip->update();
  bottomVStrip->update();
  frontUStrip->update();
  backUStrip->update();
  backScreenStrip->update();
  pedestalStrip->update();
}

void audio_id3data(const char *info){  //id3 metadata
  std::string id3info = info;
  if (id3info.find("Title:") != std::string::npos) {
    title = id3info.substr(6);
  } else if (id3info.find("Artist:") != std::string::npos) {
    artist = id3info.substr(7);
  }
}

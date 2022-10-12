#include <Arduino.h>

#include <AceButton.h>
using namespace ace_button;

#include "config.h"
#include "buttons.h"
#include "BLE.h"
#include "CostumeControllerService.h"
#include "TextDisplayService.h"
#include "LightDeviceService.h"
#include "MusicService.h"
#include "SD_FS.h"
#include "settingsManager.h"

CostumeControlService* costumeController = nullptr;
TextDisplayService* frontText = nullptr;

LightDeviceService* racetrackStrip = nullptr;
LightDeviceService* bottomVStrip = nullptr;
LightDeviceService* frontUStrip = nullptr;
LightDeviceService* backUStrip = nullptr;
LightDeviceService* backScreenStrip = nullptr;
LightDeviceService* pedestalStrip = nullptr;

MusicService* musicService = nullptr;

void setup()
{
  Serial.begin(115200);

  setupButtons();
  setupSD();

  initSettings();

  auto bleServer = setupBLE();
  costumeController = new CostumeControlService(bleServer, FW_VERSION);

  // LED strips
  frontText = new TextDisplayService(bleServer, getTextDisplaySettings(), getTextDisplaySettings(true));
  addLEDsToTextDisplayService<FRONT_TEXT_PIN>(frontText);

  racetrackStrip = new LightDeviceService(bleServer, RACETRACK_NUM_LEDS, "Racetrack", getChairLightSettings(), getChairLightSettings(true));
  addLEDsToLightDeviceService<RACETRACK_STRIP_PIN>(racetrackStrip);
  bottomVStrip = new LightDeviceService(bleServer, BOTTOM_V_NUM_LEDS, "Bottom V", getChairLightSettings(), getChairLightSettings(true));
  addLEDsToLightDeviceService<BOTTOM_V_PIN>(bottomVStrip);
  frontUStrip = new LightDeviceService(bleServer, FRONT_U_NUM_LEDS, "Front U", getChairLightSettings(), getChairLightSettings(true));
  addLEDsToLightDeviceService<FRONT_U_PIN>(frontUStrip);
  backUStrip = new LightDeviceService(bleServer, BACK_U_NUM_LEDS, "Back U", getChairLightSettings(), getChairLightSettings(true));
  addLEDsToLightDeviceService<BACK_U_PIN>(backUStrip);
  backScreenStrip = new LightDeviceService(bleServer, BACK_SCREEN_NUM_LEDS, "Back screen", getChairLightSettings(), getChairLightSettings(true));
  backScreenStrip->_backScreenHack = true;
  addLEDsToLightDeviceService<BACK_SCREEN_PIN>(backScreenStrip);

  pedestalStrip = new LightDeviceService(bleServer, PEDESTAL_NUM_LEDS, "Pedestal", getPedestalLightSettings(), getPedestalLightSettings(true));
  addLEDsToLightDeviceService<PEDESTAL_PIN>(pedestalStrip);

  musicService = new MusicService(bleServer, getMusicSettings());

  costumeController->service->start();
  frontText->service->start();
  racetrackStrip->service->start();
  bottomVStrip->service->start();
  frontUStrip->service->start();
  backUStrip->service->start();
  backScreenStrip->service->start();
  pedestalStrip->service->start();

  musicService->service->start();

  BLEDevice::startAdvertising();

  Serial.print("MW4 init complete - running version: "); Serial.println(FW_VERSION);
}

std::string title;
std::string artist;

void loop()
{
  checkButtons();

  bool altMode = getAltMode();
  bool pressed = getAndResetPressed();

  if (pressed) {
    musicService->play();
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

  if (costumeController->dangerZone) {
    Serial.println("danger");
  }

  LightDeviceService::globalAnimationUpdate();

  frontText->update(altMode);
  racetrackStrip->update(altMode);
  bottomVStrip->update(altMode);
  frontUStrip->update(altMode);
  backUStrip->update(altMode);
  backScreenStrip->update(altMode);
  pedestalStrip->update(altMode);

  FastLED.show();
}

void audio_id3data(const char *info){  //id3 metadata
  std::string id3info = info;
  if (id3info.find("Title:") != std::string::npos) {
    title = id3info.substr(6);
  } else if (id3info.find("Artist:") != std::string::npos) {
    artist = id3info.substr(7);
  }
}

void audio_eof_mp3(const char *info) {
  Serial.println("mp3 playback ended");
}

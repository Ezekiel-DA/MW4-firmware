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

LightDeviceService* chairLights = nullptr;
LightDeviceService* pedestalLights = nullptr;

MusicService* musicService = nullptr;

void setup()
{
  Serial.begin(115200);

  setupButtons();
  setupSD();

  initSettings();

  auto bleServer = setupBLE();
  costumeController = new CostumeControlService(bleServer, FW_VERSION);

  // define LED strip channels
  auto racetrackChannel = new LEDChannel(RACETRACK_NUM_LEDS);
  addLEDsToChannel<RACETRACK_STRIP_PIN>(racetrackChannel);
  auto bottomVChannel = new LEDChannel(BOTTOM_V_NUM_LEDS);
  addLEDsToChannel<BOTTOM_V_PIN>(bottomVChannel);
  auto frontUChannel = new LEDChannel(FRONT_U_NUM_LEDS);
  addLEDsToChannel<FRONT_U_PIN>(frontUChannel);
  auto backUChannel = new LEDChannel(BACK_U_NUM_LEDS);
  addLEDsToChannel<BACK_U_PIN>(backUChannel);
  auto backScreenChannel = new LEDChannel(BACK_SCREEN_NUM_LEDS, /*backScreenHack*/true);
  addLEDsToChannel<BACK_SCREEN_PIN>(backScreenChannel);
  auto pedestalChannel = new LEDChannel(PEDESTAL_NUM_LEDS);
  addLEDsToChannel<PEDESTAL_PIN>(pedestalChannel);
  
  // define front text service and attach LEDs
  frontText = new TextDisplayService(bleServer, getTextDisplaySettings(), getTextDisplaySettings(true));
  addLEDsToTextDisplayService<FRONT_TEXT_PIN>(frontText);

  // define LED services and attach groups of LED channels to them
  chairLights = new LightDeviceService(bleServer, "Chair lights", getChairLightSettings(), getChairLightSettings(true), /*id*/1);
  chairLights->channels.push_back(racetrackChannel);
  chairLights->channels.push_back(bottomVChannel);
  chairLights->channels.push_back(frontUChannel);
  chairLights->channels.push_back(backUChannel);
  chairLights->channels.push_back(backScreenChannel);

  pedestalLights = new LightDeviceService(bleServer, "Pedestal lights", getPedestalLightSettings(), getPedestalLightSettings(true), /*id*/2);
  pedestalLights->channels.push_back(pedestalChannel);
  
  musicService = new MusicService(bleServer, getMusicSettings());

  costumeController->service->start();
  frontText->service->start();
  chairLights->service->start();
  pedestalLights->service->start();
  musicService->service->start();
  BLEDevice::startAdvertising();

  Serial.print("MW4 init complete - running version: "); Serial.println(FW_VERSION);
}

std::string title;
std::string artist;

void loop()
{
  checkButtons();

  if (getReset()) {
    Serial.print("Resetting settings to defaults and rebooting...");
    resetSettings();
  }

  bool altMode = getAltMode();
  bool pressed = getAndResetPressed();

  if (pressed) {
    musicService->play();
  }

  if (costumeController->dangerZone) {
    Serial.println("danger");
  }

  if (!costumeController->OTAUpdateInProgress) {
    LightDeviceService::globalAnimationUpdate();

    frontText->update(altMode);
    chairLights->update(altMode);
    pedestalLights->update(altMode);

    FastLED.show();

    saveSettings();
  }
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

#include <Arduino.h>
#include <BLEServer.h>

#include <AceButton.h>
using namespace ace_button;

#include "config.h"
#include "utils.h"
#include "buttons.h"
#include "BLE.h"
#include "CostumeControllerService.h"
#include "TextDisplayService.h"
#include "LightService.h"

#define FW_VERSION 2

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

  Serial.print("MW4 init complete - running version: "); Serial.println(FW_VERSION);
}

void loop()
{
  checkButtons();

  frontText->update();
}

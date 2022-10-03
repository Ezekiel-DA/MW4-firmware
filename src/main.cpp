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

#include <Wire.h> // for SX1509 I2C
#include <SparkFunSX1509.h>

// SX1509 I2C address (set by ADDR1 and ADDR0 (00 by default):
const byte SX1509_ADDRESS = 0x3E; // SX1509 I2C address
SX1509 io;                        // Create an SX1509 object to be used throughout

// SX1509 Pin definition:
const byte SX1509_LED_PIN = 15; // LED to SX1509's pin 15


#define FW_VERSION 1

CostumeControlService* costumeController = nullptr;
TextDisplayService* frontText = nullptr;
LightDeviceService* frontStrip = nullptr;
LightDeviceService* backStrip = nullptr;
LightDeviceService* racetrackStrip = nullptr;

void setup()
{
  Serial.begin(115200);

  // TODO: move this: SX1509 LED output setup
  Wire.begin();

  if (io.begin(SX1509_ADDRESS) == false)
  {
    Serial.println("Failed to communicate. Check wiring and address of SX1509.");
    while (1)
      ; // If we fail to communicate, loop forever.
  }

  // Use the internal 2MHz oscillator.
  // Set LED clock to 500kHz (2MHz / (2^(3-1)):
  //io.clock(INTERNAL_CLOCK_2MHZ, 3);

  // To breathe an LED, make sure you set it as an
  // ANALOG_OUTPUT, so we can PWM the pin:
  //io.pinMode(SX1509_LED_PIN, ANALOG_OUTPUT);
  io.pinMode(SX1509_LED_PIN, OUTPUT);

  // Breathe an LED: 1000ms LOW, 500ms HIGH,
  // 500ms to rise from low to high
  // 250ms to fall from high to low
  //io.breathe(SX1509_LED_PIN, 10, 10000, 1, 1);
  //io.blink(SX1509_LED_PIN, 5, 10);
  io.digitalWrite(SX1509_LED_PIN, HIGH);

  // TODO move the above (SWX1509 led setup)


  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);

  setupButtons();
  setupSD();

  audioInit();

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

std::string title;
std::string artist;

void loop()
{
  checkButtons();

  if (altMode) {
    digitalWrite(STATUS_LED_PIN, HIGH);
    frontText->settingsAlt.text = artist + " - " + title;
    io.breathe(SX1509_LED_PIN, 10000, 10, 1, 1);
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

  frontText->update(altMode);
  frontStrip->update();
  backStrip->update();
  racetrackStrip->update();

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

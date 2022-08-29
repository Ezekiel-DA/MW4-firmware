#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#include <AceButton.h>
using namespace ace_button;

#define FONT_6x8_FIXED_MEDIUM
#include <StripDisplay.h>

#include "config.h"
#include "utils.h"
#include "buttons.h"
#include "BLE.h"
#include "CostumeControllerService.h"
#include "lightService.h"

XBMFont *fontP = &fixedMedium_6x8;

#define STRIPLED_GPIO 14
#define STRIPLED_W 64
#define STRIPLED_H 8

CRGB leds[STRIPLED_W * STRIPLED_H];

StripDisplay strip(STRIPLED_GPIO, STRIPLED_W, STRIPLED_H, WRAP_COLUMNS, ORIGIN_TOP_LEFT, leds);

CostumeControlService *costumeController = nullptr;

void setup()
{
  Serial.begin(115200);

  FastLED.addLeds<WS2812B, STRIPLED_GPIO, GRB>(leds, STRIPLED_W * STRIPLED_H);
  FastLED.setBrightness(10);
  strip.setup(fontP);
  Serial.begin(115200);

  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);

  setupButtons();

  BLEDevice::init("MagicWheelchair-Next");
  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  costumeController = new CostumeControlService(server);

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->setAppearance(0x0CC1); // powered wheelchair appearance

  // advertising: we wanna advertise a service with a fixed "name" (UUID), so that clients can look for this, decide to connect based on that,
  // and then enumerate the other services (one per logical "device" on the costume), which do not need to be advertised.
  pAdvertising->addServiceUUID(MWNEXT_BLE_COSTUME_CONTROL_SERVICE_UUID);

  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue // NLV: what the heck is this? Taken from the ESP32 samples, should probably test what "iPhone issue" it's talking about...
  pAdvertising->setMinPreferred(0x12);

  BLEDevice::startAdvertising();
  Serial.println("BLE init complete.");
}

void loop()
{
  static int16_t offset = -60;

  checkButtons();

  auto text = costumeController->_text;
  auto customOffset = costumeController->_offset;
  auto isCustomOffsetOn = costumeController->_customOffset;

  strip.setFgColor(CRGB::White);
  strip.setBgColor(CRGB(0, 0, 0));
  strip.setText(text.c_str());

  if (isCustomOffsetOn) {
    Serial.print("Displaying with offset "); Serial.println(customOffset);
    strip.displayText(customOffset);
    FastLED.delay(50);
  }
  // else {
    // strip.displayText(offset++);
    // FastLED.delay(50);

    // if (text.size() <= 10) { // only pause if text will fit
    //   if (offset == 0)
    //   {
    //     FastLED.delay(3000);
    //   }
    // }

    // if (offset == text.size() * 6)
    // {
    //   offset = -60;
    // }
  //}
}

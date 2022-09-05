#include "TextDisplayService.h"

#include <string>

#include <Arduino.h>
#include <NimBLEDevice.h>

#define FONT_6x8_FIXED_MEDIUM
#include <StripDisplay.h>

#include "config.h"
#include "BLE.h"

BLECharacteristic* TextDisplayService::createCharacteristic(const char* iDisplayName, const char* uuid) {
  auto characteristic = service->createCharacteristic(uuid, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE |NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::INDICATE);
  characteristic->setCallbacks(this);
  attachUserDescriptionToCharacteristic(characteristic, iDisplayName);

  return characteristic;
}

void TextDisplayService::createBLECharacteristics(TextDisplayServiceSettings& settings) {
  auto frontPanelText = createCharacteristic(MW4_BLE_TEXT_DISPLAY_TEXT_CHARACTERISTIC_UUID, "Front text");
  frontPanelText->setValue(settings.text);

  auto frontPanelOffset = createCharacteristic(MW4_BLE_TEXT_DISPLAY_OFFSET_CHARACTERISTIC_UUID, "Text offset");
  frontPanelOffset->setValue((uint16_t&) settings.offset);

  auto frontPanelCustomizeOffset = createCharacteristic(MW4_BLE_TEXT_DISPLAY_SCROLLING_CHARACTERISTIC_UUID, "Scrolling");
  frontPanelCustomizeOffset->setValue(reinterpret_cast<uint8_t*>(&(settings.scrolling)), 1);

  auto scrollSpeed = createCharacteristic(MW4_BLE_TEXT_DISPLAY_SCROLL_SPEED_CHARACTERISTIC_UUID, "Scroll speed");
  scrollSpeed->setValue(&(settings.scrollSpeed), 1);

  auto pauseTime = createCharacteristic(MW4_BLE_TEXT_DISPLAY_PAUSE_TIME_CHARACTERISTIC_UUID, "Pause time");
  pauseTime->setValue(&(settings.pauseTime), 1);

  auto frontPanelBrightness = createCharacteristic(MW4_BLE_TEXT_DISPLAY_BRIGHTNESS_CHARACTERISTIC_UUID, "Brightness");
  frontPanelBrightness->setValue(&(settings.brightness), 1);

  auto fgColor = createCharacteristic(MW4_BLE_TEXT_DISPLAY_FG_COLOR_CHARACTERISTIC_UUID, "Text color");
  fgColor->setValue(settings.fgColor, 3);

  auto bgColor = createCharacteristic(MW4_BLE_TEXT_DISPLAY_BG_COLOR_CHARACTERISTIC_UUID, "Bg. color");
  bgColor->setValue(settings.bgColor, 3);
}

TextDisplayService::TextDisplayService(BLEServer* iServer, const std::string& iDefaultText) {
  this->settings.text = iDefaultText;
  this->strip = new StripDisplay(FRONT_TEXT_PIN, STRIPLED_W, STRIPLED_H, WRAP_COLUMNS, ORIGIN_TOP_LEFT, this->leds);
  FastLED.addLeds<WS2812B, FRONT_TEXT_PIN, GRB>(leds, STRIPLED_W * STRIPLED_H);
  FastLED.setBrightness(this->settings.brightness);
  this->strip->setup(&fixedMedium_6x8);

  service = iServer->createService(MW4_BLE_TEXT_DISPLAY_SERVICE_UUID);

  createBLECharacteristics(settings);
  createBLECharacteristics(settingsAlt);

  Serial.println("Text Display init complete.");
}

void TextDisplayService::onWrite(BLECharacteristic* characteristic) {
    BLEUUID id = characteristic->getUUID();

    std::string safeData = characteristic->getValue();
    uint8_t* data = (uint8_t*)safeData.data();  
    
    if (id.equals(std::string(MW4_BLE_TEXT_DISPLAY_TEXT_CHARACTERISTIC_UUID))) {
      std::string val = characteristic->getValue();
      // Serial.print("New text: "); Serial.println(val.c_str());
      this->settings.text = val;
    }
    else if (id.equals(std::string(MW4_BLE_TEXT_DISPLAY_OFFSET_CHARACTERISTIC_UUID))) {
      uint8_t val[2] = { data[0], data[1] }; // useless since we don't need to swap endianness (the BLE spec specifies little endian, ESP32 is also little endian)
      auto offset = reinterpret_cast<int16_t*>(val);
      // Serial.println("New offset: "); Serial.println(std::to_string(static_cast<int>(*offset)).c_str());
      this->settings.offset = *offset;
    }
    else if (id.equals(std::string(MW4_BLE_TEXT_DISPLAY_SCROLLING_CHARACTERISTIC_UUID))) {
      this->settings.scrolling = (bool) *data;
    } else if (id.equals(std::string(MW4_BLE_TEXT_DISPLAY_BRIGHTNESS_CHARACTERISTIC_UUID))) {
      this->settings.brightness = *data;
    } else if (id.equals(std::string(MW4_BLE_TEXT_DISPLAY_FG_COLOR_CHARACTERISTIC_UUID))) {
      memcpy(this->settings.fgColor, data, 3);
    } else if (id.equals(std::string(MW4_BLE_TEXT_DISPLAY_BG_COLOR_CHARACTERISTIC_UUID))) {
      memcpy(this->settings.bgColor, data, 3);
    } else if (id.equals(std::string(MW4_BLE_TEXT_DISPLAY_SCROLL_SPEED_CHARACTERISTIC_UUID))) {
      this->settings.scrollSpeed = *data;
    } else if (id.equals(std::string(MW4_BLE_TEXT_DISPLAY_PAUSE_TIME_CHARACTERISTIC_UUID))) {
      this->settings.pauseTime = *data;
    }
}

void TextDisplayService::update() {
  static int16_t offset = STARTING_OFFSET;

  FastLED.setBrightness(this->settings.brightness);

  auto text = this->settings.text;
  auto customOffset = this->settings.offset;
  auto isCustomOffsetOn = !this->settings.scrolling;

  this->strip->setFgColor(CRGB(this->settings.fgColor[0], this->settings.fgColor[1], this->settings.fgColor[2]));
  this->strip->setBgColor(CRGB(this->settings.bgColor[0], this->settings.bgColor[1], this->settings.bgColor[2]));
  this->strip->setText(text.c_str());

  if (isCustomOffsetOn) {
    this->strip->displayText(customOffset);
  }
  else {
    this->strip->displayText(offset++);
    delay(this->settings.scrollSpeed);

    if (text.size() <= 10) { // only pause if text will fit
      if (offset == 0)
      {
         // (TODO: put this in a task instead of blocking and using this hack?)
        FastLED.delay(this->settings.pauseTime * 1000); // FastLED.delay calls FastLED.show(), which we want here
      }
    }

    if (offset == text.size() * 6)
    {
      offset = STARTING_OFFSET;
    }
  }
}

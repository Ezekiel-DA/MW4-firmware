#include "TextDisplayService.h"

#include <Arduino.h>
#include <NimBLEDevice.h>


#define FONT_6x8_FIXED_MEDIUM
#include <StripDisplay.h>

#include <string>

#include "config.h"
#include "BLE.h"

// TODO: this probably shouldn't be shared by every instance of the text display service... but then again, this build will only have one  ¯\_(ツ)_/¯
XBMFont *fontP = &fixedMedium_6x8;
CRGB leds[STRIPLED_W * STRIPLED_H];
StripDisplay strip(STRIPLED_GPIO, STRIPLED_W, STRIPLED_H, WRAP_COLUMNS, ORIGIN_TOP_LEFT, leds);

TextDisplayService::TextDisplayService(BLEServer* iServer, const std::string& iDefaultText) : _text(iDefaultText) {
  FastLED.addLeds<WS2812B, STRIPLED_GPIO, GRB>(leds, STRIPLED_W * STRIPLED_H);
  FastLED.setBrightness(this->_brightness);
  strip.setup(fontP);

  _service = iServer->createService(MW4_BLE_TEXT_DISPLAY_SERVICE_UUID);

  BLECharacteristic* frontPanelText = _service->createCharacteristic(MW4_BLE_TEXT_DISPLAY_TEXT_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE |NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::INDICATE);
  frontPanelText->setCallbacks(this);
  frontPanelText->setValue(this->_text);
  attachUserDescriptionToCharacteristic(frontPanelText, "Front text");

  BLECharacteristic* frontPanelOffset = _service->createCharacteristic(MW4_BLE_TEXT_DISPLAY_OFFSET_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE |NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::INDICATE);
  frontPanelOffset->setCallbacks(this);
  frontPanelOffset->setValue((uint16_t&) this->_offset);
  attachUserDescriptionToCharacteristic(frontPanelOffset, "Text offset");

  BLECharacteristic* frontPanelCustomizeOffset = _service->createCharacteristic(MW4_BLE_TEXT_DISPLAY_SCROLLING_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE |NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::INDICATE);
  frontPanelCustomizeOffset->setCallbacks(this);
  frontPanelCustomizeOffset->setValue(reinterpret_cast<uint8_t*>(&(this->_scrolling)), 1);
  attachUserDescriptionToCharacteristic(frontPanelCustomizeOffset, "Scrolling");

  BLECharacteristic* scrollSpeed = _service->createCharacteristic(MW4_BLE_TEXT_DISPLAY_SCROLL_SPEED_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE |NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::INDICATE);
  scrollSpeed->setCallbacks(this);
  scrollSpeed->setValue(&(this->_scrollSpeed), 1);
  attachUserDescriptionToCharacteristic(scrollSpeed, "Scroll speed");

  BLECharacteristic* pauseTime = _service->createCharacteristic(MW4_BLE_TEXT_DISPLAY_PAUSE_TIME_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE |NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::INDICATE);
  pauseTime->setCallbacks(this);
  pauseTime->setValue(&(this->_pauseTime), 1);
  attachUserDescriptionToCharacteristic(pauseTime, "Pause time");

  BLECharacteristic* frontPanelBrightness = _service->createCharacteristic(MW4_BLE_TEXT_DISPLAY_BRIGHTNESS_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE |NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::INDICATE);
  frontPanelBrightness->setCallbacks(this);
  frontPanelBrightness->setValue(&(this->_brightness), 1);
  attachUserDescriptionToCharacteristic(frontPanelBrightness, "Brightness");

  BLECharacteristic* fgColor = _service->createCharacteristic(MW4_BLE_TEXT_DISPLAY_FG_COLOR_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE |NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::INDICATE);
  fgColor->setCallbacks(this);
  fgColor->setValue(this->_fgColor, 3);
  attachUserDescriptionToCharacteristic(fgColor, "Text color");

   BLECharacteristic* bgColor = _service->createCharacteristic(MW4_BLE_TEXT_DISPLAY_BG_COLOR_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE |NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::INDICATE);
  bgColor->setCallbacks(this);
  bgColor->setValue(this->_bgColor, 3);
  attachUserDescriptionToCharacteristic(bgColor, "Bg. color");

  Serial.println("Text Display init complete.");
}

void TextDisplayService::onWrite(BLECharacteristic* characteristic) {
    BLEUUID id = characteristic->getUUID();

    std::string safeData = characteristic->getValue();
    uint8_t* data = (uint8_t*)safeData.data();  
    
    if (id.equals(std::string(MW4_BLE_TEXT_DISPLAY_TEXT_CHARACTERISTIC_UUID))) {
      std::string val = characteristic->getValue();
      // Serial.print("New text: "); Serial.println(val.c_str());
      this->_text = val;
    }
    else if (id.equals(std::string(MW4_BLE_TEXT_DISPLAY_OFFSET_CHARACTERISTIC_UUID))) {
      uint8_t val[2] = { data[0], data[1] }; // useless since we don't need to swap endianness (the BLE spec specifies little endian, ESP32 is also little endian)
      auto offset = reinterpret_cast<int16_t*>(val);
      // Serial.println("New offset: "); Serial.println(std::to_string(static_cast<int>(*offset)).c_str());
      this->_offset = *offset;
    }
    else if (id.equals(std::string(MW4_BLE_TEXT_DISPLAY_SCROLLING_CHARACTERISTIC_UUID))) {
      this->_scrolling = (bool) *data;
    } else if (id.equals(std::string(MW4_BLE_TEXT_DISPLAY_BRIGHTNESS_CHARACTERISTIC_UUID))) {
      this->_brightness = *data;
    } else if (id.equals(std::string(MW4_BLE_TEXT_DISPLAY_FG_COLOR_CHARACTERISTIC_UUID))) {
      memcpy(this->_fgColor, data, 3);
    } else if (id.equals(std::string(MW4_BLE_TEXT_DISPLAY_BG_COLOR_CHARACTERISTIC_UUID))) {
      memcpy(this->_bgColor, data, 3);
    } else if (id.equals(std::string(MW4_BLE_TEXT_DISPLAY_SCROLL_SPEED_CHARACTERISTIC_UUID))) {
      this->_scrollSpeed = *data;
    } else if (id.equals(std::string(MW4_BLE_TEXT_DISPLAY_PAUSE_TIME_CHARACTERISTIC_UUID))) {
      this->_pauseTime = *data;
    }
}

void TextDisplayService::update() {
  static int16_t offset = STARTING_OFFSET;

  FastLED.setBrightness(this->_brightness);

  auto text = this->_text;
  auto customOffset = this->_offset;
  auto isCustomOffsetOn = !this->_scrolling;

  strip.setFgColor(CRGB(this->_fgColor[0], this->_fgColor[1], this->_fgColor[2]));
  strip.setBgColor(CRGB(this->_bgColor[0], this->_bgColor[1], this->_bgColor[2]));
  strip.setText(text.c_str());

  if (isCustomOffsetOn) {
    strip.displayText(customOffset);
    FastLED.delay(50);
  }
  else {
    strip.displayText(offset++);
    FastLED.delay(this->_scrollSpeed);

    if (text.size() <= 10) { // only pause if text will fit
      if (offset == 0)
      {
        FastLED.delay(this->_pauseTime * 1000);
      }
    }

    if (offset == text.size() * 6)
    {
      offset = STARTING_OFFSET;
    }
  }
}

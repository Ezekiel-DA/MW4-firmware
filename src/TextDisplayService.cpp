#include "TextDisplayService.h"

#include <Arduino.h>
#include <NimBLEDevice.h>

#include <string>

#define FONT_6x8_FIXED_MEDIUM
#include <StripDisplay.h>

#include "BLE.h"
#include "config.h"

BLECharacteristic* TextDisplayService::createCharacteristic(const char** uuids, const char* iDisplayName, bool iAltMode) {
    auto characteristic = service->createCharacteristic(uuids[iAltMode], NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::INDICATE);
    characteristic->setCallbacks(this);
    attachUserDescriptionToCharacteristic(characteristic, !iAltMode ? iDisplayName : std::string(iDisplayName) + "(Alt.)");

    return characteristic;
}

void TextDisplayService::createBLECharacteristics(TextDisplaySettings& iSettings, bool iAltMode) {
    auto frontPanelText = createCharacteristic(MW4_BLE_TEXT_DISPLAY_TEXT_CHARACTERISTIC_UUIDS, "Front text", iAltMode);
    frontPanelText->setValue(iSettings.text);

    auto frontPanelOffset = createCharacteristic(MW4_BLE_TEXT_DISPLAY_OFFSET_CHARACTERISTIC_UUIDS, "Text offset", iAltMode);
    frontPanelOffset->setValue((uint16_t&)iSettings.offset);

    auto frontPanelCustomizeOffset = createCharacteristic(MW4_BLE_TEXT_DISPLAY_SCROLLING_CHARACTERISTIC_UUIDS, "Scrolling", iAltMode);
    frontPanelCustomizeOffset->setValue(reinterpret_cast<uint8_t*>(&(iSettings.scrolling)), 1);

    auto scrollSpeed = createCharacteristic(MW4_BLE_TEXT_DISPLAY_SCROLL_SPEED_CHARACTERISTIC_UUIDS, "Scroll speed", iAltMode);
    scrollSpeed->setValue(&(iSettings.scrollSpeed), 1);

    auto pauseTime = createCharacteristic(MW4_BLE_TEXT_DISPLAY_PAUSE_TIME_CHARACTERISTIC_UUIDS, "Pause time", iAltMode);
    pauseTime->setValue(&(iSettings.pauseTime), 1);

    auto frontPanelBrightness = createCharacteristic(MW4_BLE_TEXT_DISPLAY_BRIGHTNESS_CHARACTERISTIC_UUIDS, "Brightness", iAltMode);
    frontPanelBrightness->setValue(&(iSettings.brightness), 1);

    auto fgColor = createCharacteristic(MW4_BLE_TEXT_DISPLAY_FG_COLOR_CHARACTERISTIC_UUIDS, "Text color", iAltMode);
    fgColor->setValue(iSettings.fgColor, 3);

    auto bgColor = createCharacteristic(MW4_BLE_TEXT_DISPLAY_BG_COLOR_CHARACTERISTIC_UUIDS, "Bg. color", iAltMode);
    bgColor->setValue(iSettings.bgColor, 3);
}

TextDisplayService::TextDisplayService(BLEServer* iServer, TextDisplaySettings* iSettings, TextDisplaySettings* iSettingsAlt)
    : settings(iSettings), settingsAlt(iSettingsAlt) {
    this->strip = new StripDisplay(FRONT_TEXT_PIN, STRIPLED_W, STRIPLED_H, WRAP_COLUMNS, ORIGIN_TOP_LEFT, this->leds);
    this->strip->setup(&fixedMedium_6x8);

    service = iServer->createService(MW4_BLE_TEXT_DISPLAY_SERVICE_UUID);

    createBLECharacteristics(*settings);
    createBLECharacteristics(*settingsAlt, true);

    Serial.println("Text Display init complete.");
}

void TextDisplayService::onWrite(BLECharacteristic* characteristic) {
    BLEUUID id = characteristic->getUUID();

    std::string safeData = characteristic->getValue();
    uint8_t* data = (uint8_t*)safeData.data();

    if (id.equals(std::string(MW4_BLE_TEXT_DISPLAY_TEXT_CHARACTERISTIC_UUID))) {
        std::string val = characteristic->getValue();
        this->settings->text = val;
    } else if (id.equals(std::string(MW4_BLE_TEXT_DISPLAY_TEXT_ALT_CHARACTERISTIC_UUID))) {
        std::string val = characteristic->getValue();
        this->settingsAlt->text = val;
    } else if (id.equals(std::string(MW4_BLE_TEXT_DISPLAY_OFFSET_CHARACTERISTIC_UUID))) {
        auto offset = reinterpret_cast<int16_t*>(data);
        this->settings->offset = *offset;
    } else if (id.equals(std::string(MW4_BLE_TEXT_DISPLAY_OFFSET_ALT_CHARACTERISTIC_UUID))) {
        auto offset = reinterpret_cast<int16_t*>(data);
        this->settingsAlt->offset = *offset;
    } else if (id.equals(std::string(MW4_BLE_TEXT_DISPLAY_SCROLLING_CHARACTERISTIC_UUID))) {
        this->settings->scrolling = (bool)*data;
    } else if (id.equals(std::string(MW4_BLE_TEXT_DISPLAY_SCROLLING_ALT_CHARACTERISTIC_UUID))) {
        this->settingsAlt->scrolling = (bool)*data;
    } else if (id.equals(std::string(MW4_BLE_TEXT_DISPLAY_BRIGHTNESS_CHARACTERISTIC_UUID))) {
        this->settings->brightness = *data;
    } else if (id.equals(std::string(MW4_BLE_TEXT_DISPLAY_BRIGHTNESS_ALT_CHARACTERISTIC_UUID))) {
        this->settingsAlt->brightness = *data;
    } else if (id.equals(std::string(MW4_BLE_TEXT_DISPLAY_FG_COLOR_CHARACTERISTIC_UUID))) {
        memcpy(this->settings->fgColor, data, 3);
    } else if (id.equals(std::string(MW4_BLE_TEXT_DISPLAY_FG_COLOR_ALT_CHARACTERISTIC_UUID))) {
        memcpy(this->settingsAlt->fgColor, data, 3);
    } else if (id.equals(std::string(MW4_BLE_TEXT_DISPLAY_BG_COLOR_CHARACTERISTIC_UUID))) {
        memcpy(this->settings->bgColor, data, 3);
    } else if (id.equals(std::string(MW4_BLE_TEXT_DISPLAY_BG_COLOR_ALT_CHARACTERISTIC_UUID))) {
        memcpy(this->settingsAlt->bgColor, data, 3);
    } else if (id.equals(std::string(MW4_BLE_TEXT_DISPLAY_SCROLL_SPEED_CHARACTERISTIC_UUID))) {
        this->settings->scrollSpeed = *data;
    } else if (id.equals(std::string(MW4_BLE_TEXT_DISPLAY_SCROLL_SPEED_ALT_CHARACTERISTIC_UUID))) {
        this->settingsAlt->scrollSpeed = *data;
    } else if (id.equals(std::string(MW4_BLE_TEXT_DISPLAY_PAUSE_TIME_CHARACTERISTIC_UUID))) {
        this->settings->pauseTime = *data;
    } else if (id.equals(std::string(MW4_BLE_TEXT_DISPLAY_PAUSE_TIME_ALT_CHARACTERISTIC_UUID))) {
        this->settingsAlt->pauseTime = *data;
    }

    saveSettings();
}

void TextDisplayService::update(bool iAltMode) {
  static int16_t offset = STARTING_OFFSET;
  static bool isPaused = false;
  
  auto settingsToUse = !iAltMode ? this->settings : this->settingsAlt;

  static auto prev = millis();
  auto now = millis();
  auto elapsed = now - prev;

  if (!isPaused || elapsed >= settingsToUse->pauseTime * 1000) {
    if (elapsed >= settingsToUse->scrollSpeed) {
        isPaused = false;
        auto text = settingsToUse->text;

        auto customOffset = settingsToUse->offset;
        auto isCustomOffsetOn = !settingsToUse->scrolling;
        int16_t centerPoint = (text.size() * 6) / 2;

        this->strip->setFgColor(CRGB(settingsToUse->fgColor[0], settingsToUse->fgColor[1], settingsToUse->fgColor[2]).nscale8_video(settingsToUse->brightness));
        this->strip->setBgColor(CRGB(settingsToUse->bgColor[0], settingsToUse->bgColor[1], settingsToUse->bgColor[2]));
        this->strip->setText(text.c_str());

        if (isCustomOffsetOn) {
            this->strip->displayText(customOffset);
        } else {
            this->strip->displayText(offset++);
            if (text.size() <= 10) {  // only pause if text will fit
                if (offset == ((-STRIPLED_W / 2) + 1 + centerPoint)) {
                    isPaused = true;
                }
            }

            if (offset >= int16_t(text.size() * 6)) {
                offset = STARTING_OFFSET;
            }
        }

        prev = now;
    }
  }
}
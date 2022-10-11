#pragma once

#include <Arduino.h>

struct LightDeviceSettings {
  bool state = true;
  
  uint8_t mode = 0; // steady, pulse,  rainbow pulse, rainbow wave

  uint8_t hue = 255;
  uint8_t saturation = 255;
  uint8_t value = 255;
};

struct TextDisplaySettings {
  bool state = true;
  std::string text;
  
  bool scrolling = true;
  uint8_t scrollSpeed = 50;
  uint8_t pauseTime = 3;
  
  int16_t offset = 0;
  uint8_t brightness = 25;

  uint8_t fgColor[3] = {255, 255, 255};
  uint8_t bgColor[3] = {0, 0, 0};
};

struct MusicSettings {
  bool state = false;
  uint8_t volume = 0;
  uint8_t track = 0;
};

/**
 * @brief Reads settings from default and user settings files and initializes auto save.
 * Auto save persists current settings to files at regular intervals if needed.
 */
void initSettings();

// delete user settings AND REBOOT to reload
void resetSettings();

LightDeviceSettings* getChairLightSettings(bool iAlt=false);
LightDeviceSettings* getPedestalLightSettings(bool iAlt=false);

TextDisplaySettings* getTextDisplaySettings(bool iAlt=false);

MusicSettings* getMusicSettings();

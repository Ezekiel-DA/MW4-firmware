#pragma once

#include <Arduino.h>

extern portMUX_TYPE settingsMutex;

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
  uint8_t scrollSpeed = 75;
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

struct CostumeSettings {
  LightDeviceSettings chairLightSettings;
  LightDeviceSettings chairLightSettingsAlt;

  LightDeviceSettings pedestalLightSettings;
  LightDeviceSettings pedestalLightSettingsAlt;

  TextDisplaySettings textSettings;
  TextDisplaySettings textSettingsAlt;

  MusicSettings musicSettings;
};

// Settings management workflow:
// Call initSettings() in setup(). This reads default and user settings and stores the results locally. This also starts a background task for saving out settings.
// Use settings by calling get<X>Settings() to get pointers directly into the data structures held here.
// When making changes to said data structures, BE SURE TO TAKE THE settingsMutex CRITICAL SECTION! This avoids race conditions with the BLE callbacks.
// When done making changes to settings, call markSettingsModified()
// At some point, the background task will serialize the settings to JSON IN MEMORY ONLY.
// From loop(), call saveSettings(); this performs the actual write and needs to be done from the main loop() (or somewhere that won't be interrupted by FastLED.show()) to avoid FastLED's RMT + Flash issues

/**
 * @brief Reads settings from default and user settings files and initializes auto save.
 * Auto save persists current settings to files at regular intervals.
 */
void initSettings();

/**
 * @brief Notify that we want settings saved. See workflow above for critical information!
 */
void markSettingsModified();

/**
 * @brief Save out to SPIFFS. See workflow above for critical information!
 * 
 */
void saveSettings();

/**
 * @brief Delete user settings AND REBOOT to reload defaults.
 * 
 */
void resetSettings();

LightDeviceSettings* getChairLightSettings(bool iAlt=false);
LightDeviceSettings* getPedestalLightSettings(bool iAlt=false);

TextDisplaySettings* getTextDisplaySettings(bool iAlt=false);

MusicSettings* getMusicSettings();
